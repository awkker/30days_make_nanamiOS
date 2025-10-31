; haribote-os (内核)
; TAB=4

;ORG 0xc200  ; 关键！告诉 NASM，这个程序会被加载到内存 0xc200 处

;MOV AL,0x13 ; 0x13 = 320x200x8bit 图形模式
;MOV AH,0x00
;INT 0x10    ; 调用显卡 BIOS
;fin:
;    HLT
;    JMP fin

; haribote-os (内核)
; TAB=4

; ---- “系统信息区”的内存地址“外号” ----
CYLS    EQU 0x0ff0  ; 启动区会把柱面数写在这里
LEDS    EQU 0x0ff1  ; 键盘 LED 状态
VMODE   EQU 0x0ff2  ; 颜色模式 (8位色, 16位色?)
SCRNX   EQU 0x0ff4  ; X分辨率
SCRNY   EQU 0x0ff6  ; Y分辨率
VRAM    EQU 0x0ff8  ; 显存 (Video RAM) 开始地址

; ---- 内核主程序 ----
        ORG 0xc200  ; 我会被加载到 0xc200

        ; 1. 切换到 320x200x8bit 图形模式
        MOV AL,0x13
        MOV AH,0x00
        INT 0x10

        ; 2. 把屏幕信息存入“信息区”
        MOV BYTE [VMODE], 8         ; 存入“8” (8位色)
        MOV WORD [SCRNX], 320       ; 存入“320”
        MOV WORD [SCRNY], 200       ; 存入“200”
        MOV DWORD [VRAM], 0x000a0000 ; 存入“0xa0000” (VRAM地址)

        ; 3. 获取键盘 LED 状态并存入“信息区”
        MOV AH,0x02
        INT 0x16                    ; 调用键盘 BIOS
        MOV [LEDS], AL              ; 把结果(AL)存入 0x0ff1

fin:
        HLT
        JMP fin



