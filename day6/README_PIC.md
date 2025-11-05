# PIC（可编程中断控制器）说明

## 什么是 PIC？

PIC（Programmable Interrupt Controller，可编程中断控制器）是用于管理中断的芯片。电脑中有主 PIC（PIC0）和从 PIC（PIC1），共可管理 16 路 IRQ（中断请求）信号。

## 硬件结构

- **主 PIC（PIC0）**：管理 IRQ0-7
- **从 PIC（PIC1）**：管理 IRQ8-15，通过 IRQ2 连接到主 PIC

```
IRQ0-7  → PIC0 (主) → CPU
IRQ8-15 → PIC1 (从) ↗
```

## PIC 端口号

### 主 PIC（PIC0）
- `0x0020`：ICW1, OCW2
- `0x0021`：IMR, ICW2, ICW3, ICW4

### 从 PIC（PIC1）
- `0x00a0`：ICW1, OCW2
- `0x00a1`：IMR, ICW2, ICW3, ICW4

## 寄存器说明

### IMR（Interrupt Mask Register - 中断屏蔽寄存器）
8 位寄存器，每位对应一路 IRQ 信号：
- **1**：屏蔽该路中断（忽略）
- **0**：允许该路中断

### ICW（Initial Control Word - 初始化控制字）
有 ICW1-ICW4 共 4 个字节：

- **ICW1**：边沿触发模式设定
  - 固定值：`0x11`
  
- **ICW2**：中断号映射（最重要！）
  - PIC0：`0x20`（IRQ0-7 → INT 0x20-0x27）
  - PIC1：`0x28`（IRQ8-15 → INT 0x28-0x2f）
  
- **ICW3**：主从连接设定
  - PIC0：`1 << 2 = 0x04`（第 2 号 IRQ 连接从 PIC）
  - PIC1：`2`（连接到主 PIC 的 IRQ2）
  
- **ICW4**：无缓冲区模式
  - 固定值：`0x01`

## 初始化流程

```c
void init_pic(void)
{
    // 1. 禁止所有中断
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);

    // 2. 初始化主 PIC
    io_out8(PIC0_ICW1, 0x11);    // 边沿触发模式
    io_out8(PIC0_ICW2, 0x20);    // IRQ0-7 → INT20-27
    io_out8(PIC0_ICW3, 1 << 2);  // IRQ2 连接从 PIC
    io_out8(PIC0_ICW4, 0x01);    // 无缓冲区模式

    // 3. 初始化从 PIC
    io_out8(PIC1_ICW1, 0x11);    // 边沿触发模式
    io_out8(PIC1_ICW2, 0x28);    // IRQ8-15 → INT28-2f
    io_out8(PIC1_ICW3, 2);       // 连接到主 PIC 的 IRQ2
    io_out8(PIC1_ICW4, 0x01);    // 无缓冲区模式

    // 4. 设定中断屏蔽
    io_out8(PIC0_IMR, 0xfb);     // 11111011 除 IRQ2 外全部禁止
    io_out8(PIC1_IMR, 0xff);     // 11111111 全部禁止
}
```

## 中断工作原理

1. **中断发生**：外部设备发送 IRQ 信号
2. **PIC 判断**：检查该路是否被屏蔽
3. **通知 CPU**：通过唯一的中断信号线通知 CPU
4. **发送中断号**：CPU 询问时，PIC 发送 `0xcd 0x??` 两个字节
5. **执行中断**：CPU 将其当作 `INT ??` 指令执行

### 巧妙的设计

PIC 发送的 `0xcd` 恰好是 `INT` 指令的机器码，CPU 会误以为是从内存读取的程序，从而执行指定的中断处理程序！

## 中断号分配

| IRQ | 设备 | INT号 |
|-----|------|-------|
| IRQ0 | 定时器 | 0x20 |
| IRQ1 | 键盘 | 0x21 |
| IRQ2 | 从PIC | 0x22 |
| IRQ3 | COM2 | 0x23 |
| IRQ4 | COM1 | 0x24 |
| IRQ5 | LPT2 | 0x25 |
| IRQ6 | 软盘 | 0x26 |
| IRQ7 | LPT1 | 0x27 |
| IRQ8 | RTC | 0x28 |
| IRQ9 | 重定向 | 0x29 |
| IRQ10 | 保留 | 0x2a |
| IRQ11 | 保留 | 0x2b |
| IRQ12 | 鼠标 | 0x2c |
| IRQ13 | FPU | 0x2d |
| IRQ14 | 硬盘 | 0x2e |
| IRQ15 | 保留 | 0x2f |

## 文件结构

```
int.c          - PIC 初始化实现
bootpack.h     - PIC 端口号定义和函数声明
bootpack.c     - 调用 init_pic()
```

## 注意事项

1. **初始化顺序**：必须先写 ICW1，然后按顺序写 ICW2、ICW3、ICW4
2. **端口号相同**：虽然端口号相同，但 PIC 根据写入顺序区分不同的寄存器
3. **屏蔽中断**：在修改中断设定时必须先屏蔽中断，防止混乱
4. **IRQ2 特殊**：IRQ2 用于连接从 PIC，必须保持开启（bit2=0）

## 为什么要初始化 PIC？

1. **避免冲突**：BIOS 的中断号设定可能与我们的操作系统冲突
2. **统一管理**：将所有硬件中断集中到 INT 0x20-0x2f
3. **可控性**：可以选择性地开启/关闭某些中断
4. **为后续开发铺路**：为处理键盘、鼠标、定时器等中断做准备

