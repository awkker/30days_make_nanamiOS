# Day 5 - 接收启动信息（harib02a）

## 📋 本节目标

在 Day 4 中，我们的程序把屏幕参数（如 0xa0000、320、200）直接硬编码在 C 程序中。这样做的问题是：
- **不灵活**：如果显示模式改变，程序就无法正常运行
- **不规范**：这些信息应该从启动阶段传递过来

本节的目标是：**让 C 程序从内存中读取启动信息**，实现汇编和 C 代码之间的数据传递。

---

## 🔄 数据传递流程

```
[启动阶段 - op.asm]
      ↓
   保存信息到固定内存地址
   - 0x0ff0: 磁盘柱面数
   - 0x0ff4: 屏幕宽度
   - 0x0ff6: 屏幕高度  
   - 0x0ff8: 显存地址
      ↓
[C程序 - bootpack.c]
      ↓
   从固定内存地址读取信息
      ↓
   使用这些信息绘制界面
```

---

## 💾 内存地址约定

我们在汇编程序和 C 程序之间约定了一些固定的内存地址来传递信息：

| 地址 | 大小 | 内容 | 说明 |
|------|------|------|------|
| 0x0ff0 | 2 字节 (WORD) | 磁盘柱面数 | 启动时读取的柱面数 |
| 0x0ff4 | 2 字节 (short) | 屏幕宽度 | 320 |
| 0x0ff6 | 2 字节 (short) | 屏幕高度 | 200 |
| 0x0ff8 | 4 字节 (int) | 显存地址 | 0x000a0000 |

**为什么选择这些地址？**
- 0x0ff0-0x0fff 这段内存在启动阶段是空闲的
- 足够低，在实模式和保护模式下都能访问
- 与书中保持一致，方便交流

---

## 🔧 代码实现

### 1. op.asm - 保存启动信息

```asm
; 1. 写入启动信息到内存（供C程序使用）
MOV WORD [0x0ff0], CYLS          ; 柱面数
MOV WORD [0x0ff4], 320           ; 屏幕宽度
MOV WORD [0x0ff6], 200           ; 屏幕高度
MOV DWORD [0x0ff8], 0x000a0000   ; 显存地址
```

**关键点：**
- `WORD` = 2 字节（16 位）
- `DWORD` = 4 字节（32 位）
- 方括号 `[]` 表示写入内存地址

### 2. bootpack.c - 读取启动信息

```c
void HariMain(void)
{
    unsigned char *vram;
    int xsize, ysize;
    short *binfo_scrnx, *binfo_scrny;  // short* 读取2字节
    int *binfo_vram;                    // int* 读取4字节
    
    init_palette();
    
    /* 从内存中读取启动信息 */
    binfo_scrnx = (short *) 0x0ff4;   // 指向宽度
    binfo_scrny = (short *) 0x0ff6;   // 指向高度
    binfo_vram = (int *) 0x0ff8;      // 指向显存地址
    
    xsize = *binfo_scrnx;              // 解引用，读取实际值
    ysize = *binfo_scrny;
    vram = (unsigned char *) *binfo_vram;
    
    init_screen(vram, xsize, ysize);
    
    for (;;) {
        io_hlt();
    }
}
```

---

## 🧠 核心概念：指针的两步操作

### 理解这段代码的关键

```c
short *binfo_scrnx;              // 1. 定义一个 short 指针
binfo_scrnx = (short *) 0x0ff4;  // 2. 让指针指向地址 0x0ff4
xsize = *binfo_scrnx;            // 3. 解引用，读取该地址的值
```

**为什么不直接写 `xsize = *(short *)0x0ff4;` ？**

可以！但分开写更清晰：
```c
// 方式1：一步到位（简洁但不够清晰）
xsize = *(short *)0x0ff4;

// 方式2：两步走（代码更可读）
short *binfo_scrnx = (short *) 0x0ff4;
xsize = *binfo_scrnx;
```

在学习阶段，方式2更有助于理解指针的本质。

---

## 📊 指针大小与数据类型

| C 类型 | 汇编类型 | 大小 | 用途 |
|--------|----------|------|------|
| `short *` | WORD | 2 字节 | 屏幕宽度、高度 |
| `int *` | DWORD | 4 字节 | 显存地址（32位地址） |
| `char *` | BYTE | 1 字节 | 像素数据 |

**关键原则：** C 程序中的指针类型必须与汇编中保存的数据大小一致！

---

## 🎨 代码重构：init_screen() 函数

为了提高代码可读性，我们把绘制桌面的代码独立成了一个函数：

### 重构前（Day 4）

```c
void HariMain(void)
{
    // ... 初始化 ...
    
    // 一大堆绘制代码
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
    // ... 更多绘制 ...
    
    for (;;) {
        io_hlt();
    }
}
```

