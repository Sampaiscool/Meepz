all: os.bin

# compile kernel C code to object file (no linking yet)
kernel.o: kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -c kernel.c -o kernel.o

idt.o: idt.c
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -c idt.c -o idt.o

kernel_entry.o: kernel_entry.asm
	nasm -f elf kernel_entry.asm -o kernel_entry.o

# assemble kernel entry stub
kernel.bin: kernel_entry.o kernel.o idt.o
	ld -m elf_i386 -o kernel.bin -Ttext 0x1000 -e kernel_entry kernel_entry.o kernel.o idt.o --oformat binary

# assemble bootloader
boot.bin: boot.asm
	nasm -f bin boot.asm -o boot.bin

# combine bootloader + kernel into one disk image
os.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os.bin

run: os.bin
	ghostty -e qemu-system-i386 -drive format=raw,file=os.bin -display curses -no-reboot -no-shutdown
