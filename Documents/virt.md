# 一、概述
 virt模块提供了虚拟化的能力，包括：vcpu_run主循环、machine的模拟、中断虚拟化的处理、 timer虚拟化的处理、虚拟机异常的处理、虚拟机和host上下文切换处理、设备直通（pass through）等。

当gos创建并运行一个vcpu时，myGuest在的程序会作为guest os在cpu跳转到vs模式时被执行，myGuest在系统初始化完成后会进入到一个大循环中等待gos发来command并执行。 

host可以多次向某个vcpu发送command，当host向vcpu发送command时，vcpu会去处理相应的命令（在myGuest中注册⼀个command）。  

vcpu在gos中是作为⼀个task存在的，当在某个task中第⼀次调用virt模块提供的vcpu_run接口时会根据参数，在某个cpu上运行一个vcpu，并分配给其一个vm id，后面可以多次向该vcpu发送command让其执行。  

# 二、vcpu的注册及运行  
gos支持多vcpu，每个vcpu都作为一个task，在其上执行myGuest，互不干扰。

## vcpu_create
创建一个vcpu，并返回一个struct vcpu，该接口在一个cpu上只会创建一个vcpu，重复调用时只会返回该cpu上创建的第一个vcpu

## vcpu_create_force
创建一个vcpu，并返回一个struct vcpu，该接口不管该cpu上存在多少vcpu（不能多于spec规定）都会强制创建一个新的vcpu

## vcpu_create_ext
创建一个vcpu，同vcpu_create_force一样，会强制创建一个新的vcpu，除此之外，其参数可以传入一个struct device **的指针，可以将多个设备传入该接口，其中会将该设备直通到vcpu中

## vcpu_run
传入vcpu_create_xxx创建返回的struct vcpu，使其跳转到vs模式启动guest os（运行myGuest中的代码）。如果传入的vcpu已经被创建，则直接向该vcpu发送command让其执行。

# 三、发送request
## vcpu_set_request
向vcpu发送request（比如发送hfence.gvma刷新vcpu的gstage tlb）

# 四、中断虚拟化
 目前支持aia的中断直通：virt中对imsic interrupt file进行了模拟，myGuest中可以编写驱动，将 msi中断直通到VS模式，测试命令：vcpu_run imsic_test。  

# 五、timer虚拟化
 目前通过在host中注册timer，hvip注⼊timer中断的⽅式⽀持timer虚拟化，这需要在virt中模拟⼀个 clint，测试命令：vcpu_run timer_test  

# 六、io虚拟化（设备直通）
目前将my_pci_dmaengine设备（qemu上模拟）直通到了一个vcpu，vcpu中的驱动程序可以直接驱动该设备在vcpu的memory间搬运数据，该过程不会有vm exit,测试命令：vcpu_run_ext pt=1234:1 dma_test 4096

# 七、相关命令
gos中支持了几个command用于在Shell中直接创建vcpu并运行：

## vcpu_run
用法：

vcpu_run [command] [param1] [param2] ...

在cpu0上启动一个vcpu，myGuest中执行command并传入参数param1、param2 ...

该命令多次执行也只会创建一个vcpu，第二次以后执行只会将command发送到第一次执行时创建的vcpu上去并执行

## vcpu_run_ext
用法：

vcpu_run_ext cpu=[cpu_id] pt=[device_name] [cmd] [param1] [param2]...

该命令会强制在"cpu="指定的cpu上创建一个vcpu（如没有指定则默认在cpu0上），并在其上运行cmd命令，参数为param1、param2 ... 并将"pt="中配置的设备直通到该vcpu上

## vcpu_run_ext_at
用法：

vcpu_run_ext_at cpu=[cpu id] vmid=[vmid] [cmd] [params1] [params2] ...

该命令不会创建新的vcpu，只会在"cpu="指定的cpu上运行的"vmid="指定的vcpu上执行cmd，参数为param1、param2 ... .如果不存在该vcpu则返回错误

## vcpu_info
显示gos中当前存在的所有vcpu
