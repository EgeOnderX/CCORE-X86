#!/bin/bash

# Hata alindiginda betigi durdur
set -e

echo "[1/5] Bootloader ve Kernel Giris Modulu Derleniyor..."
nasm -f bin Boot.asm -o boot.bin
nasm -f elf32 System/Kernel.asm -o kernel_entry.o

echo "[2/5] DWM ve PROFLS Modulleri Derleniyor..."
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c System/DWM.c -o dwm.o -fno-pie -nostdlib
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c Profls/calc.c -o calc.o -fno-pie -nostdlib
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c Profls/settings.c -o settings.o -fno-pie -nostdlib
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c Profls/filemgr.c -o filemgr.o -fno-pie -nostdlib
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c Profls/taskman.c -o taskman.o -fno-pie -nostdlib
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c Profls/cver.c -o cver.o -fno-pie -nostdlib
gcc -m32 -fno-leading-underscore -ffreestanding -I System -c Profls/paint.c -o paint.o -fno-pie -nostdlib

echo "[3/5] Moduller Birlestiriliyor (Linker)..."
ld -m elf_i386 -T linker.ld kernel_entry.o dwm.o calc.o settings.o filemgr.o taskman.o cver.o paint.o -o system.elf

echo "[4/5] Saf Binary'ye Donusturuluyor (Objcopy)..."
objcopy -O binary -S system.elf system.bin

echo "[5/5] Disk Imai Olusturuluyor (1.44MB Exact Padding)..."
cat boot.bin system.bin > ccore.img
truncate -s 1474560 ccore.img

# Gecici nesne ve elf dosyalarini temizle
rm -f *.o *.elf

echo "CCORE x86 V1.0 Basariyla Olusturuldu! QEMU Baslatiliyor..."
qemu-system-i386 -drive file=ccore.img,format=raw,index=0,if=floppy -boot a
