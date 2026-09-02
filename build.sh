#!/bin/bash

# Останавливать скрипт при любой ошибке
set -e

echo "=== [1/4] Компиляция исходного кода MaxOS ==="
nasm -f elf32 entry.asm -o entry.o
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [2/4] Линковка бинарного файла ядра ==="
# Флаг --no-warn-rwx-segments убирает предупреждение линкера
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld -o mykernel.bin entry.o kernel.o

echo "=== [3/4] Подготовка структуры ISO ==="
# Создаем строго стандартные папки для GRUB
mkdir -p iso/boot/grub

# Генерируем конфигурационный файл GRUB
# Генерируем конфигурационный файл GRUB с принудительной графикой
cat << 'EOF' > iso/boot/grub/grub.cfg
insmod vbe
insmod vga
insmod video_bochs
insmod video_cirrus

# Принудительно ставим разрешение графики для самого GRUB
set gfxmode=1024x768x16
# Указываем GRUB передать этот графический режим ядру "как есть" (НЕ переключать в текст)
set gfxpayload=keep

menuentry "maxOS NeonCycle" {
    multiboot /boot/mykernel.bin
    boot
}
EOF

# Копируем ядро в папку boot внутри будущего диска
cp mykernel.bin iso/boot/

echo "=== [4/4] Создание загрузочного диска maxos.iso ==="
grub-mkrescue -o maxos.iso iso

echo "============================================="
echo " Сборка завершена успешно! Файл ОС: maxos.iso"
echo " Запуск в QEMU начинается!"
echo "============================================="
qemu-system-i386 -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -cdrom maxos.iso