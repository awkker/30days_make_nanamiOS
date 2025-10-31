/* bootpack.ld 链接器脚本 */
OUTPUT_FORMAT("binary") /* 输出为纯二进制 .bin 文件 */
ENTRY(_HariMain)        /* 指定 C 语言的 _HariMain 为程序入口点 */
SECTIONS
{
    . = 0xc200;         /* 关键！告诉链接器，把所有东西都放在 0xc200 地址开始 */
    .text : {
        *(.text)        /* 把所有 .text (代码) 段放在这里 */
    }
    .data : {
        *(.data)        /* 把所有 .data (数据) 段放在这里 */
    }
    .bss : {
        *(.bss)         /* 把所有 .bss (未初始化数据) 段放在这里 */
    }
}
