[org 0x7c00] //where 2 load
bits 16 //real mode

start:
    cli                 ;no interrupts

    xor ax, ax
    mov ds, ax          ;data seg 0
    mov es, ax          ;extra seg 0
    mov ss, ax          ;stack seg 0
    mov sp, 0x7c00      ;stack pointer here so no overwriting

    ;al to debug successfull bootload
    mov ah, 0x0E
    mov al, ''
    int 0x10

    ;save boot drive # for later use
    mov [boot_drive], dl

    ;where 2 load kernel
    mov ax, 0x1000 
    mov es, ax
    xor bx, bx          ;offset 0x0

    mov ah, 0x02        ;BIOS read sectors fn
    mov al, 3           ;3 sectors
    mov ch, 0           ;cylinder 0
    mov cl, 2           ;sector 2 (start of kernel)
    mov dh, 0           ;head 0
    mov dl, [boot_drive];drive #
read_loop:
    int 0x13            ;disk read
    jc disk_error       ;in case error 

    ;GDT so CPU knows how to understand mem regions
    lgdt [gdt_descriptor]

    ;protected mode by turning on CR0 
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;to protected mode code and switched segemnt
    jmp 0x08:init_pm   

disk_error: ;show E if error
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    jmp $

bits 32 ;32 bit mode
init_pm:
    ;set up data seg registers
    mov ax, 0x10        ;data seg selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax          ;set stack segment

    mov esp, 0x90000    ;set stack pointer somewhere safe

    jmp 0x10000         ;to kernel code

hang:
    jmp hang

;gdt tells cpu ab memory parts to use
gdt_start:
    dq 0x0000000000000000                ;empty spot
    dq 0x00CF9A000000FFFF                ;code part for programs 4 gb
    dq 0x00CF92000000FFFF                ;data part for storing data 4 gb
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ;size of gdt - 1
    dd gdt_start ;where gdt starts

boot_drive: db 0 ;what drive booted from

CODE_SEG equ 0x08 ;use code part
DATA_SEG equ 0x10 ;use data part

;fill up the rest to finish boot
times 510 - ($ - $$) db 0
dw 0xAA55
