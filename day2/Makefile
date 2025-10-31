# Makefile (Day 3 现代化版本)

# 默认目标 (只敲 'make' 就执行 'make all')
# 'all' 依赖于 'helloos.img'
all: helloos.img

# 规则：如何运行 (依赖于 'helloos.img')
# 这代替了 'run.bat'
run: helloos.img
	qemu-system-i386 -fda helloos.img

# 规则：如何制作 'helloos.img' (依赖于 'ipl.bin')
# 这代替了 'makeimg.bat'
helloos.img: ipl.bin
	# 1. (edimg.exe 功能) 创建一个 1.44MB 全是 0 的空白软盘
	dd if=/dev/zero of=helloos.img bs=1k count=1440
	
	# 2. (edimg.exe 功能) 把 ipl.bin 写入到空白软盘的开头
	#    conv=notrunc 意思是“不要截断文件”，只覆盖开头 512 字节
	dd if=ipl.bin of=helloos.img conv=notrunc

# 规则：如何编译 'ipl.bin' (依赖于 'ipl.asm')
# 这代替了 'asm.bat'
ipl.bin: ipl.asm
	# -l ipl.lst 就是书中所说的“列表文件”
	nasm -f bin ipl.asm -o ipl.bin -l ipl.lst

# 规则：如何清理
clean:
	# 现在我们要删除 3 个生成的文件
	rm -f ipl.bin ipl.lst helloos.img