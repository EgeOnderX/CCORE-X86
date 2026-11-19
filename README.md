# CCORE X86

**A 32-bit bare-metal operating system with a custom graphical desktop environment, hardware drivers, persistent storage, and integrated system utilities.**

CCORE X86 is an experimental open-source operating system developed for the x86 architecture. Written primarily in C and x86 Assembly, it operates directly on hardware without relying on an underlying host operating system.

The project combines a custom bootloader, a 32-bit protected-mode kernel, a graphical desktop environment, low-level hardware drivers, a persistent disk-based file system, and a collection of integrated applications.

**Development Progress — v0.2**

Version 0.2 represents a significant architectural and codebase improvement over earlier snapshots. A substantial portion of legacy non-functional components, static demonstrations, placeholder implementations, and inadequately developed AI-generated code has been removed or rewritten.

Previously static components have increasingly been replaced with functional implementations, including actual ATA-backed file operations, persistent document storage, account management, and an integrated command interpreter.

AI-assisted development was used during the project's initial stages. However, the codebase has undergone substantial manual restructuring, cleanup, and implementation work to establish a more consistent and maintainable low-level architecture. This transition is ongoing.

---

<img width="728" height="485" alt="image" src="https://github.com/user-attachments/assets/cf7dc3fc-96d2-4e0f-a0f9-afc020e1b9f7" />

---

<img width="644" height="411" alt="Ekran Görüntüsü - 2026-11-19 19-41-02" src="https://github.com/user-attachments/assets/09b35e5f-c49d-4bb1-9181-99840a91181c" />

---

## 1. Core Architecture

CCORE X86 uses a custom boot process that transitions the processor from 16-bit Real Mode into 32-bit Protected Mode.

### 1.1 Bootloader & Protected Mode

**File:** `System/Boot.asm`

The custom bootloader is responsible for:

- BIOS-based disk initialization.
- Extended INT 13h disk access using LBA addressing.
- Loading the kernel into physical memory at `0x10000`.
- Initializing VGA Mode 13h (320×200, 256 colors).
- Enabling the A20 address line.
- Loading the Global Descriptor Table (GDT).
- Switching the CPU into 32-bit Protected Mode.
- Initializing segment registers and the kernel stack.
- Transferring execution to the kernel entry point.

### 1.2 Kernel Initialization

**File:** `System/Kernel.asm`

The kernel entry point establishes the fundamental execution environment.

Implemented components include:

- Interrupt Descriptor Table (IDT).
- Interrupt Service Routine (ISR) infrastructure.
- Programmable Interrupt Controller (PIC) remapping.
- Programmable Interval Timer (PIT) configuration.
- Hardware interrupt handling.
- Keyboard, mouse, and timer interrupt routing.
- BSS memory initialization.
- Kernel entry and processor halt handling.

The kernel is compiled as a freestanding 32-bit binary without depending on a standard host operating system runtime.

---

## 2. Graphical Desktop Environment

**Files:** `System/DWM.c` and `System/DWM.h`

CCORE features a custom Windows 3.1-inspired graphical desktop environment.

The Desktop Window Manager (DWM) provides the primary graphical interface and manages desktop rendering, application windows, input events, and system interactions.

### 2.1 Graphics System

- Software backbuffer rendering.
- VGA Mode 13h support.
- Bochs/QEMU VBE linear framebuffer support.
- 256-color graphics.
- Built-in bitmap font rendering.
- Clipping and drawing primitives.
- Window borders, buttons, and interface elements.
- Desktop wallpaper and graphical themes.

### 2.2 Display Resolutions

| Resolution | Display Mode |
|---|---|
| 320×200 | VGA Mode 13h |
| 640×480 | VBE Linear Framebuffer |
| 720×480 | VBE Linear Framebuffer |

The default display mode is 320×200. Higher resolutions require compatible VBE hardware emulation.

### 2.3 Window Management

- Multiple application windows.
- Window focusing and activation.
- Window closing.
- Window maximization and restoration.
- Application title management.
- Mouse-driven window interaction.
- Desktop icons and taskbar.
- Start menu and application launching.

### 2.4 Desktop Customization

Users can configure:

- Desktop background colors.
- Wallpaper patterns.
- Window themes.
- Mouse speed.
- Mouse trails and trail length.
- Desktop icon arrangement.
- Display resolution.

Configuration changes are stored in the persistent system configuration area.

---

## 3. Hardware Drivers & System Interfaces

CCORE implements low-level hardware communication through direct port I/O and processor instructions.

### 3.1 ATA PIO Storage Driver

The ATA driver communicates with the primary ATA interface using 28-bit LBA addressing.

Implemented functionality:

- ATA device identification.
- Disk model detection.
- Disk capacity detection.
- Sector-based reading.
- Sector-based writing.
- Device status polling.
- Write-cache flushing.

