# 中断处理程序实现说明

## 概述

本次添加了键盘（IRQ1）和鼠标（IRQ12）的中断处理程序。当用户按下键盘或移动鼠标时，系统会收到中断信号并显示相应的信息。

## 中断处理流程

```
硬件中断 → PIC → CPU → IDT查表 → 汇编包装 → C处理函数
IRQ1/IRQ12  ↓      ↓     ↓          ↓           ↓
           INT   IRETD  0x21/0x2c  保存寄存器  inthandler21/2c
```

## 文件修改

### 1. int.c - 中断处理函数

添加了两个C语言的中断处理函数：

```c
void inthandler21(int *esp)  // 键盘中断 (IRQ1 → INT 0x21)
void inthandler2c(int *esp)  // 鼠标中断 (IRQ12 → INT 0x2c)
```

这些函数接收 `esp` 指针（虽然暂时不用），在屏幕上显示中断信息后进入无限循环。

### 2. naskfunc.nas - 汇编包装函数

添加了汇编包装函数来正确处理中断：

```nasm
asm_inthandler21:
    PUSH ES
    PUSH DS
    PUSHAD           ; 保存所有通用寄存器
    MOV EAX, ESP
    PUSH EAX         ; 传递 ESP 给 C 函数
    MOV AX, SS
    MOV DS, AX       ; 设置数据段
    MOV ES, AX       ; 设置扩展段
    CALL inthandler21
    POP EAX
    POPAD            ; 恢复所有寄存器
    POP DS
    POP ES
    IRETD            ; 中断返回（不能用 RET）
```

### 3. dsctbl.c - 注册中断处理程序

在 IDT 中注册中断处理程序：

```c
set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, 0x008e);
set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, 0x008e);
```

参数说明：
- `idt + 0x21`：中断号 0x21（键盘）
- `(int) asm_inthandler21`：处理程序地址
- `2 * 8`：代码段选择子（GDT 第2项）
- `0x008e`：属性（存在位=1, DPL=0, 类型=中断门）

### 4. bootpack.c - 启用中断

```c
init_gdtidt();
init_pic();
io_sti();  // 开放CPU中断
```

### 5. int.c - 允许键盘和鼠标中断

```c
io_out8(PIC0_IMR, 0xf9); // 11111001 允许IRQ1(键盘)和IRQ2(PIC1)
io_out8(PIC1_IMR, 0xef); // 11101111 允许IRQ12(鼠标)
```

## 为什么不能用 RET？

**关键点**：中断处理必须用 `IRETD` 而不是 `RET`

### RET 指令
```
POP EIP          ; 只弹出返回地址
跳转到 EIP
```

### IRETD 指令（Interrupt Return）
```
POP EIP          ; 弹出返回地址
POP CS           ; 弹出代码段
POP EFLAGS       ; 弹出标志寄存器
跳转到 CS:EIP
```

**原因**：
- CPU 响应中断时，会自动压栈：`EFLAGS → CS → EIP`
- 如果用 `RET` 返回，只弹出 EIP，栈会错位
- 必须用 `IRETD` 来正确恢复这3个值

## 为什么要保存寄存器？

中断可能在任何时候发生，打断正在执行的程序：

```c
// 主程序正在执行
int x = 5;
int y = x + 3;  // ← 中断在这里发生！EAX可能正在使用
int z = y * 2;
```

如果中断处理程序修改了 EAX，主程序恢复后会得到错误的结果！

**解决方案**：
1. **PUSHAD**：保存所有通用寄存器（EAX, EBX, ECX, EDX, ESP, EBP, ESI, EDI）
2. **PUSH DS/ES**：保存段寄存器
3. 执行中断处理
4. **POP ES/DS**：恢复段寄存器
5. **POPAD**：恢复所有通用寄存器

这样主程序完全感觉不到中断的发生！

## ESP 参数的作用

```nasm
MOV EAX, ESP
PUSH EAX
CALL inthandler21
```

这里传递的 ESP 指向中断发生时的栈状态，包含：
- 保存的寄存器值
- 中断发生时的 CS:EIP
- EFLAGS

虽然现在不用，但以后可以用来：
- 调试（查看中断发生时的寄存器）
- 获取错误代码（某些异常会压入错误码）
- 任务切换

## 中断号分配

| IRQ | 设备 | INT号 | 处理函数 |
|-----|------|-------|----------|
| IRQ1 | 键盘 | 0x21 | inthandler21 |
| IRQ12 | 鼠标 | 0x2c | inthandler2c |

## 测试方法

启动系统后：
- **按任意键** → 显示 "INT 21 (IRQ-1) : PS/2 keyboard"
- **移动鼠标** → 显示 "INT 2C (IRQ-12) : PS/2 mouse"

显示后系统进入无限循环（这是预期行为，用于验证中断确实触发了）。

## EXTERN 和 GLOBAL

### EXTERN（从 C 导入）
```nasm
EXTERN inthandler21, inthandler2c
```
告诉汇编器：这两个符号在其他文件（C文件）中定义

### GLOBAL（导出给 C）
```nasm
GLOBAL asm_inthandler21, asm_inthandler2c
```
告诉汇编器：这两个符号要导出，让 C 文件可以使用

## 编译结果

- kernel.bin 大小：**8160 字节**（增加了 352 字节）
- 新增文件：无（修改现有文件）
- 编译成功 ✅
- 运行测试 ✅

## 下一步

目前中断处理程序只是简单显示信息。后续可以：
1. 从键盘端口读取按键数据
2. 从鼠标端口读取位置数据
3. 实现键盘输入队列
4. 实现鼠标光标移动

## 注意事项

1. **中断屏蔽位**：
   - bit0 = 0：允许 IRQ0（定时器）❌ 暂时不用
   - bit1 = 0：允许 IRQ1（键盘）✅
   - bit2 = 0：允许 IRQ2（从 PIC）✅ 必须
   - bit3-7 = 1：禁止其他中断

2. **段选择子**：
   - `2 * 8` = 0x10 = GDT 第2项
   - 这是我们在 dsctbl.c 中设置的代码段

3. **中断门属性 0x008e**：
   - `0x8_` = 存在位（P=1）
   - `0x_0` = DPL=0（特权级0）
   - `0x__e` = 类型=1110（32位中断门）

## 总结

通过本次实现，我们完成了：
✅ PIC 初始化
✅ IDT 注册中断处理程序
✅ 汇编包装函数（保存/恢复寄存器）
✅ C 语言处理函数
✅ 启用中断

现在操作系统可以响应键盘和鼠标中断了！🎉

