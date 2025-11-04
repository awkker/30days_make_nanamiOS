/*void io_hlt(void);
void write_mem8(int addr, int data);
void HariMain(void)
{
    int i; //变量声明：i是一个32位整数
    char *p;
    for (i = 0xa0000; i <= 0xaffff; i++) {
        //write_mem8(i, i & 0x0f);
        p=(char *)i;
        *p=(char)(i&0x0f);
    }
    for (;;) {
        io_hlt();
    }
}*/


/* 启动信息结构体 */
struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    unsigned char *vram;
};

/* GDT/IDT 相关结构体 */
struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

/* 颜色常量定义 */
#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(unsigned char *vram, int xsize, int ysize);
void putfont8(unsigned char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, int x, int y, char c, char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(unsigned char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
int sprintf(char *str, const char *fmt, ...);

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    char s[40], mcursor[256];
    int mx, my;
    
    init_gdtidt();
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
    
    for (;;) {
        io_hlt();
    }
}

void init_palette(void)
{
    static unsigned char table_rgb[16*3]={
        0x00, 0x00, 0x00, /* 0:黑 */
        0xff, 0x00, 0x00, /* 1:亮红 */
        0x00, 0xff, 0x00, /* 2:亮绿 */
        0xff, 0xff, 0x00, /* 3:亮黄 */
        0x00, 0x00, 0xff, /* 4:亮蓝 */
        0xff, 0x00, 0xff, /* 5:亮紫 */
        0x00, 0xff, 0xff, /* 6:浅亮蓝 */
        0xff, 0xff, 0xff, /* 7:白 */
        0xc6, 0xc6, 0xc6, /* 8:亮灰 */
        0x84, 0x00, 0x00, /* 9:暗红 */
        0x00, 0x84, 0x00, /* 10:暗绿 */
        0x84, 0x84, 0x00, /* 11:暗黄 */
        0x00, 0x00, 0x84, /* 12:暗青 */
        0x84, 0x00, 0x84, /* 13:暗紫 */
        0x00, 0x84, 0x84, /* 14:浅暗蓝 */
        0x84, 0x84, 0x84 /* 15:暗灰 */
    };
    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
    int i,eflags;
    eflags=io_load_eflags();
    io_cli();
    io_out8(0x03c8, start);
    for(i=start;i<=end;i++) {
        io_out8(0x03c9, rgb[0]/4);
        io_out8(0x03c9, rgb[1]/4);
        io_out8(0x03c9, rgb[2]/4);
        rgb += 3;  // ★ 移动指针到下一个颜色的 RGB 值
    }
    io_store_eflags(eflags);
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
    }
    return;
}

void init_screen(unsigned char *vram, int xsize, int ysize)
{
    /* 绘制桌面背景 */
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    
    /* 绘制任务栏 */
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 26, xsize - 1, ysize - 1);
    
    /* 绘制左侧按钮（凸起效果） */
    boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize - 24, 59, ysize - 24);
    boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize - 24, 2, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 3, ysize - 4, 59, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 59, ysize - 23, 59, ysize - 5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8(vram, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);
    
    /* 绘制右侧托盘（凹陷效果） */
    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize - 4);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
    
    return;
}

void putfont8(unsigned char *vram, int xsize, int x, int y, char c, char *font)
{
    int i;
    unsigned char *p, d /* data */;
    for (i = 0; i < 16; i++) {
        p = vram + (y + i) * xsize + x;
        d = font[i];
        if ((d & 0x80) != 0) { p[0] = c; }
        if ((d & 0x40) != 0) { p[1] = c; }
        if ((d & 0x20) != 0) { p[2] = c; }
        if ((d & 0x10) != 0) { p[3] = c; }
        if ((d & 0x08) != 0) { p[4] = c; }
        if ((d & 0x04) != 0) { p[5] = c; }
        if ((d & 0x02) != 0) { p[6] = c; }
        if ((d & 0x01) != 0) { p[7] = c; }
    }
    return;
}

void putfonts8_asc(unsigned char *vram, int xsize, int x, int y, char c, char *s)
{
    extern char hankaku[4096];
    for (; *s != 0x00; s++) {
        putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
        x += 8;
    }
    return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
    static char cursor[16][16] = {
        "**..............",
        "*O*.............",
        "*OO*............",
        "*OOO*...........",
        "*OOOO*..........",
        "*OOOOO*.........",
        "*OOOOOO*........",
        "*OOOOOOO*.......",
        "*OOOOOOOO*......",
        "*OOOO*****......",
        "*OO*O*..........",
        "*O*.*O*.........",
        "**...*O*........",
        "*.....*O*.......",
        ".......*O*......",
        ".........*......"
    };
    int x, y;
    
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (cursor[y][x] == '*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            if (cursor[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if (cursor[y][x] == '.') {
                mouse[y * 16 + x] = bc;
            }
        }
    }
    return;
}

void putblock8_8(unsigned char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
    int x, y;
    for (y = 0; y < pysize; y++) {
        for (x = 0; x < pxsize; x++) {
            vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
        }
    }
    return;
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

void init_gdtidt(void)
{
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;
    int i;
    
    /* GDT的初始化 */
    for (i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);
    
    /* IDT的初始化 */
    for (i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);
    
    return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff) {
        ar |= 0x8000; /* G_bit = 1 */
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}