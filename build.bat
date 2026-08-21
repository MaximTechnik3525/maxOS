@echo off
rem ==========================================================
rem   maxOS v2.7 — Стабильный ультра-сжатый релиз (Windows)
rem ==========================================================
cls

echo [1/4] Compiling Bootloader...
nasm -f bin boot.asm -o boot.bin
if %errorlevel% neq 0 goto error

rem Компиляция Си-ядра со всеми твоими флагами точечной диеты
echo [2/4] Compiling C kernel (Safe Size Optimization)...
i686-w64-mingw32-gcc -m32 -ffreestanding -Os -fno-toplevel-reorder -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -c kernel.c -o kernel.o -fmerge-all-constants -fno-ident -fno-stack-protector -fno-exceptions
if %errorlevel% neq 0 goto error

rem Линковка i386pe с жестким выравниванием секций по 32 байта
echo [3/4] Linking binary (i386pe)...
i686-w64-mingw32-ld -m i386pe -s -X --file-alignment=32 --section-alignment=32 -Ttext 0x1000 -e _kmain -o kernel.exe kernel.o
if %errorlevel% neq 0 goto error

echo [4/4] Extracting flat binary...
objcopy -O binary kernel.exe kernel.bin
if %errorlevel% neq 0 goto error

echo [5/4] Creating Floppy Image...
rem Замена Linux-команды cat на бинарную склейку Windows
copy /b boot.bin + kernel.bin maxos.img > nul
if %errorlevel% neq 0 goto error

rem Замена команды truncate на PowerShell (увеличивает файл ровно до 1474560 байт)
powershell -Command "$f = [System.IO.File]::OpenWrite('maxos.img'); $f.SetLength(1474560); $f.Close()"
if %errorlevel% neq 0 goto error

rem Безопасная очистка временного мусора
del kernel.o kernel.exe boot.bin kernel.bin

echo =======================================
echo    SUCCESS! Running maxOS in QEMU...
echo =======================================
rem Аудиокарта переключена на DirectSound (dsound) для нативного звука спикера в Windows
qemu-system-i386 -vga cirrus -audiodev dsound,id=snd0 -machine pcspk-audiodev=snd0 -display default,full-screen=on -fda maxos.img
exit /b 0

:error
echo ---------------------------------------
echo  [ERROR] Build failed! Check your code.
echo ---------------------------------------
pause
exit /b 1