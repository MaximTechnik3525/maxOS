@echo off
echo =======================================
echo         Building maxOS Kernel...
echo =======================================

:: 1. Сборка загрузчика
echo [1/5] Compiling bootloader...
nasm -f bin boot.asm -o boot.bin
if %errorlevel% neq 0 goto erro

:: 2. Сборка Си-кода
echo [2/5] Compiling C kernel...
gcc -m32 -ffreestanding -c kernel.c -o kernel.o
if %errorlevel% neq 0 goto error

:: 3. Линковка
echo [3/5] Linking binary...
ld -m i386pe -s -X --file-alignment=32 --section-alignment=32 -Ttext 0x1000 -e kmain -o kernel.exe kernel.o
if %errorlevel% neq 0 goto error


:: 4. Создание чистого бинарника
echo [4/5] Extracting flat binary...

objcopy -O binary kernel.exe kernel.bin
if %errorlevel% neq 0 goto error

:: 5. Создание итогового образа диска
echo [5/5] Creating OS image...
copy /b boot.bin + kernel.bin maxos.img > nul
if %errorlevel% neq 0 goto error

echo =======================================
echo    SUCCESS! Running maxOS in QEMU...
echo =======================================
rem Современный способ подключения звука QEMU к звуковой карте Windows (sdl или dsound)
qemu-system-i386 -vga std -machine pcspk-audiodev=audio0 -audiodev sdl,id=audio0 -fda maxos.img
goto end

:error
echo ---------------------------------------
echo  [ERROR] Build failed! Check code.
echo ---------------------------------------
pause

:end