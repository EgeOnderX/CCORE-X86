; ============================================================
; CCORE v0.2 - Boot.asm : 16-bit MBR bootloader / mode switcher
; Loads kernel (LBA 1..256) to 0x10000, sets VGA mode 13h,
; enables A20, loads GDT, enters protected mode, jumps to kernel.
; ============================================================
[bits 16]
[org 0x7C00]

KERNEL_SEG      equ 0x1000          ; segment -> physical 0x10000
KERNEL_ENTRY    equ 0x10000
KERNEL_SECTORS  equ 256
CHUNK           equ 64              ; sectors per INT 13h ext read

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    mov [boot_drive], dl

    mov cx, KERNEL_SECTORS / CHUNK
.load:
    push cx
    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error
    add word [dap_seg], CHUNK * 512 / 16
    add word [dap_lba], CHUNK
    pop cx
    loop .load

    mov ax, 0x0013                  ; VGA 320x200x256
    int 0x10

    mov ax, 0x2401                  ; A20 via BIOS (ignore result)
    int 0x15
    in al, 0x92                     ; A20 fast gate
    or al, 2
    and al, 0xFE
    out 0x92, al

    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry

disk_error:
    mov si, msg_err
.p: lodsb
    test al, al
    jz .h
    mov ah, 0x0E
    int 0x10
    jmp .p
.h: cli
    hlt
    jmp .h

[bits 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp 0x08:KERNEL_ENTRY

[bits 16]
boot_drive: db 0
msg_err:    db "CCORE: disk read error", 0

dap:        db 0x10, 0
            dw CHUNK
            dw 0x0000
dap_seg:    dw KERNEL_SEG
dap_lba:    dd 1
            dd 0

align 8
gdt_start:
    dq 0
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00       ; 0x08 code  (flat 4GB)
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00       ; 0x10 data  (flat 4GB)
gdt_end:
gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55
