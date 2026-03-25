# Objecten
OBJS = kernel_entry.o kernel.o idt.o gdt.o

all: meepz.iso

kernel_entry.o: kernel_entry.asm
	nasm -f elf32 kernel_entry.asm -o kernel_entry.o

kernel.o: kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -c kernel.c -o kernel.o

idt.o: idt.c
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -c idt.c -o idt.o

gdt.o: gdt.c
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -c gdt.c -o gdt.o

kernel.elf: $(OBJS)
	ld -m elf_i386 -T linker.ld -o kernel.elf $(OBJS)

meepz.iso: kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	@echo 'set timeout=0' > iso/boot/grub/grub.cfg
	@echo 'set default=0' >> iso/boot/grub/grub.cfg
	@echo 'menuentry "Meepz OS" {' >> iso/boot/grub/grub.cfg
	@echo '  multiboot /boot/kernel.elf' >> iso/boot/grub/grub.cfg
	@echo '  boot' >> iso/boot/grub/grub.cfg
	@echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o meepz.iso iso

run: meepz.iso
	qemu-system-i386 -cdrom meepz.iso -display sdl

clean:
	rm -rf *.o kernel.elf meepz.iso iso
