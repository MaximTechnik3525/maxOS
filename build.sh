#!/bin/bash
# ==========================================================
#   maxOS v2.4 — Стабильный ультра-сжатый релиз (Floppy)
# ==========================================================

check_error() {
    if [ $1 -ne 0 ]; then
        echo "---------------------------------------"
        echo " [ERROR] Build failed! Check code."
        echo "---------------------------------------"
        exit 1
    fi
}

echo "[1/4] Compiling Bootloader..."
nasm -f bin boot.asm -o boot.bin
check_error $?

# 1. Включаем оптимизацию размера -Os, но ЗАПРЕЩАЕМ компилятору разносить функции!
echo "[2/4] Compiling C kernel (Safe Size Optimization)..."
i686-w64-mingw32-gcc -m32 -ffreestanding -Os -fno-toplevel-reorder -c kernel.c -o kernel.o
check_error $?

# 2. Линкуем с оригинальным выравниванием по 32 байта из твоего батника.
# Флаг -e _kmain и -Ttext 0x1000 жестко привяжут старт к первому байту адреса 0x1000!
echo "[3/4] Linking binary (i386pe)..."
i686-w64-mingw32-ld -m i386pe -s -X \
  --file-alignment=32 --section-alignment=32 \
  -Ttext 0x1000 -e _kmain -o kernel.exe kernel.o
check_error $?

# 3. Чистое извлечение плоского бинарника из твоего родного батника
echo "[4/4] Extracting flat binary..."
objcopy -O binary kernel.exe kernel.bin
check_error $?

echo "[5/4] Creating Floppy Image..."
cat boot.bin kernel.bin > maxos.img
check_error $?

# 4. Добиваем до эталонных 1.44 МБ для строгого SeaBIOS
truncate -s 1440k maxos.img
check_error $?

rm -f kernel.o kernel.exe boot.bin kernel.bin

echo "======================================="
echo "   SUCCESS! Running maxOS in QEMU..."
echo "======================================="
qemu-system-i386 -vga cirrus -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -display default,full-screen=on -fda maxos.img