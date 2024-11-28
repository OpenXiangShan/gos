# 一、概述
device_driver模块向gos提供了设备注册、驱动注册以及设备驱动相匹配并执行设备驱动程序的能力。

# 二、设备注册
gos中的所有设备均在bsp/hw-xxx.c中注册，具体可以参考bsp/hw-qemu.c。

# 三、驱动注册
使用DRIVER_REGISTER接口向gos注册一个驱动程序。其定义为：

DRIVER_REGISTER(name, init_fn, compat)

在gos启动过程中，当其中的compat字段与hw-xxx.c中的compatible字段匹配时，init_fn会被调用。

