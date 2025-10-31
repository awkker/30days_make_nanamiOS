; op.asm   
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
; 程序核心
; ---------------------------------
entry:
    ; 初始化寄存器 (设置栈)
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX
    ; MOV ES, AX ; 后面会马上重设 ES，这句可以省略





;读磁盘
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
    JNC next ; 没出错的话跳转到fin


    ADD SI,1 ; 往SI加1
    CMP SI,5 ; 比较SI与5
    JAE error ; SI >= 5时，跳转到error
    MOV AH,0x00
    MOV DL,0x00 ; A驱动器
    INT 0x13 ; 重置驱动器
    JMP retry
next:
    MOV AX,ES; 把内存地址后移0x200
    ADD AX,512/16
    MOV ES,AX ; 因为没有ADD ES,0x020指令，所以这里稍微绕个弯
    ADD CL,1; 往CL里加1 
    CMP CL,18; 比较CL与18
    JBE readloop ; 如果CL <= 18 跳转至readloop
    MOV CL,1
    ADD DH,1
    CMP DH,2
    JB readloop; 如果DH < 2，则跳转到readloop(也就是失败了)
    MOV DH,0
    ADD CH,1
    CMP CH,CYLS
    JB readloop; 如果CH < CYLS，则跳转到readloop  
    MOV WORD [0x0ff0], CYLS
    JMP 0xc200
error:
    JMP fin         ; 失败了也先跳转到 fin  

fin:
    HLT
    JMP fin


; ---------------------------------
; 填充 & 魔法数字 (确保 512 字节)
; ---------------------------------
    RESB 510 - ($ - $$) 
    DB 0x55, 0xaa

