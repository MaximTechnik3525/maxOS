@echo off
chcp 65001 >nul

echo === [1/3] Компиляция исходного кода maxOS ===
nasm -f elf32 entry.asm -o entry.o
if %errorlevel% neq 0 exit /b %errorlevel%

gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -Wall -Wextra
if %errorlevel% neq 0 exit /b %errorlevel%

gcc -m32 -c ata.c -o ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 exit /b %errorlevel%

gcc -m32 -c pci.c -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 exit /b %errorlevel%

echo === [2/3] Линковка бинарного файла ядра ===
ld -m elf_i386 --no-warn-rwx-segments -T linker.ld entry.o kernel.o ata.o pci.o -o mykernel.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo === [3/3] Создание структуры и генерация загрузочного диска ===
if not exist iso\boot\grub mkdir iso\boot\grub
copy /Y mykernel.bin iso\boot\ >nul

(
echo font=video
echo if loadfont /boot/grub/fonts/unicode.pf2 ; then
echo     insmod vbe
echo     insmod vga
echo     insmod gfxterm
echo     insmod video_all
echo     set gfxmode=1280x720
echo     local_video
echo     terminal_output gfxterm
echo fi
echo.
echo set timeout=3
echo set default=0
echo set gfxpayload=keep
echo.
echo insmod part_msdos
echo insmod fat
echo insmod ext2
echo.
echo set gfxmode=1280x720x16
echo set gfxpayload=keep
echo.
echo menuentry "maxOS" {
echo     search --no-floppy --set=root --file /boot/mykernel.bin
echo     multiboot /boot/mykernel.bin
echo     boot
echo }
) > iso\boot\grub\grub.cfg

grub-mkrescue -o maxos.iso iso
if %errorlevel% neq 0 exit /b %errorlevel%

echo =============================================
echo  Сборка завершена успешно!
echo  maxOS запускается в QEMU!
echo =============================================

qemu-system-i386 -hda maxos.iso -m 256M -vga std -machine pc,acpi=off,pcspk-audiodev=snd0 -device sb16,audiodev=snd0 -audiodev dsound,id=snd0 -netdev user,id=net0 -device rtl8139,netdev=net0