; ============================================================
; CCORE v0.2 - Kernel.asm : 32-bit entry, IDT, PIC, PIT, ISR stubs
; (generated stub list is plain NASM, edit freely)
; ============================================================
[bits 32]
global _start
extern kernel_main
extern isr_dispatch
extern __bss_start
extern __bss_end

%macro ISR_NOERR 1
isr%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro
%macro ISR_ERR 1
isr%1:
    push dword %1
    jmp isr_common
%endmacro
%macro IRQ 2
isr%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro
%macro IOWAIT 0
    out 0x80, al
%endmacro

section .text.entry progbits alloc exec nowrite align=16
_start:
    cli
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    cld
    mov edi, __bss_start            ; BIOS only loaded file bytes: clear .bss
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    call setup_idt
    call remap_pic
    call setup_pit
    call kernel_main                ; never returns (interrupts still off; C does sti)
.halt:
    cli
    hlt
    jmp .halt

section .text progbits alloc exec nowrite align=16
setup_idt:
    xor ecx, ecx
.l:
    mov eax, [isr_table + ecx*4]
    lea edi, [idt + ecx*8]
    mov [edi], ax
    mov word [edi+2], 0x08
    mov byte [edi+4], 0
    mov byte [edi+5], 0x8E          ; present, ring0, 32-bit interrupt gate
    shr eax, 16
    mov [edi+6], ax
    inc ecx
    cmp ecx, 48
    jb .l
    lidt [idt_ptr]
    ret

remap_pic:
    mov al, 0x11
    out 0x20, al
    IOWAIT
    out 0xA0, al
    IOWAIT
    mov al, 0x20                    ; master vectors 32..39
    out 0x21, al
    IOWAIT
    mov al, 0x28                    ; slave vectors 40..47
    out 0xA1, al
    IOWAIT
    mov al, 4
    out 0x21, al
    IOWAIT
    mov al, 2
    out 0xA1, al
    IOWAIT
    mov al, 1
    out 0x21, al
    IOWAIT
    out 0xA1, al
    IOWAIT
    mov al, 0xF8                    ; unmask IRQ0 (PIT), IRQ1 (kbd), IRQ2 (cascade)
    out 0x21, al
    mov al, 0xEF                    ; unmask IRQ12 (PS/2 mouse)
    out 0xA1, al
    ret

setup_pit:                          ; 100 Hz
    mov al, 0x36
    out 0x43, al
    mov ax, 11932
    out 0x40, al
    mov al, ah
    out 0x40, al
    ret

isr_common:
    pushad
    push esp                        ; Regs* for C
    call isr_dispatch
    add esp, 4
    popad
    add esp, 8                      ; vector + error code
    iretd

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR 8
ISR_NOERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR 29
ISR_ERR 30
ISR_NOERR 31
IRQ 32, 0
IRQ 33, 1
IRQ 34, 2
IRQ 35, 3
IRQ 36, 4
IRQ 37, 5
IRQ 38, 6
IRQ 39, 7
IRQ 40, 8
IRQ 41, 9
IRQ 42, 10
IRQ 43, 11
IRQ 44, 12
IRQ 45, 13
IRQ 46, 14
IRQ 47, 15

section .data progbits alloc noexec write align=4
idt_ptr:
    dw 256*8-1
    dd idt
isr_table:
    dd isr0
    dd isr1
    dd isr2
    dd isr3
    dd isr4
    dd isr5
    dd isr6
    dd isr7
    dd isr8
    dd isr9
    dd isr10
    dd isr11
    dd isr12
    dd isr13
    dd isr14
    dd isr15
    dd isr16
    dd isr17
    dd isr18
    dd isr19
    dd isr20
    dd isr21
    dd isr22
    dd isr23
    dd isr24
    dd isr25
    dd isr26
    dd isr27
    dd isr28
    dd isr29
    dd isr30
    dd isr31
    dd isr32
    dd isr33
    dd isr34
    dd isr35
    dd isr36
    dd isr37
    dd isr38
    dd isr39
    dd isr40
    dd isr41
    dd isr42
    dd isr43
    dd isr44
    dd isr45
    dd isr46
    dd isr47

section .bss nobits alloc noexec write align=16
idt: resb 256*8

section .note.GNU-stack noalloc noexec nowrite progbits
