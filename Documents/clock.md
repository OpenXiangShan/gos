# 一、概述
Clock模块向Gos提供了两种能力：clock_source和clock_event。

其中，clock_source向gos提供时钟源，clock_event向gos提供定时事件。clock event提供的是一个高精度的tickless的timer，如果clock模块中没有任何clock event事件时，timer中断不会触发。

其中，clock_event是一个percpu的结构（具体请移步percpu.md），每一个cpu上都有自己的一个clock_event结构，来管理自己cpu上的clock event事件。

# 二、分层结构
## core（core/clock.c）：
作为中间层向其他模块以及app comand程序提供了获取系统时间、注册与处理timer事件的各个接口，同时向timer驱动提供了clock_source设备与clock_event设备的注册接口。

## driver（driver/timer/）：
core作为抽象层，其操作最终都会调用到具体driver中去，目前只提供riscv-clint一个timer驱动（driver/timer/clint.c），其中调用clock core里面的注册接口向clock模块注册了clock source与clock event。

如果需要添加新的timer驱动，在driver/timer在参照clint.c添加即可。

# 三、core接口
## register_clock_source：
注册clock source设备，通常是timer驱动中调用。

## register_clock_event：
注册clock event设备，通常是timer驱动中调用。

## register_timer_event：
注册一个timer事件，通常是其他模块（timer模块）调用，根据传入的到时时间设定timer中断。

## unregister_timer_event：
timer事件反注册接口。

## get_clocksource_counter：
通过clock source设备获取系统时间，单位为ms。

## get_clocksource_counter_us：
通过clock source设备获取系统时间，单位为ms。

## get_system_tick：
通过clock source设备获取cpu cycles。

## clock_set_next_event：
设置本cpu上，clock event的下一次到时时间。

## do_clock_event_handler：
遍历本cpu上clock event的每一个timer事件，如果到时则调用相应的处理handler。



