# 这是一个 Makefile

# 默认目标：运行 make 就等于运行 make all
all: helloos.img

# 规则：如何从 .asm 生成 .img
helloos.img: helloos.asm
	nasm -f bin helloos.asm -o helloos.img

# 规则：如何运行
run: all
	qemu-system-i386 -fda helloos.img

# 规则：如何清理
clean:
	rm -f helloos.img