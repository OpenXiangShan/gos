# 一、概述
timer模块向用户提供了设置定时器的功能。

# 二、相关接口
## init_timer
系统初始化时被调用。传入device_init_entry，遍历其中的所有设备，与 __timer_init_table段中的timer驱动匹配，匹配成功调⽤timer驱动初始化函数  

## set_timer
在当前cpu上设置一个timer，到时时调用传入的handler

## set_timer_restart
在当前cpu上设置一个timer，该timer在到时后会自动重置

## set_timer_cpu
在某个cpu上设置一个timer

## set_timer_restart_cpu
在某个cpu上设置一个timer，该timer在到时后会自动重置

## del_timer
删除当前cpu上的某个timer

## del_timer_cpu
删除某个cpu上的某个timer

## set_timer_freeze
传入0时冻结某个timer，到时时也不会调用handler，传入1解冻

