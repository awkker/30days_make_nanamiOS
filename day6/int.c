/* int.c - 中断相关 */

#include "bootpack.h"

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

void wait_KBC_sendready(void)
{
    /* 等待键盘控制器准备好发送 */
    int timeout = 100000;
    for (;;) {
        if (--timeout == 0) {
            break;  /* 超时退出 */
        }
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(void)
{
    /* 初始化键盘控制器 */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

void enable_mouse(void)
{
    /* 激活鼠标 */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, 0xd4);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, 0xf4);
    return;
    /* 鼠标会发送 ACK (0xFA)，这会触发中断，由中断处理程序读取 */
}

void init_pic(void)
/* PIC的初始化 */
{
    io_out8(PIC0_IMR, 0xff); /* 禁止所有中断 */
    io_out8(PIC1_IMR, 0xff); /* 禁止所有中断 */

    io_out8(PIC0_ICW1, 0x11); /* 边沿触发模式（edge trigger mode） */
    io_out8(PIC0_ICW2, 0x20); /* IRQ0-7由INT20-27接收 */
    io_out8(PIC0_ICW3, 1 << 2); /* PIC1由IRQ2连接 */
    io_out8(PIC0_ICW4, 0x01); /* 无缓冲区模式 */

    io_out8(PIC1_ICW1, 0x11); /* 边沿触发模式（edge trigger mode） */
    io_out8(PIC1_ICW2, 0x28); /* IRQ8-15由INT28-2f接收 */
    io_out8(PIC1_ICW3, 2); /* PIC1由IRQ2连接 */
    io_out8(PIC1_ICW4, 0x01); /* 无缓冲区模式 */

    io_out8(PIC0_IMR, 0xf9); /* 11111001 允许PIC1和键盘(11111001) */
    io_out8(PIC1_IMR, 0xef); /* 11101111 允许鼠标(11101111) */

    return;
}

void inthandler21(int *esp)
/* 来自PS/2键盘的中断 */
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");
    io_in8(PORT_KEYDAT); /* 读取键盘数据，清除中断 */
    io_out8(PIC0_OCW2, 0x61); /* 通知PIC IRQ-01已经处理完成 */
    return;
}

void inthandler2c(int *esp)
/* 来自PS/2鼠标的中断 */
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
    io_in8(PORT_KEYDAT); /* 读取鼠标数据，清除中断 */
    io_out8(PIC1_OCW2, 0x64); /* 通知PIC1 IRQ-12已经处理完成 */
    io_out8(PIC0_OCW2, 0x62); /* 通知PIC0 IRQ-02已经处理完成 */
    return;
}

