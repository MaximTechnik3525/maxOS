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

CFLAGS="-std=gnu99 -ffreestanding -O2 -Wall -Wextra -mno-red-zone -mcmodel=small -mgeneral-regs-only"

echo "=== [1/5] Сборка MBR загрузчика ==="
nasm -f bin boot_mbr.asm -o boot_mbr.bin
nasm -f elf64 mbr_data.asm -o mbr_data.o

echo "=== [2/5] Компиляция исходного кода MaxOS (x86_64) ==="
nasm -f elf64 entry.asm -o entry.o
$CC -c ata.c -o ata.o $CFLAGS
$CC -c maxfs.c -o maxfs.o $CFLAGS
$CC -c maxp.c -o maxp.o $CFLAGS
$CC -c taskbar.c -o taskbar.o $CFLAGS
$CC -c notepad.c -o notepad.o $CFLAGS
$CC -c installer.c -o installer.o $CFLAGS
$CC -c explorer.c -o explorer.o $CFLAGS
$CC -c calc.c -o calc.o $CFLAGS
$CC -c sysinfo.c -o sysinfo.o $CFLAGS
$CC -c pong.c -o pong.o $CFLAGS
$CC -c kernel.c -o kernel.o $CFLAGS

echo "=== [3/5] Линковка 64-битного ядра (ELF64) ==="
$LD --no-warn-rwx-segments -T linker.ld -o mykernel.bin entry.o mbr_data.o kernel.o ata.o maxfs.o maxp.o taskbar.o notepad.o installer.o explorer.o calc.o sysinfo.o pong.o

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

menuentry "maxOS RedCycle (x86_64 Long Mode)" {
    multiboot /boot/mykernel.bin
    boot
}
EOF

# Копируем ядро в папку boot внутри будущего диска
cp mykernel.bin iso/boot/

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
echo " Сборка 64-битной maxOS завершена успешно!"
echo " Архитектура: x86_64 (Long Mode)"
echo " ISO файл: maxos.iso"
echo " HDD диск: maxos_disk.img"
echo "============================================="

if [ "$1" == "--boot-hdd" ]; then
    echo " Запуск maxOS напрямую с жесткого диска (HDD) в QEMU x86_64..."
    qemu-system-x86_64 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=maxos_disk.img,format=raw,index=0,media=disk -boot c
elif [ "$1" != "--no-run" ]; then
    echo " Запуск в QEMU x86_64 с подключенным жестким диском..."
    qemu-system-x86_64 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=maxos_disk.img,format=raw,index=0,media=disk -cdrom maxos.iso -boot d
fi