# CCORE Setup : builds setup.img, an installer disk. Booting it starts CCORE normally,
# then the Setup wizard asks for account, date/time and personalization and installs CCORE onto the disk.
ROOT := ../CCORE-X86
CC := gcc
AS := nasm
LD := ld
OBJCOPY := objcopy
CFLAGS := -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -O2 -Wall -Wextra -fno-asynchronous-unwind-tables -fno-tree-loop-distribute-patterns -mno-sse -mno-mmx -march=i486 -I$(ROOT)/System -DSETUP_BUILD
LDFLAGS := -m elf_i386 -T $(ROOT)/linker.ld -nostdlib -z noexecstack --no-warn-rwx-segments
APPS := calc cver filemgr notepad paint settings taskman cmd
OBJS := build/Kernel.o build/DWM.o build/Setup.o $(addprefix build/,$(addsuffix .o,$(APPS)))

all: setup.img

build:
	@mkdir -p build

# the normal CCORE system is built first; it is the payload Setup installs
$(ROOT)/build/boot.bin $(ROOT)/build/kernel.bin: $(wildcard $(ROOT)/System/* $(ROOT)/Apps/*)
	$(MAKE) -C $(ROOT) build/boot.bin build/kernel.bin

build/Kernel.o: $(ROOT)/System/Kernel.asm | build
	$(AS) -f elf32 $< -o $@
build/DWM.o: $(ROOT)/System/DWM.c $(ROOT)/System/DWM.h | build
	$(CC) $(CFLAGS) -c $< -o $@
build/Setup.o: Setup.c $(ROOT)/System/DWM.h | build
	$(CC) $(CFLAGS) -c $< -o $@
build/%.o: $(ROOT)/Apps/%.c $(ROOT)/System/DWM.h | build
	$(CC) $(CFLAGS) -c $< -o $@
build/setup.elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
build/setup.bin: build/setup.elf
	$(OBJCOPY) -O binary $< $@
	@test $$(stat -c %s $@) -le 131072 || (echo "setup kernel too large"; exit 1)

# LBA 0 boot | 1..256 setup kernel | 999 payload boot | 1000..1255 payload CCORE kernel
setup.img: $(ROOT)/build/boot.bin build/setup.bin $(ROOT)/build/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=8192 2>/dev/null
	dd if=$(ROOT)/build/boot.bin of=$@ bs=512 seek=0 conv=notrunc 2>/dev/null
	dd if=build/setup.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	dd if=$(ROOT)/build/boot.bin of=$@ bs=512 seek=999 conv=notrunc 2>/dev/null
	dd if=$(ROOT)/build/kernel.bin of=$@ bs=512 seek=1000 conv=notrunc 2>/dev/null

run: setup.img
	qemu-system-i386 -drive file=setup.img,format=raw,if=ide,index=0,media=disk -m 128 -vga std

reinstall:
	rm -f setup.img
	$(MAKE) run

clean:
	rm -rf build setup.img

.PHONY: all run reinstall clean
