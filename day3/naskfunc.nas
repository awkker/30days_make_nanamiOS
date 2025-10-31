; naskfunc (真正现代化的 NASM 语法)
; TAB=4

; ---------------------------------
; BITS, SECTION, GLOBAL 
; 都是 NASM 的标准指令，不能带方括号
; [FORMAT] 和 [FILE] 指令已被删除
; ---------------------------------

BITS 32         ; 告诉 NASM 编译 32 位代码
    
SECTION .text   ; 声明 .text (代码) 段
    
GLOBAL  io_hlt ; 声明 _io_hlt 是全局的，C 语言可以找到它
    
io_hlt:        ; C 语言的 void io_hlt(void)
    HLT
    RET         ; C 函数调用必须用 RET (Return) 返回

 