# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard kernel/*.c include/*.c kernel/int/*.c)
ASM_SOURCES = $(wildcard kernel/int/*.asm)
HEADERS = $(wildcard kernel/*.h kernel/int/*.h include/*.h)

# Convert the *.c filenames to *.o to give a list of object files to build
OBJC = ${C_SOURCES:.c=.o}
OBJA = ${ASM_SOURCES:.asm=.o}

QEMU = /mnt/a/Programs/QEMU/qemu-system-i386.exe -cdrom BluOS.iso

# Default build target
all: iso

run: all
	${QEMU}

debug: all
	${QEMU} -s -S & gdb --quiet

debugmon: all
	${QEMU} -monitor stdio

iso: BluOS.iso

BluOS.iso: kernel/kernel.elf iso/boot/grub/grub.cfg
	cp kernel/kernel.elf iso/boot/BluOS.elf
	grub-mkrescue -o BluOS.iso iso -- -quiet

kernel/kernel.elf: link.ld boot/grubboot.o ${OBJC} ${OBJA}
	ld -T link.ld -m elf_i386 -o kernel/kernel.elf boot/grubboot.o ${OBJC} ${OBJA}
	objcopy --only-keep-debug kernel/kernel.elf kernel/kernel.sym
	objcopy --strip-debug kernel/kernel.elf kernel/kernel.elf

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files.
%.o: %.c ${HEADERS}
	gcc -g3 -gdwarf-2 -fvar-tracking -fvar-tracking-assignments -m32 -ffreestanding -I include -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

# Clear away all generated files .
clean:
	rm -rf *.bin *.iso *.o *.tmp
	rm -rf kernel/*.o kernel/*.tmp kernel/*.bin kernel/*.sym kernel/*.elf
	rm -rf kernel/int/*.o
	rm -rf include/*.o
	rm -rf boot/*.bin boot/*.o