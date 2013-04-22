################################################################################
#                              MakeFile of ictOS                               #
################################################################################

# Tools setting
AS  = nasm
CC  = gcc
LD  = ld

# Compiling arguments Setting used by gcc
KERNEL_CC_ARG = -c -fno-stack-protector -I src/include/kernel/ -o
KERNEL_AS_ARG = -f elf -i src/include/kernel/ -o

# All files of this OS
ALL_FILES = boot/sysboot.bin \
            boot/bootblock.bin \
            kernel/sysldr.ict \
            kernel/kernel.ict

# Kernel objects write by asm
KERNEL_AS_OBJS = bin/kernel/kentry.o \
                 bin/kernel/kasm.o \
                 bin/kernel/intentry.o 

# Kernel objects write by c
KERNEL_C_OBJS = bin/kernel/kernel.o \
                bin/kernel/klib/protect.o \
                bin/kernel/klib/basic.o \
                bin/kernel/servers/keyboard.o \
                bin/kernel/servers/clock.o \
                bin/kernel/servers/msg.o \
                bin/kernel/servers/kproc.o \
                bin/kernel/servers/hd.o \
                bin/kernel/servers/mem.o \
                bin/kernel/servers/video.o \
                bin/kernel/servers/kfs.o

install : complie setup

complie : $(ALL_FILES) 

setup :
	cat bin/boot/sysboot.bin bin/boot/bootblock.bin > bin/boot/boot.bin
	dd if=bin/boot/boot.bin of=hd.img bs=4096 count=1 conv=notrunc
	sudo mount -o loop hd.img /mnt/hd
	sudo cp bin/kernel/sysldr.ict /mnt/hd/
	sudo cp bin/kernel/kernel.ict /mnt/hd/
	sleep 0.1s
	sudo umount /mnt/hd

clear :
	rm -f $(ALL_FILES) $(KERNEL_AS_OBJS) $(KERNEL_C_OBJS)

tree :
	tree -I *~ src/ > filetree.txt

boot/sysboot.bin :
	$(AS) -i src/include/boot/ -o bin/boot/sysboot.bin \
		src/boot/sysboot.s

boot/bootblock.bin :
	$(AS) -i src/include/boot/ -o bin/boot/bootblock.bin \
		src/boot/bootblock.s

kernel/sysldr.ict:
	$(AS) -i src/include/kernel/ -o bin/kernel/sysldr.ict \
		src/kernel/sysldr.s

kernel/kernel.ict :
	$(AS) $(KERNEL_AS_ARG) bin/kernel/kentry.o src/kernel/kentry.s
	$(AS) $(KERNEL_AS_ARG) bin/kernel/kasm.o src/kernel/kasm.s
	$(AS) $(KERNEL_AS_ARG) bin/kernel/intentry.o src/kernel/intentry.s
	$(CC) $(KERNEL_CC_ARG) bin/kernel/kernel.o src/kernel/kernel.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/klib/protect.o src/kernel/klib/protect.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/klib/basic.o src/kernel/klib/basic.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/keyboard.o src/kernel/servers/keyboard.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/clock.o src/kernel/servers/clock.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/msg.o src/kernel/servers/msg.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/kproc.o src/kernel/servers/kproc.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/hd.o src/kernel/servers/hd.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/mem.o src/kernel/servers/mem.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/video.o src/kernel/servers/video.c
	$(CC) $(KERNEL_CC_ARG) bin/kernel/servers/kfs.o src/kernel/servers/kfs.c
	$(LD) -s -Ttext 0x501000 -o bin/kernel/kernel.ict $(KERNEL_AS_OBJS) $(KERNEL_C_OBJS)
