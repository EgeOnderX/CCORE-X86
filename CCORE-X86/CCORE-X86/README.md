# CCORE v0.2 (x86, 32-bit)

Zero-dependency hobby OS with a Windows 3.1-style desktop. NASM + freestanding GCC + LD.

    make        # builds ccore.img (disk is created once; data area survives rebuilds)
    make run    # qemu-system-i386 -drive file=ccore.img,format=raw -m 128 -vga std

Layout: `System/` (Boot.asm, Kernel.asm, DWM.c/h) and `Apps/` (calc, cver, cmd, filemgr, notepad, paint, settings, taskman).
Disk map: LBA 0 boot, 1-256 kernel, 300 superblock (accounts/settings/icons), 301-304 directory, 400+ file data (8 sectors/file).

Accounts: the first login creates the user account (whatever you enter). The administrator is `admin`
(password set by Setup; `admin` if Setup was not used). Only the administrator can edit, copy, rename or delete
system files (marked S) and end the DWM.SYS task. Ending DWM.SYS crashes the system with the
stop code CRITICAL_PROCESS_DIED (blue screen).

Resolutions: 320x200 VGA (mode 13h) plus 640x480 and 720x480 (Bochs/QEMU VBE linear framebuffer, needs `-vga std`).
Command Prompt (cmd.c): HELP DIR TYPE COPY DEL REN ECHO DATE TIME MEM VOL CHKDSK WHOAMI TASKLIST TASKKILL START ATTRIB COLOR TITLE RUNAS ...
Requires: nasm, gcc (with -m32 support), binutils, make, qemu-system-i386.
