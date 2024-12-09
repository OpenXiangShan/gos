Gos（generic operating system）为北京开源芯片研究院开发的一套用于riscv cpu/SOC系统测试的软件程序,支持M态、S态、U态、VS态的各种测试.
Gos的核心层提供了很多功能接口，供测试程序调用，以便更快开发出测试用例，诸如：内存管理、任务调度、文件系统、设备驱动、中断控制、Pcie、
定时器、dma、iommu、ipi、虚拟化等(相关介绍详见Documents/)。

Gos使用GPL协议开源，使用时请遵循GPL开源协议。

Ps. 该套软件的某些模块参考了Linux & Xvisor

# 一、结构
gos完全编译会生成4个bin文件：gos.bin、myGuest.bin、myUser.bin、mysbi.bin

其中：

mysbi.bin ：M态程序，系统启动时的入口，完成部分初始化及向低特权级提供sbi接口；

gos.bin ： S（HS）态程序，gos主要运行的地方，并提供了对于大部分异常及中断的处理，其提供了一个	Shell，用于接收客户请求，执行对应的command；

myUser.bin ：U态程序，作为payload被打包到gos.bin中，gos中可以在Shell命令行输入命令“user_run [command] [param1] [param2] ..."来执行它；

myGuest.bin ：VS态程序，作为payload被打包到gos.bin中，gos中可以在Shell命令行中输入命令“vcpu_run [command] [param1] [param2] ...”来执行它；

用户可以在myGuest/myUser中添加VS/U态的command，当使用vcpu_run/user_run执行时，会根据后面的参数中的command执行相应command，并传入param参数。

# 二、编译
gos支持Kbuild构建。

## 使用build.sh编译：
为了简化编译步骤，在build.sh中封装了编译命令（下面的make xxx），使用build.sh编译：

./build.sh         : 使用当前的配置进行全部编译（.config下的内容，如果使用menuconfig修改了配置，直接运行build.sh即可）

./build.sh default : 编译默认配置（为qemu下运行的全量编译，编译后可以直接使用./build.sh run使用qemu运行）

./build.sh fpga-h  : 编译fpga-h配置（为kunminghuv2下可运行的编译配置）

./build.sh vcs-h   : 编译vcs-h配置（为vcs下可运行的编译配置）

./build.sh vcs-aia-minimum : vcs下的最小系统（使用aia）

## 编译命令：
make menuconfig     :  调出menuconfig配置

make xxx_defconfig  :  匹配configs/下的xxx_defconfig，生成对应配置文件

make autoconf       :  根据配置文件内容进行配置

make                :  全部编译（gos+myGuset+myUser），并打包成Image.bin

make myUser_bin     :  编译myUser，生成myUser.bin（位于out/myUser.bin）

make myGuest_bin    :  编译myGuest，生成myGuset.bin（位于out/myGuset.bin）

make xxx.dtb        :  编译设备树，匹配configs/dts下的dtb

make clean          :  全部clean

make myUser-clean   :  clean User

make myGuest-clean  :  clean myGuest

make run            :  使用qemu/nemu运行

make run-debug      :  使用qemu -S -s选项debug

make format         :  格式化

make fpga           :  生成用于fpga烧录的data.txt

## 编译步骤：
1、make clean

2、make xxx.dtb

3、make xxx_defconfig（make menuconfig）

4、make autoconf

5、make

# 三、执行测试命令
gos启动完成并进入Shell后输入ls命令可以看到目前支持的测试命令，在Shell中输入相应的命令执行相应的测试程序（测试程序位于app/command/下）。

如果想要gos启动后自动执行测试命令，在auto_run.bin下配置要执行的命令即可。

