#!/bin/bash
set -e

echo "=== [1/4] Компиляция исходного кода maxOS ==="
nasm -f elf32 entry.asm -o entry.o
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -Wall -Wextra
gcc -m32 -c ata.c -o ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
gcc -m32 -c pci.c -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "=== [2/4] Линковка ядра, demo.elf и game.elf ==="
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld entry.o kernel.o ata.o pci.o -o mykernel.bin

gcc -m32 -c demo.c -o demo_game.o -std=gnu99 -ffreestanding -O2
ld -m elf_i386 --no-warn-rwx-segments -T app.ld demo_game.o -o demo.elf

gcc -m32 -c game.c -o game.o -std=gnu99 -ffreestanding -O2
ld -m elf_i386 --no-warn-rwx-segments -T app.ld game.o -o game.elf

echo "=== [3/4] Компиляция порта DOOM (doom.elf) ==="
rm -f doom/i_main.c doom/w_file_posix.c doom/w_file_win32.c doom/i_allegromusic.c doom/i_allegrosound.c doom/i_sdlmusic.c doom/i_sdlsound.c
rm -rf doom_obj
mkdir -p doom_obj
for f in doom/*.c; do
    base=$(basename "$f" .c)
    gcc -m32 -c "$f" -o "doom_obj/${base}.o" -std=gnu99 -ffreestanding -fno-builtin -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2 -Idoom
done
LIBGCC=$(gcc -m32 -print-libgcc-file-name)
ld -m elf_i386 --no-warn-rwx-segments -T app.ld doom_obj/*.o "$LIBGCC" -o doom.elf

echo "=== [4/4] Создание структуры и генерация загрузочного диска ==="
mkdir -p iso/boot/grub

cp mykernel.bin iso/boot/
cp demo.elf iso/boot/
cp doom.elf iso/boot/
cp doom1.WAD iso/boot/doom1.wad
cp game.elf iso/boot/

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
    module /boot/doom.elf
    module /boot/doom1.wad
    module /boot/game.elf
    boot
}
EOF

grub-mkrescue -o maxos.iso iso

echo "============================================="
echo " Сборка завершена успешно!"
echo " maxOS + DOOM + game.elf запускаются в QEMU!"
echo "============================================="

qemu-system-i386 -hda maxos.iso -m 256M -vga std -machine pc,acpi=off,pcspk-audiodev=snd0 -device sb16,audiodev=snd0 -audiodev alsa,id=snd0 -netdev user,id=net0 -device rtl8139,netdev=net0