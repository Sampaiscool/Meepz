# Objecten
OBJS = kernel_entry.o kernel.o idt.o gdt.o memory.o screen.o fs.o commands.o shell.o editor.o keyboard.o ata.o

all: meepz.iso

kernel_entry.o: kernel_entry.asm
	nasm -f elf32 kernel_entry.asm -o kernel_entry.o

kernel.o: src/kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/kernel.c -o kernel.o

idt.o: src/cpu/idt.c src/cpu/idt.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/cpu/idt.c -o idt.o

gdt.o: src/cpu/gdt.c src/cpu/gdt.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/cpu/gdt.c -o gdt.o

memory.o: src/lib/memory.c src/lib/memory.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/lib/memory.c -o memory.o

screen.o: src/lib/screen.c src/lib/screen.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/lib/screen.c -o screen.o

fs.o: src/fs/fs.c src/fs/fs.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/fs/fs.c -o fs.o

commands.o: src/shell/commands.c src/shell/commands.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/shell/commands.c -o commands.o

shell.o: src/shell/shell.c src/shell/shell.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/shell/shell.c -o shell.o

editor.o: src/shell/editor.c src/shell/editor.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/shell/editor.c -o editor.o

keyboard.o: src/drivers/keyboard.c src/drivers/keyboard.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/drivers/keyboard.c -o keyboard.o

ata.o: src/drivers/ata.c src/drivers/ata.h
	gcc -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -I src -c src/drivers/ata.c -o ata.o

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

run: meepz.iso disk.img
	qemu-system-i386 -cdrom meepz.iso -drive file=disk.img,format=raw,if=ide -display sdl

disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=32

clean:
	rm -rf *.o kernel.elf meepz.iso iso
