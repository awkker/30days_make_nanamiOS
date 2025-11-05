/* bootpack.c - 主函数 */

#include "bootpack.h"

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    char s[40], mcursor[256];
    int mx, my;
    
    init_gdtidt();
    init_pic();
    io_sti(); /* IDT/PIC的初始化已经完成，开放CPU的中断 */
    
    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    mx = (binfo->scrnx - 16) / 2;  /* 计算画面中心坐标 */
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    
    putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "ABC 123");
    putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Nanami OS.");
    putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Nanami OS.");
    
    sprintf(s, "scrnx = %d", binfo->scrnx);
    putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);
    
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 80, COL8_FFFFFF, "Initializing keyboard...");
    init_keyboard();
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 96, COL8_FFFFFF, "Keyboard OK");
    
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 112, COL8_FFFFFF, "Enabling mouse...");
    enable_mouse();
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 128, COL8_FFFFFF, "Mouse OK");
    
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 144, COL8_FFFFFF, "Initialization complete!");
    
    for (;;) {
        io_hlt();
    }
}

/* 简化版sprintf实现 - 支持%d, %x, %X格式 */
int sprintf(char *str, const char *fmt, ...)
{
    int *args = (int *)(&fmt + 1);  /* 可变参数指针 */
    int arg_index = 0;
    char *s = str;
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                /* 十进制整数 */
                int num = args[arg_index++];
                char tmp[12];
                int i = 0, j;
                
                if (num < 0) {
                    *s++ = '-';
                    num = -num;
                }
                if (num == 0) {
                    *s++ = '0';
                } else {
                    while (num > 0) {
                        tmp[i++] = '0' + (num % 10);
                        num /= 10;
                    }
                    for (j = i - 1; j >= 0; j--) {
                        *s++ = tmp[j];
                    }
                }
            } else if (*fmt == 'x' || *fmt == 'X') {
                /* 十六进制整数 */
                unsigned int num = args[arg_index++];
                char tmp[12];
                int i = 0, j;
                char base = (*fmt == 'x') ? 'a' : 'A';
                
                if (num == 0) {
                    *s++ = '0';
                } else {
                    while (num > 0) {
                        int digit = num % 16;
                        if (digit < 10) {
                            tmp[i++] = '0' + digit;
                        } else {
                            tmp[i++] = base + (digit - 10);
                        }
                        num /= 16;
                    }
                    for (j = i - 1; j >= 0; j--) {
                        *s++ = tmp[j];
                    }
                }
            } else if (*fmt == 's') {
                /* 字符串 */
                char *str_arg = *((char **)&args[arg_index++]);
                while (*str_arg) {
                    *s++ = *str_arg++;
                }
            } else if (*fmt == 'c') {
                /* 字符 */
                *s++ = (char)args[arg_index++];
            } else {
                *s++ = *fmt;
            }
            fmt++;
        } else {
            *s++ = *fmt++;
        }
    }
    *s = '\0';
    return s - str;
}
