; ipl.asm (Day 3 启动扇区专用)
; TAB=4

BITS 16
ORG  0x7c00

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
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX
    MOV ES, AX

    MOV SI, msg
putloop:
    MOV AL, [SI]
    ADD SI, 1
    CMP AL, 0
    JE  fin
    MOV AH, 0x0e
    MOV BX, 15
    INT 0x10
    JMP putloop
fin:
    HLT
    JMP fin

msg:
    DB 0x0a, 0x0a
    DB "hello, world"
    DB 0x0a
    DB 0

; ---------------------------------
; 填充 & 魔法数字
; ---------------------------------
    ; 自动计算填充，确保 55 AA 在第 511, 512 字节
    RESB 510 - ($ - $$) 

    DB 0x55, 0xaa
; ---------------------------------
; 启动扇区 (512 字节) 结束
; (后面所有的 FAT 表和 resb 都删掉了)
; ---------------------------------