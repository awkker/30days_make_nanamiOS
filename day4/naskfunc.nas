; naskfunc.nas - 现代化的 NASM 语法
; TAB=4
; 提供操作系统开发所需的底层汇编函数

; ---------------------------------
; 说明：
; - 使用标准 NASM 语法，不使用旧的 NASK 专有指令
; - 所有函数使用 cdecl 调用约定（参数从右到左压栈，调用者清理栈）
; ---------------------------------

BITS 32         ; 32 位保护模式代码
SECTION .text   ; 代码段

; ---------------------------------
; 声明全局符号（导出给 C 语言使用）
; ---------------------------------
GLOBAL  io_hlt, io_cli, io_sti, io_stihlt
GLOBAL  io_in8, io_in16, io_in32
GLOBAL  io_out8, io_out16, io_out32
GLOBAL  io_load_eflags, io_store_eflags
GLOBAL  write_mem8

; =================================
; CPU 控制指令
; =================================

io_hlt:         ; void io_hlt(void);
    HLT         ; 暂停 CPU，等待下一次中断
    RET

io_cli:         ; void io_cli(void);
    CLI         ; 禁用中断（Clear Interrupt Flag）
    RET

io_sti:         ; void io_sti(void);
    STI         ; 启用中断（Set Interrupt Flag）
    RET

io_stihlt:      ; void io_stihlt(void);
    STI         ; 开中断
    HLT         ; 暂停（等待中断唤醒）
    RET

; =================================
; 端口输入指令（从设备读取数据）
; =================================

io_in8:         ; int io_in8(int port);
    MOV     EDX, [ESP+4]    ; EDX = port
    MOV     EAX, 0          ; 清空 EAX
    IN      AL, DX          ; 从端口读取 8 位数据到 AL
    RET

io_in16:        ; int io_in16(int port);
    MOV     EDX, [ESP+4]    ; EDX = port
    MOV     EAX, 0          ; 清空 EAX
    IN      AX, DX          ; 从端口读取 16 位数据到 AX
    RET

io_in32:        ; int io_in32(int port);
    MOV     EDX, [ESP+4]    ; EDX = port
    IN      EAX, DX         ; 从端口读取 32 位数据到 EAX
    RET

; =================================
; 端口输出指令（向设备写入数据）
; =================================

io_out8:        ; void io_out8(int port, int data);
    MOV     EDX, [ESP+4]    ; EDX = port
    MOV     AL, [ESP+8]     ; AL = data (低 8 位)
    OUT     DX, AL          ; 向端口输出 8 位数据
    RET

io_out16:       ; void io_out16(int port, int data);
    MOV     EDX, [ESP+4]    ; EDX = port
    MOV     EAX, [ESP+8]    ; EAX = data
    OUT     DX, AX          ; 向端口输出 16 位数据
    RET

io_out32:       ; void io_out32(int port, int data);
    MOV     EDX, [ESP+4]    ; EDX = port
    MOV     EAX, [ESP+8]    ; EAX = data
    OUT     DX, EAX         ; 向端口输出 32 位数据
    RET

; =================================
; EFLAGS 寄存器操作
; =================================

io_load_eflags:     ; int io_load_eflags(void);
    PUSHFD              ; 将 EFLAGS 压栈（PUSH EFLAGS Doubleword）
    POP     EAX         ; 弹出到 EAX 返回
    RET

io_store_eflags:    ; void io_store_eflags(int eflags);
    MOV     EAX, [ESP+4]    ; EAX = eflags
    PUSH    EAX             ; 压栈
    POPFD                   ; 从栈恢复到 EFLAGS（POP to EFLAGS Doubleword）
    RET

; =================================
; 内存操作辅助函数
; =================================

write_mem8:     ; void write_mem8(int addr, int data);
    MOV     ECX, [ESP+4]    ; ECX = addr
    MOV     AL, [ESP+8]     ; AL = data (低 8 位)
    MOV     [ECX], AL       ; 写入内存 [addr] = data
    RET
