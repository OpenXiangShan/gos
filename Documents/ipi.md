# 一、概述
ipi模块为gos核间通信能力。其向ipi驱动提供了注册ipi设备接口，以及向上层或者其他模块提供了发送ipi接口。

# 二、相关接口
## register_ipi
该接口通常由ipi设备驱动调用，向ipi core注册ipi设备。gos中只会有一个ipi，如果多个设备驱动中都调用该接口注册ipi设备，那么会使用第一个。

## send_ipi
发送ipi，接口参数中需要传入cpu、id和一个priv指针。

其中cpu为要向哪个cpu发送ipi，id为ipi_msg的id，目前只定义了一个id 0，当目标cpu收到发来的ipi时会调用ipi_handler[id]（定义在core/ipi.c中，目前id 0对应的处理为ipi_do_nothing，里面只会打印一条ipi do nothing...）所指向的处理函数进行处理（会向处理函数传入priv作为参数）。

如果需要添加新的ipi处理，参数ipi_do_nothing即可。

## process_ipi
由中断分发模块调用，用户通常不需要关心

# 三、ipi设备
目前支持aia-ipi以及clint-ipi，在menuconfig中的Select ipi type中配置（如果选择了使用plic，那么其中只有clint-ipi一个选项）。

如果需要添加新的ipi设备，驱动中调用register_ipi向ipi core注册接口，可以参照其中aia-ipi或者clint-ipi。

