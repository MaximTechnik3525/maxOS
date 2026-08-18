#!/bin/bash
# ==========================================================
#   maxOS v2.1 — Идеальная Windows-сборка на движке Mingw-w64
# ==========================================================

check_error() {
    if [ $1 -ne 0 ]; then
        echo "---------------------------------------"
        echo " [ERROR] Build failed! Check code."
        echo "---------------------------------------"
        exit 1
    fi
}

# 1. Сборка оригинального загрузчика (40 секторов)
echo "[1/5] Compiling bootloader..."
nasm -f bin boot.asm -o boot.bin
check_error $?

# 2. Сборка Си-кода через оригинальный Windows GCC внутри Linux!
echo "[2/5] Compiling C kernel (Windows-style)..."
i686-w64-mingw32-gcc -m32 -ffreestanding -c kernel.c -o kernel.o
check_error $?

# 3. ЛИНКОВКА: Твой родной Windows-линкер собирает эталонный kernel.exe!
# Все флаги выравнивания по 32 байта и точка входа kmain взяты один в один из твоего батника.
echo "[3/5] Linking binary (i386pe)..."
i686-w64-mingw32-ld -m i386pe -s -X --file-alignment=32 --section-alignment=32 -Ttext 0x1000 -e _kmain -o kernel.exe kernel.o
check_error $?

# 4. Создание чистого бинарника
echo "[4/5] Extracting flat binary..."
objcopy -O binary kernel.exe kernel.bin
check_error $?

# 5. Создание итогового образа диска
echo "[5/5] Creating OS image..."
cat boot.bin kernel.bin > maxos.img
check_error $?

# Чистим временные файлы
rm -f kernel.o kernel.exe

echo "======================================="
echo "   SUCCESS! Running maxOS in QEMU..."
echo "======================================="
# Чистый запуск графики эмулятора
qemu-system-i386 -vga cirrus -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -display default,full-screen=on -fda maxos.img