The storage driver provides the foundation for the CCORE file system.

### 3.2 PS/2 Input Drivers

**Keyboard:**
- IRQ1 interrupt handling.
- Keyboard scan-code processing.
- Character translation.
- Shift and Caps Lock handling.
- Keyboard input buffering.

**Mouse:**
- IRQ12 interrupt handling.
- PS/2 packet decoding.
- Mouse movement tracking.
- Button state detection.
- Screen boundary handling.
- Configurable mouse speed and trails.

### 3.3 CMOS / Real-Time Clock

The system communicates with the CMOS RTC through ports `0x70` and `0x71`.

Supported operations include:

- Reading system date.
- Reading system time.
- BCD-to-binary conversion.
- Date and time configuration.
- Displaying time information in the desktop environment.

### 3.4 Processor & System Information

The system provides hardware and runtime information through its startup diagnostics and integrated utilities.

---

## 4. CCORE File System (CFS)

CCORE includes a custom disk-based file system implemented directly on top of the ATA PIO driver.

Unlike the previous static file representations, the current File Manager operates on actual disk-backed directory entries and file data.

### 4.1 Storage Architecture

The disk image uses the following layout:

| LBA Range | Purpose |
|---|---|
| 0 | Bootloader |
| 1–256 | Kernel image |
| 300 | File system superblock and configuration |
| 301–304 | Directory metadata |
| 400+ | File data storage |

Files use fixed storage allocations of eight sectors per file.

### 4.2 File Operations

Implemented file system operations include:

- File creation.
- File deletion.
- File reading.
- File writing.
- File renaming.
- File copying.
- Directory scanning.
- System file identification.
- Persistent metadata updates.

The file system supports up to 64 file entries.

### 4.3 System File Protection

System files can be marked with the `S` attribute.

Administrator privileges are required to modify, rename, copy, or delete protected system files.

This mechanism provides basic access control within the operating system.

**Note:** CFS is a custom file system and is not compatible with standard FAT or NTFS volumes.

---

## 5. Built-in Applications

CCORE includes several integrated graphical applications.

Unlike earlier snapshots, the current application set contains substantially more functional implementations and direct integration with the kernel and storage subsystems.

### 5.1 Command Prompt (`cmd.c`)

A DOS-inspired command interpreter integrated into the graphical desktop.

Supported commands include:

| Command | Description |
|---|---|
| HELP | Display available commands |
| CLS | Clear the console |
| VER | Display OS version |
| DIR | List stored files |
| TYPE | Display file contents |
| COPY | Copy files |
| DEL | Delete files |
| REN | Rename files |
| ECHO | Display or write text |
| DATE | Display system date |
| TIME | Display system time |
| MEM | Display memory information |
| VOL | Display disk information |
| CHKDSK | Display file system information |
| WHOAMI | Display current account |
| TASKLIST | List running tasks |
| TASKKILL | Terminate an application |
| START | Launch an application |
| EDIT | Open a file in Notepad |
| ATTRIB | Display or modify file attributes |
| COLOR | Change console colors |
| TITLE | Change console window title |
| RUNAS | Request administrator privileges |
| SHUTDOWN | Shut down the system |
| RESTART | Restart the system |

Additional functionality includes:

- Command history and keyboard interaction.
- Application launching.
- Administrator elevation.
- System file access control.
- Basic `AUTOEXEC.BAT` command processing.

### 5.2 Calculator (`calc.c`)

A graphical calculator featuring:

- Addition.
- Subtraction.
- Multiplication.
- Division.
- Fixed-point arithmetic with three decimal places.
- Overflow detection.
- Keyboard and mouse input.

### 5.3 File Manager (`filemgr.c`)

A graphical interface for managing files stored on the ATA-backed CFS file system.

Features include:

- Viewing actual directory entries.
- Displaying file names and sizes.
- Creating files.
- Opening supported text files.
- Copying files.
- Managing system file attributes.
- Administrator permission enforcement.

### 5.4 Notepad (`notepad.c`)

A graphical text editor integrated with the CCORE file system.

Features include:

- Creating documents.
- Opening existing files.
- Saving documents directly to disk.
- Multiline text editing.
- Cursor navigation.
- Backspace and Delete support.
- Line navigation and scrolling.
- Document modification tracking.

Documents are stored persistently through the ATA storage subsystem.

### 5.5 Paint (`paint.c`)

A graphical pixel-based drawing application.

Features include:

- Canvas-based drawing.
- Mouse-driven drawing operations.
- Line drawing.
- Flood-fill functionality.
- 16/256-color palette support.
- Adjustable brush sizes.
- Canvas operations.

### 5.6 Settings (`settings.c`)

The integrated system configuration utility.

Available configuration sections:

- Graphics.
- Personalization.
- Mouse.
- Drivers.
- System.

