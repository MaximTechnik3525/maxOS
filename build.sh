#!/bin/bash
set -e

echo "=== [1/3] Компиляция исходного кода maxOS ==="
nasm -f elf32 entry.asm -o entry.o

# Компилируем главное ядро
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -Wall -Wextra

# Компилируем драйвер жесткого диска ATA
gcc -m32 -c ata.c -o ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# Компилируем PCI-драйвер
gcc -m32 -c pci.c -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [2/3] Линковка ядра и сборка внешних ELF-приложений ==="
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld entry.o kernel.o ata.o pci.o -o mykernel.bin

gcc -m32 -c demo.c -o demo_game.o -std=gnu99 -ffreestanding -O2
ld -m elf_i386 --no-warn-rwx-segments -T app.ld demo_game.o -o demo.elf

echo "=== [3/3] Создание структуры и генерация загрузочного диска ==="
mkdir -p iso/boot/grub

cp mykernel.bin iso/boot/
cp demo.elf iso/boot/

cat << 'EOF' > iso/boot/grub/grub.cfg
font=video
if loadfont /boot/grub/fonts/unicode.pf2 ; then
    insmod vbe
    insmod vga
    insmod gfxterm
    insmod video_all
    set gfxmode=1280x720
    local_video
    terminal_output gfxterm
fi

set timeout=3
set default=0
set gfxpayload=keep

insmod part_msdos
insmod fat
insmod ext2

set gfxmode=1280x720x16
set gfxpayload=keep

menuentry "maxOS" {
    search --no-floppy --set=root --file /boot/mykernel.bin
    multiboot /boot/mykernel.bin
    module /boot/demo.elf
    boot
}
EOF

grub-mkrescue -o maxos.iso iso

echo "============================================="
echo " Сборка завершена успешно!"
echo " maxOS запускается в QEMU!"
echo "============================================="

qemu-system-i386 -hda maxos.iso -m 256M -vga std -machine pc,acpi=off,pcspk-audiodev=snd0 -device sb16,audiodev=snd0 -audiodev alsa,id=snd0 -netdev user,id=net0 -device rtl8139,netdev=net0