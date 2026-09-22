#!/bin/bash
set -e

echo "=== [1/3] Компиляция исходного кода maxOS ==="
nasm -f elf32 entry.asm -o entry.o

# Компилируем главное ядро
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# Компилируем драйвер жесткого диска ATA
gcc -m32 -c ata.c -o ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# Компилируем звуковой драйвер
gcc -m32 -c sb16.c -o sb16.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# === ИСПРАВЛЕНО 1: Добавили компиляцию нашего нового PCI-драйвера! ===
gcc -m32 -c pci.c -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [2/3] Линковка бинарного файла ядра ==="
# === ИСПРАВЛЕНО 2: Добавили pci.o в цепочку линковщика ld ===
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld entry.o kernel.o ata.o sb16.o pci.o -o mykernel.bin

echo "=== [3/3] Создание структуры и генерация загрузочного диска ==="
mkdir -p iso/boot/grub

cp mykernel.bin iso/boot/

cat << 'EOF' > iso/boot/grub/grub.cfg
insmod vbe
insmod vga
insmod video_all
insmod part_msdos
insmod fat
insmod ext2

set gfxmode=1024x768x16
set gfxpayload=keep

menuentry "maxOS SystemDisk" {
    search --no-floppy --set=root --file /boot/mykernel.bin
    multiboot /boot/mykernel.bin
    boot
}
EOF

grub-mkrescue -o maxos.img iso

echo "============================================="
echo " Сборка завершена успешно!"
echo " maxOS запускается в QEMU!"
echo "============================================="

qemu-system-i386 -hda maxos.img -m 256M -vga qxl -machine pc,pcspk-audiodev=snd0 -device sb16,audiodev=snd0 -audiodev alsa,id=snd0
