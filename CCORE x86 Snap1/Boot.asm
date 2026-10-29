[ORG 0x7C00]
[BITS 16]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [BOOT_DRIVE], dl

    ; 1. Sürücüyü Sıfırla
    xor ax, ax
    mov dl, [BOOT_DRIVE]
    int 0x13

    ; 2. Kernel Sektörlerini Döngü İle Oku (40 Sektör = 20 KB)
    mov word [buffer_ptr], 0x1000
    mov word [lba_sector], 1
    mov cx, 40

read_loop:
    push cx
    mov ax, [lba_sector]
    call lba_to_chs
    mov dl, [BOOT_DRIVE]

    mov ax, 0x0201
    mov bx, [buffer_ptr]
    int 0x13
    jc disk_error

    add word [buffer_ptr], 512
    inc word [lba_sector]
    pop cx
    loop read_loop

    ; 3. VBE 640x480 256 Renk Modunu Aç (Mode 0x4101)
    xor ax, ax
    mov es, ax
    mov ax, 0x4F01
    mov cx, 0x0101
    mov di, 0x8000
    int 0x10

    mov eax, [es:0x8028]
    mov [0x7E00], eax

    mov ax, 0x4F02
    mov bx, 0x4101
    int 0x10

    ; 4. Protected Mode'a Geçiş
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:init_pm

lba_to_chs:
    push ax
    mov bx, 18
    xor dx, dx
    div bx
    inc dx
    mov cl, dl

    mov bx, 2
    xor dx, dx
    div bx
    mov ch, al
    mov dh, dl
    pop ax
    ret

disk_error:
    cli
    hlt

[BITS 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x1000

gdt_start:
    dd 0x0, 0x0
gdt_code:
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
gdt_data:
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BOOT_DRIVE  db 0
buffer_ptr  dw 0x1000
lba_sector  dw 1

times 510 - ($ - $$) db 0
dw 0xAA55