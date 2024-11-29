# 一、概述
在gos上添加的test case通常作为一个command存在。用户需要在app/command/下通过command模块提供的接口添加程序。gos启动后，在Shell中输入该command就可以执行到添加的测试代码。

# 二、相关接口
## register_command
注册一个命令，调用时需要传入struct command结构，其中cmd字段为gos Shell看到的命令名称，handler对应输入命令名称后执行到的测试代码

# 三、添加一个command
1、添加command代码，支持.c、.S（汇编）、.a（静态链接库）；

2、如果测试程序在一个单独的文件里，在app/command/Makefile里添加：

如果是.c文件： obj-$(XXX) += xxx.c

如果是.S文件： obj-$(XXX) += xxx.S

3、如果是多个源文件，则在app/command/Makefile中添加：

a. obj-$(XXX) += aaa/  (为app/command/下创建个一个目录名)

b. 在app/command/aaa下创建一个Makefile

c. 将源文件发到app/command/aaa/下

d. 在app/command/aaa/Makefile下添加：

对每一个.c文件： obj-$(XXX) += xxx.c

对每一个.S文件： obj-$(XXX) += xxx.S

对每一个.a文件：  lib-$(XXX) += xxx.a

4、在app/command/Kconfig下添加config XXX

5、在menuconfig下勾选XXX

