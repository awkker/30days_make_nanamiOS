# 定义工具
CC=i686-elf-gcc
LD=i686-elf-ld
NASM=nasm
# 默认目标
all: op.img

# 规则：如何运行
run: op.img
	qemu-system-i386 -fda op.img

# 规则：如何制作 1.44MB 的 'op.img'
# 依赖于 512 字节的 'op.bin'
op.img: op.bin kernel.bin
	# 1. 创建空软盘
	dd if=/dev/zero of=op.img bs=1k count=1440
	# 2. 写入启动区
	dd if=op.bin of=op.img bs=512 seek=0 conv=notrunc
	# 3. 写入 C 语言内核 (还是老位置 0x4200)
	dd if=kernel.bin of=op.img bs=1 seek=16896 conv=notrunc

op.bin: op.asm
	$(NASM) -f bin op.asm -o op.bin -l op.lst


# ----------------------------------------------------
# 6. ★★★ C 语言内核的全新编译流程 ★★★
# ----------------------------------------------------

# (6a) C 语言内核 (kernel.bin) 依赖于 C 和 汇编 的 .o 文件
#      我们用 ld (链接器) 把它们链接在一起
kernel.bin: bootpack.o naskfunc.o
	$(LD) -m elf_i386 -T bootpack.ld -o kernel.bin bootpack.o naskfunc.o
	
# (6b) 编译 C 代码 (bootpack.c -> bootpack.o)
#      我们用 gcc (C 编译器)
bootpack.o: bootpack.c
	$(CC) -m32 -fno-pie -O0 -std=c99 -c bootpack.c -o bootpack.o

# (6c) 编译 C 的汇编助手 (naskfunc.nas -> naskfunc.o)
#      我们用 nasm
naskfunc.o: naskfunc.nas
	$(NASM) -f elf32 naskfunc.nas -o naskfunc.o -l naskfunc.lst

# 7. 清理规则 (现在要多清理 C 的文件了)
clean:
	rm -f op.bin op.lst op.img
	rm -f kernel.bin *.o *.lst