Users can modify display settings, themes, wallpaper, mouse behavior, and account configuration.

### 5.7 Task Manager (`taskman.c`)

A graphical system monitoring utility.

Features include:

- Running application enumeration.
- Process identification.
- PID display.
- Memory usage information.
- Task state information.
- System uptime.
- Application termination.

The kernel desktop manager is represented as PID 0. Terminating the protected DWM system task triggers a critical system stop.

### 5.8 System Information (`cver.c`)

Displays operating system version information, architecture details, and system metadata.

---

## 6. User Accounts & Security Model

CCORE includes a basic account and privilege management system.

Implemented functionality:

- First-login account creation.
- User authentication.
- Guest login.
- Administrator account.
- Password-based authentication.
- Administrator privilege elevation.
- Protected system file attributes.
- Restricted system operations.

The administrator account is named `admin`.

The default administrator password is `admin` when the system has not been configured through the Setup utility.

This is a basic hobby-OS permission model, not a production-grade security architecture.

---

## 7. CCORE Setup

CCORE includes a separate graphical installation utility located in the `Setup/` directory.

The Setup environment provides a first-run configuration wizard.

### Setup Features

- Display resolution selection.
- Administrator password configuration.
- User account creation.
- Date and time configuration.
- Desktop personalization.
- Disk installation.
- Installation progress display.
- Automatic system restart.

The installer writes the configured CCORE kernel and bootloader to the designated disk image and initializes the CFS storage area.

For additional information, see `Setup/README.md`.

---

## 8. Project Structure

```text
CCORE-X86/
│
├── System/
│   ├── Boot.asm
│   ├── Kernel.asm
│   ├── DWM.c
│   └── DWM.h
│
├── Apps/
│   ├── calc.c
│   ├── cver.c
│   ├── cmd.c
│   ├── filemgr.c
│   ├── notepad.c
│   ├── paint.c
│   ├── settings.c
│   └── taskman.c
│
├── Setup/
│   ├── Makefile
│   ├── README.md
│   └── Setup.c
│
├── Makefile
├── linker.ld
├── README.md
└── LICENSE
```

---

## 9. Build Requirements

The following development tools are required:

- GCC with 32-bit compilation support.
- NASM (Netwide Assembler).
- GNU Binutils (LD and OBJCOPY).
- GNU Make.
- QEMU System Emulator.

The project targets the 32-bit i386 architecture and uses freestanding compilation settings.

### 9.1 Build CCORE

Clone the repository:

```bash
git clone https://github.com/EgeOnderX/CCORE-X86.git
cd CCORE-X86
```

Compile the operating system and generate the disk image:

```bash
make
```

### 9.2 Run in QEMU

```bash
make run
```

The default QEMU configuration uses:

- i386 system emulation.
- 128 MB RAM.
- Standard VGA emulation.
- An IDE-attached raw disk image.

### 9.3 Clean Build Files

Remove generated build objects:

```bash
make clean
```

Remove build objects and the generated disk image:

```bash
make distclean
```

**Important:** The standard build preserves the existing disk image and its persistent data. Use `make distclean` when a completely fresh disk image is required.

### 9.4 Build the Installer

From the repository root:

```bash
cd Setup
make
```

Run the installer:

```bash
make run
```

To recreate the installer environment:

```bash
make reinstall
```

---

## 10. Development Status

CCORE X86 is an actively developed experimental operating system.

Version 0.2 introduces a substantial cleanup and architectural improvement compared with the earlier snapshots.

| Component | Status |
|---|---|
| Custom Bootloader | Implemented |
| 32-bit Protected Mode | Implemented |
| IDT / ISR Infrastructure | Implemented |
| PIC / PIT | Implemented |
| VGA Graphics | Implemented |
| VBE Framebuffer | Implemented |
| Desktop Window Manager | Implemented |
| PS/2 Keyboard | Implemented |
| PS/2 Mouse | Implemented |
| ATA PIO Driver | Implemented |
| Custom CFS File System | Implemented |
| Persistent File Operations | Implemented |
| User Accounts | Implemented |
| Administrator Permissions | Implemented |
| Command Prompt | Implemented |
| Graphical Applications | Implemented |
| Setup Utility | Implemented |

The project is not intended to be a replacement for mature operating systems yet. Features such as a virtual memory manager, full hardware compatibility, and a production-grade security architecture remain outside the current implementation.

---

## 11. License

CCORE X86 is distributed under the **GNU General Public License v3.0 (GPL-3.0)**.

See the `LICENSE` file for the complete license terms.

---

## 12. Acknowledgements

CCORE X86 is an independent hobby operating system development project by **Ege Önder**.

The project focuses on learning, implementing, and improving low-level operating system concepts, hardware interaction, kernel architecture, and graphical system design.

**Copyright (C) 2026 Ege Önder**

