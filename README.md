# BasicOS
Simple bootloader + kernel operating system that allows basic filewriting in a RAM-based disk.

Bootloader in assembly that loads the kernel
Kernel written in C running in protected mode
VGA text output with menu interface
Simulated file storage in RAM 
File operations: open, create new, edit

Requirements:
- cross compilier toolchain, i used i686-elf-gcc
- nasm assembler for bootloader assembly
- qemu or other x86 emulator for testing

Assembling:
- assemble boot.asm -> nasm -f bin boot.asm -o boot.bin
- kernel.c to kernel.o -> i686-elf-gcc -m32 -ffreestanding -c kernel.c -o kernel.o
- kernel.o to kernel.bin at 0x10000 -> i686-elf-ld -Ttext 0x10000 binary -e _start kernel.o -o kernel.bin
- assemble to bin image -> cat boot.bin kernel.bin > os-image.bin
- run in qemu -> qemu-system-i386 -drive format=raw,file=os-image.bin
