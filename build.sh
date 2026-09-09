#!/bin/bash

# Останавливать скрипт при любой ошибке
set -e

# Автоопределение 64-битного компилятора и линкера
if command -v x86_64-linux-gnu-gcc >/dev/null 2>&1; then
    CC="x86_64-linux-gnu-gcc"
    LD="x86_64-linux-gnu-ld"
else
    CC="gcc -m64"
    LD="ld -m elf_x86_64"
fi

# Поддержка очистки
if [ "$1" == "--clean" ]; then
    echo "Очистка директории сборки..."
    rm -rf build iso/boot/mykernel.bin maxos.iso
    echo "Очистка завершена."
    exit 0
fi

# Создаем папку для объектных и бинарных файлов сборки
mkdir -p build

CFLAGS="-std=gnu99 -ffreestanding -O2 -Wall -Wextra -mno-red-zone -mstackrealign -mcmodel=small -Isrc -Isrc/kernel -Isrc/drivers -Isrc/fs -Isrc/apps -Isrc/boot"

echo "=== [1/5] Сборка MBR загрузчика ==="
nasm -f bin src/boot/boot_mbr.asm -o build/boot_mbr.bin
nasm -Ibuild/ -f elf64 src/boot/mbr_data.asm -o build/mbr_data.o

echo "=== [2/5] Компиляция исходного кода MaxOS (x86_64) ==="
nasm -f elf64 src/boot/entry.asm -o build/entry.o
nasm -f elf64 src/kernel/syscall_asm.asm -o build/syscall_asm.o
nasm -f elf64 src/kernel/idt_asm.asm -o build/idt_asm.o
$CC -c src/kernel/debug.c -o build/debug.o $CFLAGS
$CC -c src/kernel/idt.c -o build/idt.o $CFLAGS
$CC -c src/kernel/user.c -o build/user.o $CFLAGS
$CC -c src/drivers/ata.c -o build/ata.o $CFLAGS
$CC -c src/fs/maxfs.c -o build/maxfs.o $CFLAGS
$CC -c src/apps/maxp.c -o build/maxp.o $CFLAGS
$CC -c src/kernel/taskbar.c -o build/taskbar.o $CFLAGS
$CC -c src/apps/notepad.c -o build/notepad.o $CFLAGS
$CC -c src/apps/installer.c -o build/installer.o $CFLAGS
$CC -c src/apps/explorer.c -o build/explorer.o $CFLAGS
$CC -c src/apps/calc.c -o build/calc.o $CFLAGS
$CC -c src/apps/sysinfo.c -o build/sysinfo.o $CFLAGS
$CC -c src/apps/pong.c -o build/pong.o $CFLAGS
$CC -c src/apps/mem.c -o build/mem.o $CFLAGS
$CC -c src/kernel/kernel.c -o build/kernel.o $CFLAGS

echo "=== [3/5] Линковка 64-битного ядра (ELF64) ==="
$LD --no-warn-rwx-segments -T src/linker.ld -o build/mykernel.bin \
    build/entry.o build/mbr_data.o build/syscall_asm.o build/idt_asm.o \
    build/debug.o build/idt.o build/user.o build/kernel.o build/ata.o \
    build/maxfs.o build/maxp.o build/taskbar.o build/notepad.o \
    build/installer.o build/explorer.o build/calc.o build/sysinfo.o build/pong.o build/mem.o

echo "=== [4/5] Подготовка структуры ISO и сборка maxos.iso ==="
mkdir -p iso/boot/grub

# Генерируем конфигурационный файл GRUB с принудительной графикой
cat << 'EOF' > iso/boot/grub/grub.cfg
insmod vbe
insmod vga
insmod video_bochs
insmod video_cirrus

# Принудительно ставим разрешение графики для самого GRUB
set default=0
set timeout=0
set gfxmode=1024x768x16
# Указываем GRUB передать этот графический режим ядру "как есть" (НЕ переключать в текст)
set gfxpayload=keep

menuentry "maxOS RedCycle v3.1 (x86_64 Long Mode)" {
    multiboot /boot/mykernel.bin
    boot
}
EOF

# Копируем ядро в папку boot внутри будущего диска
cp build/mykernel.bin iso/boot/

# Сборка ISO
grub-mkrescue -o maxos.iso iso

echo "=== [5/5] Подготовка виртуального жесткого диска ==="
if [ ! -f maxos_disk.img ]; then
    echo "Создание чистого диска maxos_disk.img (64 МБ)..."
    qemu-img create -f raw maxos_disk.img 64M
else
    echo "Диск maxos_disk.img уже существует."
fi

echo "============================================="
echo " Сборка 64-битной maxOS v3.1 завершена успешно!"
echo " Архитектура: x86_64 (Long Mode)"
echo " ISO файл: maxos.iso"
echo " HDD диск: maxos_disk.img"
echo "============================================="

if [ "$1" == "--boot-hdd" ]; then
    echo " Запуск maxOS напрямую с жесткого диска (HDD) в QEMU x86_64..."
    qemu-system-x86_64 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=maxos_disk.img,format=raw,index=0,media=disk -boot c -serial stdio
elif [ "$1" != "--no-run" ]; then
    echo " Запуск в QEMU x86_64 с подключенным жестким диском..."
    qemu-system-x86_64 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=maxos_disk.img,format=raw,index=0,media=disk -cdrom maxos.iso -boot d -serial stdio
fi