### 重构后（Day 5）

```c
void HariMain(void)
{
    // ... 读取启动信息 ...
    
    init_screen(vram, xsize, ysize);  // 一行搞定！
    
    for (;;) {
        io_hlt();
    }
}

void init_screen(unsigned char *vram, int xsize, int ysize)
{
    /* 绘制桌面背景 */
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    
    /* 绘制任务栏 */
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
    // ... 更多绘制 ...
    
    return;
}
```

**重构的好处：**
1. **主函数更清晰**：一眼就能看出程序流程
2. **功能模块化**：绘制屏幕的代码独立管理
3. **便于维护**：要修改界面只需改 `init_screen()`
4. **可重用**：以后可以在其他地方调用这个函数

---

## 🔍 深入理解：为什么要这样传递数据？

### 问题：为什么不直接在 C 程序中定义？

```c
// 为什么不这样？
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define VRAM_ADDR 0xa0000
```

### 答案：因为这些值在启动阶段确定！

1. **显示模式**：由 BIOS 中断 (INT 0x10) 设置
2. **显存地址**：不同显示模式可能不同
3. **磁盘信息**：运行时读取的，编译时不知道

**核心思想：** 汇编负责硬件初始化，C 程序负责应用逻辑，两者通过约定的内存地址传递信息。

---

## 📝 变量命名规范

```c
short *binfo_scrnx;  // bootinfo_screen_x
short *binfo_scrny;  // bootinfo_screen_y
int *binfo_vram;     // bootinfo_video_ram
```

- `binfo` = **boot info**（启动信息）
- `scrn` = **screen**（屏幕）
- `x` / `y` = 宽度 / 高度
- `vram` = **Video RAM**（显存）

好的命名让代码自解释！

---

## 🚀 本节成果

运行程序后，你会发现：
- ✅ 界面显示正常（和 Day 4 一样）
- ✅ 但代码更规范、更灵活了
- ✅ 为将来支持不同显示模式打下了基础

**表面上没变化，但内部架构更合理了！** 这就是重构的意义。

---

## 🤔 思考题

1. **为什么 `vram` 使用 `int *` 而不是 `char *` 来读取地址？**
   <details>
   <summary>点击查看答案</summary>
   
   因为显存地址是一个 32 位的整数值（如 0x000a0000），需要用 4 字节的 `int` 来存储。读取后再转换为 `char *` 用于操作像素。
   
   ```c
   int *binfo_vram = (int *) 0x0ff8;        // 读取4字节的地址值
   vram = (unsigned char *) *binfo_vram;    // 转换为像素指针
   ```
   </details>

2. **如果我想在 0x0ffc 保存鼠标信息，汇编和 C 代码应该怎么写？**
   <details>
   <summary>点击查看答案</summary>
   
   汇编（假设鼠标位置 x=100, y=50）：
   ```asm
   MOV WORD [0x0ffc], 100  ; 鼠标 x 坐标
   MOV WORD [0x0ffe], 50   ; 鼠标 y 坐标
   ```
   
   C 代码：
   ```c
   short *binfo_mouse_x = (short *) 0x0ffc;
   short *binfo_mouse_y = (short *) 0x0ffe;
   int mouse_x = *binfo_mouse_x;
   int mouse_y = *binfo_mouse_y;
   ```
   </details>

3. **为什么不用结构体来组织这些启动信息？**
   <details>
   <summary>点击查看答案</summary>
   
   好问题！确实可以用结构体，而且在后续章节中就会这样做：
   
   ```c
   struct BOOTINFO {
       char cyls;
       char leds;
       char vmode;
       char reserve;
       short scrnx, scrny;
       unsigned char *vram;
   };
   ```
   
   但现在我们一步步来，先理解指针和内存布局的基本概念。
   </details>

---

## 📚 关键知识点总结

1. **汇编与C的数据传递**：通过约定的内存地址
2. **指针的本质**：内存地址 + 数据类型
3. **代码重构**：将功能独立成函数
4. **硬编码的危害**：降低灵活性和可维护性
5. **数据类型匹配**：C 的类型大小必须与汇编一致

---

## 🎯 下一步

现在我们已经：
- ✅ 能够绘制彩色界面
- ✅ 实现了汇编和C的数据传递
- ✅ 代码结构更加清晰

下一步可能会做：
- 📝 显示字符和字符串
- 🖱️ 绘制鼠标光标
- 🪟 实现简单的窗口系统

继续加油！💪

---

## 💡 编程思想

> "好的代码不仅要能运行，还要易于理解和维护。"

本节的重构虽然没有增加新功能，但让代码：
- 更**清晰**（主函数一目了然）
- 更**灵活**（支持不同显示模式）
- 更**规范**（遵循数据传递约定）

**这就是软件工程的价值！** 🌟

