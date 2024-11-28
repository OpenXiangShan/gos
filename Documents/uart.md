# 一、概述
uart模块提供了uart初始化，early print初始化接⼝，他们会调⽤到driver层具体的串⼝驱动，向 串⼝驱动提供注册接⼝；并向上层的打印函数提供抽象接口。

# 二、相关接口
## EARLYCON_REGISTER
注册一个earlycon接口，用于早期的串口打印，其中的compat与hw-xxx.c中的compatible匹配时，相应初始化函数被调用

## uart_putc
串口字符输出

## uart_puts
串口字符输出

# 三、串口输入
Menuconfig中有一个Use uart poll选项，勾选时串口输入不依赖中断，不勾选时需要中断正常才可以输入

