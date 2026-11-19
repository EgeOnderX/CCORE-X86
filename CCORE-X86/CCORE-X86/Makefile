CC := gcc
AS := nasm
LD := ld
OBJCOPY := objcopy
CFLAGS := -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -O2 -Wall -Wextra -fno-asynchronous-unwind-tables -fno-tree-loop-distribute-patterns -mno-sse -mno-mmx -march=i486 -ISystem
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib -z noexecstack --no-warn-rwx-segments
KSECT := 256

APPS := calc cver filemgr notepad paint settings taskman cmd
OBJS := build/Kernel.o build/DWM.o $(addprefix build/,$(addsuffix .o,$(APPS)))

all: ccore.img

build:
	@mkdir -p build

build/boot.bin: System/Boot.asm | build
	$(AS) -f bin $< -o $@

build/Kernel.o: System/Kernel.asm | build
	$(AS) -f elf32 $< -o $@

build/DWM.o: System/DWM.c System/DWM.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: Apps/%.c System/DWM.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@
	@test $$(stat -c %s $@) -le $$(( $(KSECT) * 512 )) || (echo "kernel larger than $(KSECT) sectors"; exit 1)

# The disk is created once; boot sector + kernel are rewritten, data area (LBA 300+) persists.
ccore.img: build/boot.bin build/kernel.bin
	@[ -f $@ ] || dd if=/dev/zero of=$@ bs=512 count=8192 2>/dev/null
	dd if=build/boot.bin of=$@ bs=512 seek=0 conv=notrunc 2>/dev/null
	dd if=build/kernel.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null

run: ccore.img
	qemu-system-i386 -drive file=ccore.img,format=raw,if=ide,index=0,media=disk -m 128 -vga std

clean:
	rm -rf build

distclean: clean
	rm -f ccore.img

.PHONY: all run clean distclean
