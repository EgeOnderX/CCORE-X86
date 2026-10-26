# CCORE X86


https://github.com/user-attachments/assets/7f31e1a2-5499-4440-bc03-ac8d81764e13


CCORE X86 is an open-source 32-bit bare-metal operating system kernel and graphical desktop environment built for x86 architecture systems. It is written in C and x86 Assembly, featuring a custom two-stage bootloader, a protected-mode transition pipeline, a double-buffered Desktop Window Manager (DWM), low-level hardware drivers, and built-in GUI applications.

> **Development Note & AI Transparency:**
> AI generation tools were utilized during the initial phase of this project to rapidly bootstrap early infrastructure and generate foundational C structures, primarily to compensate for initial limitations in low-level C programming experience. However, as kernel development progresses, direct AI dependency is being systematically reduced in favor of manually engineered, low-level implementations and refactored kernel subsystems. Yes, I am aware that while this is version 0.1, the texts display version 1.0.0 or experience overflow issues, and components like the file manager are currently static; however, upon reaching version 1.0, all of these will be resolved, transitioning the project from AI slop into a more usable and self-evolving operating system.

---

## 1. System Architecture Overview

CCORE X86 transitions from 16-bit Real Mode to 32-bit Protected Mode using a custom boot sector, setting up linear video addressing and low-level hardware interface handlers.

### 1.1 Bootloader & Mode Switching (`Boot.asm`)
* **Execution Start:** Loaded by BIOS at physical address `0x7C00` in 16-bit Real Mode.
* **Disk I/O:** Resets the drive controller and reads 48 sequential kernel sectors from disk using CHS conversion calculated from LBA offsets via `INT 0x13` into buffer address `0x1000`.
* **VBE Initialization:** Queries and sets VBE Mode `0x4101` (640x480 resolution, 256 colors) via `INT 0x10`. The Linear Framebuffer (LFB) physical address returned at `es:0x8028` is mapped to memory location `0x7E00` for kernel consumption.
* **GDT & Protected Mode:** Initializes a Global Descriptor Table defining 32-bit Code (`0x08`) and Data (`0x10`) flat memory segments, enables bit 0 of `CR0`, and performs a far jump (`jmp 0x08:init_pm`) into 32-bit Protected Mode.
* **Stack & Kernel Jump:** Configures segment registers, sets the stack pointer to `0x90000`, and jumps directly to the entry point at address `0x1000`.

### 1.2 Kernel Entry (`Kernel.asm`)
* Receives execution control in 32-bit mode and invokes `kmain()`.
* Contains an inline assembly fall-through loop (`cli`, `hlt`) to safely lock the CPU if `kmain` terminates unexpectedly.

---

## 2. Desktop Window Manager & Core System (`DWM.c` / `DWM.h`)

### 2.1 Double Buffering & Rendering
To prevent screen tearing across VBE 640x480 rendering operations, CCORE X86 implements software double buffering:
* Pixel operations write directly to a backbuffer located at address `0x200000`.
* The `flip_buffer()` routine waits for vertical blanking signals via the VGA status register (`inb(0x3DA)`) before copying the backbuffer to the active Linear Framebuffer (`vga_lfb`) in 32-bit word blocks.
* Bitmapped rendering is handled via a built-in 8x8 font array (`font8x8_basic`) covering ASCII characters and symbols.

### 2.2 Memory Verification Routine
Upon launch, `kmain()` executes a sequential memory testing routine:
* Iterates from physical memory address `0x100000` (1 MB) up to 32 MB in 1 MB blocks.
* Writes pattern `0x55AA55AA` to test locations and validates readback integrity.
* Triggers a kernel BSOD halt if memory verification fails.

### 3.3 Blue Screen of Death (BSOD) & Panic Handling
* **Unified Panic Entry:** `trigger_bsod(const char* error_code)` paints a solid blue canvas (`0x01`), renders diagnostic error details and stop messages, flushes the framebuffer, disables interrupts (`cli`), and halts the processor (`hlt`).

---

## 3. Low-Level Hardware Drivers

* **PS/2 Mouse Driver:** Communicates with the 8042 Keyboard/Mouse controller across Port `0x60` and Port `0x64`. Decodes 3-byte packets, handles directional sign extension, enforces boundary clamping (640x480), updates software mouse trails, and manages click state transitions.
* **ATA Hard Disk Driver:** Uses ports `0x1F0` through `0x1F7` to issue direct sector I/O operations in LBA mode. Contains polling routines for disk ready status (`0x1F7`) and data transfer (`0x1F0`).
* **CMOS / Real-Time Clock (RTC):** Reads registers `0x00`, `0x02`, and `0x04` via Ports `0x70` and `0x71`. Handles BCD-to-Binary conversions to format real-time system timestamps on the taskbar.

