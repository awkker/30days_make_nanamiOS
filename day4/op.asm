; op.asm (Day 5 - 切换到 32 位保护模式)
; 完整、可启动的 512 字节版本

BITS 16
ORG  0x7c00

; ---------------------------------
; 定义常量
; ---------------------------------
CYLS EQU 10         ; 我们假设先只读 10 个柱面

; ---------------------------------
; FAT12 软盘头部 (BPB)
; ---------------------------------
    JMP entry
    DB 0x90
    DB "HELLOIPL" ; 厂商名 (8 字节)
    DB 0x00, 0x02, 0x01, 0x01, 0x00
    DB 0x02, 0xe0, 0x00, 0x40, 0x0b, 0xf0, 0x09, 0x00
    DB 0x12, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
    DB 0x40, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x29, 0xff
    DB 0xff, 0xff, 0xff, "HELLO-OS   " ; 卷标 (11 字节)
    DB "FAT12   " ; 文件系统类型 (8 字节)
    DB 0x00, 0x00
    RESB 16     ; 保留 16 字节

; ---------------------------------
; 16 位模式程序核心
; ---------------------------------
entry:
    ; 初始化寄存器 (设置栈)
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX

; ---------------------------------
; 读磁盘 (加载内核)
; ---------------------------------
    MOV AX,0x0820
    MOV ES,AX
    MOV CH,0 ; 柱面0
    MOV DH,0 ; 磁头0
    MOV CL,2 ; 扇区2
readloop:
    MOV SI,0 ; 记录失败次数的寄存器
retry:
    MOV AH,0x02 ; AH=0x02 : 读入磁盘
    MOV AL,1 ; 1个扇区
    MOV BX,0
    MOV DL,0x00 ; A驱动器
    INT 0x13 ; 调用磁盘BIOS
    JNC next
    ADD SI,1
    CMP SI,5
    JAE error
    MOV AH,0x00
    MOV DL,0x00
    INT 0x13
    JMP retry
error:
    JMP fin  

next:
    MOV AX,ES
    ADD AX, 512/16
    MOV ES,AX
    ADD CL,1
    CMP CL,18
    JBE readloop
    MOV CL,1
    ADD DH,1
    CMP DH,2
    JB readloop
    MOV DH,0
    ADD CH,1
    CMP CH,CYLS
    JB readloop

; ---------------------------------
; ★★★ 切换到 32 位保护模式 ★★★
; ---------------------------------

    ; 1. 写入 CYLS 信息
    MOV WORD [0x0ff0], CYLS

    ; 2. 键盘控制器：打开 A20 Gate
    ;    (允许 CPU 访问 1MB 以上内存)
wait_KBC_sendready:
    IN  AL, 0x64
    AND AL, 0x02
    JNE wait_KBC_sendready
    MOV AL, 0xd1
    OUT 0x64, AL
wait_KBC_sendready_2:
    IN  AL, 0x64
    AND AL, 0x02
    JNE wait_KBC_sendready_2
    MOV AL, 0xdf  ; 0xdf = 开启 A20
    OUT 0x60, AL
wait_KBC_sendready_3:
    IN  AL, 0x64
    AND AL, 0x02
    JNE wait_KBC_sendready_3

    ; 3. 加载 GDT (全局描述符表)
    ;    GDT 告诉 CPU 32位模式下的内存布局
    LGDT [GDTR0]

    ; 4. 拨动开关！设置 CR0 寄存器的 PE 位
    ;    PE=1 (Protection Enable)
    MOV EAX, CR0
    AND EAX, 0x7fffffff ; (书中不用)
    OR  EAX, 0x00000001
    MOV CR0, EAX

    ; 5. ★★★ 真正切换！ ★★★
    ;    一个 JMP 指令清空 CPU 缓存，
    ;    并设置 CS 寄存器为 0x0008 (32位代码段)
    JMP DWORD 8:prot_mode_start ; 8 = 0x0008

fin:
    HLT
    JMP fin

; ---------------------------------
; GDT (全局描述符表)
; ---------------------------------
GDT0:
    ; 空描述符 (必须有)
    RESB 8
    ; 代码段描述符 (4GB)
    DW 0xffff   ; 段界限 (low)
    DW 0x0000   ; 段基址 (low)
    DB 0x00     ; 段基址 (mid)
    DB 0x9a     ; 属性 (P=1, DPL=00, S=1, TYPE=1010 -> 可执行/可读)
    DB 0xcf     ; 属性+界限 (G=1, D=1, L=0, AVL=0, Limit=1111)
    DB 0x00     ; 段基址 (high)
    ; 数据段描述符 (4GB)
    DW 0xffff
    DW 0x0000
    DB 0x00
    DB 0x92     ; 属性 (P=1, DPL=00, S=1, TYPE=0010 -> 可读/可写)
    DB 0xcf
    DB 0x00

GDTR0:
    DW 8*3 - 1  ; GDT 界限 (23)
    DD GDT0     ; GDT 起始地址

; ---------------------------------
; 32 位模式代码 BITS 32
; ---------------------------------
BITS 32
prot_mode_start:
    ; 设置 32 位数据段寄存器
    MOV AX, 0x0010  ; 0x0010 是数据段 (GDT 的第 2 项)
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX
    
    MOV ESP, 0x9fc00 
    
    ; 跳转到 C 语言内核！
    JMP 0xc200

; ---------------------------------
; 填充 & 魔法数字 (确保 512 字节)
; ---------------------------------
BITS 16
    RESB 510 - ($ - $$) 
    DB 0x55, 0xaa

