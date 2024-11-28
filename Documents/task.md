# 一、概述
task模块提供了创建task的接口，该task被注册后，会在后⾯某时候被调度运⾏；并提供了task调 度以及上下⽂切换逻辑。

task模块会调⽤clock模块的register_timer_event接⼝注册scheduler定时器，以10ms为周期触 发，scheduler的event handler中会按顺序调度已注册的task运⾏。创建的task执行完毕后，会由task模块的task_fn_wrap负责释放掉task占据的内存并销毁掉进程。

task模块还提供了⼀个do_idle函数，在secondary cpus被bringup起来并做好基本的初始化工作后便会进入idle，直到在该cpu上被创建了task。⼀个cpu上有待执行的task时会以默认10ms（可在menuconfig的CONFIG_TASK_SCHEDULER_PERIOD中配置）的粒度挨个执行这些 task，当没有可执行的task时会再次进入idle。

# 二、相关接口
## create_task
创建一个task，在指定的cpu上执行，执行的函数为参数中传入的fn

## create_user_task
创建一个user task，在指定的cpu上执行。主要和user_run配合使用来支持多user

## schedule
使当前task立刻让出cpu，并重新调度

## Sleep
使当前task立刻让出cpu，并把task状态配置为SLEEP，此时该task不会参与调度

## sleep_to_timeout
使当前task立刻让出cpu，并把task状态配置为SLEEP，设置的timeout时间到时后task会重新参数调度

## set_task_status
配置task状态，当task为SLEEP状态时不会参与调度，当为READY时正常调度

## walk_task_per_cpu
遍历并打印某个cpu上的所有task

## walk_task_all_cpu
遍历并打印所有cpu上的所有task

# 三、将一个命令放到一个单独的task中执行
Gos中提供了一个load命令，其用法是：

load cpu=[cpuid] [comand] [params1] [params2] ...

在gos的Shell中使用load执行一个命令时，会把该命令放到一个新的task中去执行，而不是在Shell task中。