---

## 4. Built-in Applications

The OS core includes multiple graphical utility modules:

* **Calculator (`calc.c`):** Implements integer arithmetic operations (`+`, `-`, `*`), custom ASCII-to-Integer string parsing, and button click collision bounds.
* **Paint (`paint.c`):** Canvas drawing utility operating over physical memory buffer `0x280000`. Supports color selection, adjustable brush sizes (1px–8px), canvas resets, and ATA write signals.
* **Notepad (`notepad.c`):** Graphical text editor providing line wrapped text rendering and direct persistent disk sector saving via ATA driver calls (Target Sector 20).
* **File Manager (`filemgr.c`):** Static file structure representation displaying system image allocations (`SYSTEM.SYS`, `DWM.BIN`, `CONFIG.DAT`, `BOOT.BIN`).
* **Settings (`settings.c`):** System configuration utility allowing dynamic modifications to desktop background colors, pattern toggles, mouse pointer speeds/trails, resolution parameters, and RAM limits.
* **Task Manager (`taskman.c`):** Process tracking dashboard displaying running application titles, virtual PIDs, memory stability states, CPU utilization metrics derived from timer ticks, and manual process termination ("Kill") functions.
* **System Version (`cver.c`):** Displays kernel version string, architectural target, and copyright metadata.

---

## 5. File & Directory Layout

```text
CCORE-X86/
├── System/
│   ├── Boot.asm         # 16-bit assembly bootloader & mode switcher
│   ├── Kernel.asm       # 32-bit kernel assembly entry point
│   ├── DWM.c            # Desktop window manager, drivers & core loop
│   └── DWM.h            # System headers, macros & interface structures
└── Apps/
    ├── calc.c           # Desktop calculator application
    ├── cver.c           # System version & metadata dialog
    ├── filemgr.c        # Static file manager utility
    ├── notepad.c        # Text editor with ATA write integration
    ├── paint.c          # Pixel paint application with canvas buffer
    ├── settings.c       # System customization & hardware control panel
    └── taskman.c        # Process manager & uptime monitor

```

---

## 6. Snapshots

* Snapshot 1
* Snapshot 2
* Snapshot 2 Stable
* Snapshot 3

---


## 7. Build & Execution Instructions

### Prerequisites

* **NASM** (Netwide Assembler)
* **GCC** (Supporting 32-bit target `-m32` or `i386-elf-gcc`)
* **GNU Linker (`ld`)**
* **QEMU** (`qemu-system-i386`)

### Building the Disk Image

1. Assemble the Boot sector binary:
```bash
nasm -f bin System/Boot.asm -o boot.bin

```


2. Assemble the Kernel entry code:
```bash
nasm -f elf32 System/Kernel.asm -o kasm.o

```


3. Compile C sources to freestanding 32-bit ELF objects:
```bash
gcc -m32 -ffreestanding -fno-pie -c System/DWM.c -o dwm.o
gcc -m32 -ffreestanding -fno-pie -c Apps/calc.c -o calc.o
gcc -m32 -ffreestanding -fno-pie -c Apps/cver.c -o cver.o
gcc -m32 -ffreestanding -fno-pie -c Apps/filemgr.c -o filemgr.o
gcc -m32 -ffreestanding -fno-pie -c Apps/notepad.c -o notepad.o
gcc -m32 -ffreestanding -fno-pie -c Apps/paint.c -o paint.o
gcc -m32 -ffreestanding -fno-pie -c Apps/settings.c -o settings.o
gcc -m32 -ffreestanding -fno-pie -c Apps/taskman.c -o taskman.o

```


4. Link objects into flat binary format at base address `0x1000`:
```bash
ld -m elf_i386 -Ttext 0x1000 --oformat binary kasm.o dwm.o calc.o cver.o filemgr.o notepad.o paint.o settings.o taskman.o -o kernel.bin

```


5. Combine bootloader and kernel into a raw bootable disk image:
```bash
cat boot.bin kernel.bin > ccore.img

```


6. Execute within QEMU emulator:
```bash
qemu-system-i386 -drive format=raw,file=ccore.img

```



---
## 8. License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. Full terms are available in the `LICENSE` file.

Copyright (C) 2026 Ege Önder
