# 一、概述
 User模块提供了执行U态任务的能力，其实现逻辑大致与virt相同，支持创建user并进入U态执行myUser中的程序，其在大循环中等待并执行传入的command。

同virt一样，也支持多user同时执行，每个user作为一个task存在，互不干扰。

user中提供了user_run⼤循环、syscall接⼝、U模式异常处理、U态物理/虚拟内存映射及管理、host 和U态上下⽂切换等功能。  

# 二、user注册与运行
## user_create
创建一个user，并返回一个struct user，该接口在一个cpu上只会创建一个user，重复调用时只会返回该cpu上创建的第一个user

## user_create_force
创建一个user，并返回一个struct user，该接口不管该cpu上存在多少user（不能多于spec规定）都会强制创建一个新的user

## user_mode_run
传入user_create_xxx创建返回的struct user，使其跳转到U态启动MyUser（运行myUser中的代码）。如果传入的user已经被创建，则直接向该user发送command让其执行。

# 三、user页表映射
## user_page_mapping
将一段物理地址映射到user地址空间

# 四、相关命令
gos中支持了几个command用于在Shell中直接创建user并运行、

## user_run
用法：

user_run [command] [param1] [param2] ...

在cpu0上启动一个vcpu，myUser中执行command并传入参数param1、param2 ...

该命令多次执行也只会创建一个user，第二次以后执行只会将command发送到第一次执行时创建的user上去并执行

## user_run_ext
user_run_ext cpu=[cpu_id] [cmd] [param1] [param2]...

该命令会强制在"cpu="指定的cpu上创建一个user（如没有指定则默认在cpu0上），并在其上运行cmd命令，参数为param1、param2 ... 

## user_run_ext_at
用法：

user_run_ext_at cpu=[cpu id] userid=[vmid] [cmd] [params1] [params2] ...

该命令不会创建新的user，只会在"cpu="指定的cpu上运行的"userid="指定的user上执行cmd，参数为param1、param2 ... .如果不存在该user则返回错误

## user_info
显示目前gos中存在的所有user

