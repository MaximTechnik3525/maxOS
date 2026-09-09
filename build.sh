#!/bin/bash
set -e

echo "=== [1/3] Компиляция исходного кода  maxOS ==="
nasm -f elf32 entry.asm -o entry.o
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [2/3] Линковка бинарного файла ядра ==="
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld entry.o kernel.o -o mykernel.bin

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

menuentry "maxOS DoubleFixes" {
    # Ищем файл ядра по всему диску, игнорируя "буквы" и скобочки приводов
    search --no-floppy --set=root --file /boot/mykernel.bin
    multiboot /boot/mykernel.bin
    boot
}
EOF

# Используем grub-mkrescue, чтобы упаковать всё в правильный гибридный образ диска maxos.img
# Этот инструмент создаст и файловую систему, и правильный MBR-загрузчик в начале файла!
grub-mkrescue -o maxos.img iso

echo "============================================="
echo " Сборка завершена успешно!"
echo " maxOS запускается в QEMU!"
echo "============================================="

# Запускаем созданный полноценный образ диска как жесткий диск (-hda)
qemu-system-i386 -hda maxos.img -m 256M -vga qxl -machine pc,pcspk-audiodev=snd0 -audiodev alsa,id=snd0