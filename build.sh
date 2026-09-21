#!/bin/bash
set -e

echo "=== [1/3] Компиляция исходного кода maxOS ==="
nasm -f elf32 entry.asm -o entry.o

# Компилируем главное ядро
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# ИСПРАВЛЕНО: Добавили компиляцию нашего нового драйвера жесткого диска ATA!
gcc -m32 -c ata.c -o ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [2/3] Линковка бинарного файла ядра ==="
# ИСПРАВЛЕНО: Добавили ata.o в цепочку линковщика ld, чтобы склеить файлы вместе
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld entry.o kernel.o ata.o -o mykernel.bin

echo "=== [3/3] Создание структуры и генерация загрузочного диска ==="
mkdir -p iso/boot/grub

# Копируем ядро
cp mykernel.bin iso/boot/

# Генерируем правильный универсальный grub.cfg
cat << 'EOF' > iso/boot/grub/grub.cfg
insmod vbe
insmod vga
insmod video_all
insmod part_msdos
insmod fat
insmod ext2

set gfxmode=1024x768x16
set gfxpayload=keep

menuentry "maxOS ColorScreen" {
    search --no-floppy --set=root --file /boot/mykernel.bin
    multiboot /boot/mykernel.bin
    boot
}
EOF

# Используем grub-mkrescue, чтобы упаковать всё в правильный гибридный образ диска maxos.img
grub-mkrescue -o maxos.img iso

echo "============================================="
echo " Сборка завершена успешно!"
echo " maxOS запускается в QEMU!"
echo "============================================="

# Запускаем созданный полноценный образ диска как жесткий диск (-hda)
qemu-system-i386 -hda maxos.img -m 256M -vga qxl -machine pc,pcspk-audiodev=snd0 -audiodev alsa,id=snd0