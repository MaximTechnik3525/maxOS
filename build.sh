#!/bin/bash

# Останавливать скрипт при любой ошибке
set -e

# Автоопределение компилятора и линкера (поддержка x86 и ARM64 кросс-компиляции)
if command -v i686-linux-gnu-gcc >/dev/null 2>&1; then
    CC="i686-linux-gnu-gcc"
    LD="i686-linux-gnu-ld"
else
    CC="gcc -m32"
    LD="ld -m elf_i386"
fi

echo "=== [1/5] Сборка MBR загрузчика ==="
nasm -f bin boot_mbr.asm -o boot_mbr.bin
nasm -f elf32 mbr_data.asm -o mbr_data.o

echo "=== [2/5] Компиляция исходного кода MaxOS ==="
nasm -f elf32 entry.asm -o entry.o
$CC -c ata.c -o ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
$CC -c maxfs.c -o maxfs.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
$CC -c notepad.c -o notepad.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
$CC -c installer.c -o installer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
$CC -c explorer.c -o explorer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
$CC -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [3/5] Линковка бинарного файла ядра ==="
# Флаг --no-warn-rwx-segments убирает предупреждение линкера
$LD --no-warn-rwx-segments -T linker.ld -o mykernel.bin entry.o mbr_data.o kernel.o ata.o maxfs.o notepad.o installer.o explorer.o

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

menuentry "maxOS RedCycle" {
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
echo " Сборка завершена успешно!"
echo " ISO файл: maxos.iso"
echo " HDD диск: maxos_disk.img"
echo "============================================="

if [ "$1" == "--boot-hdd" ]; then
    echo " Запуск maxOS напрямую с жесткого диска (HDD) в QEMU..."
    qemu-system-i386 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=maxos_disk.img,format=raw,index=0,media=disk -boot c
elif [ "$1" != "--no-run" ]; then
    echo " Запуск в QEMU с подключенным жестким диском..."
    qemu-system-i386 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=maxos_disk.img,format=raw,index=0,media=disk -cdrom maxos.iso -boot d
fi