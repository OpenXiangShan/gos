/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

# 一、结构：
## 1、结构框图：
gos完全编译会生成4个bin文件：gos.bin、myGuest.bin、myUser.bin、mysbi.bin：
mysbi.bin —— M态程序，系统启动时的入口，完成部分初始化及向低特权级提供sbi接口；
gos.bin —— S（HS）态程序，gos主要运行的地方，并提供了对于大部分异常及中断的处理，其提供了一个Shell，用于接收客户请求，执行对应的command；
myUser.bin —— U态程序，作为payload被打包到gos.bin中，gos中可以在Shell命令行中输入命令“user_run [command] [param1] [param2] ..."来执行它；
myGuest.bin —— VS态程序，作为payload被打包到gos.bin中，gos中可以在Shell命令行中输入命令“vcpu_run [command] [param1] [param2] ...”来执行它；
用户可以在myGuest/myUser中添加VS/U态的command，当使用vcpu_run/user_run执行时，会根据后面的参数中的command执行相应command，并传入param参数。
## 2、文件一览：
```
    18 ./myUser/include/container_of.h
    12 ./myUser/include/string.h
    41 ./myUser/include/type.h
    10 ./myUser/include/malloc.h
    99 ./myUser/include/list.h
     6 ./myUser/include/print.h
    33 ./myUser/include/spinlocks.h
    50 ./myUser/include/command.h
    61 ./myUser/include/barrier.h
    24 ./myUser/command/ls.c
    28 ./myUser/command/hello.c
   247 ./myUser/command/sct_test.c
    66 ./myUser/command/pte_u_test.c
    11 ./myUser/entry/entry.S
    15 ./myUser/entry/main.c
    16 ./myUser/core/syscall.S
   168 ./myUser/core/user_print.c
   112 ./myUser/core/command.c
    36 ./myUser/lib/spinlocks.c
   126 ./myUser/lib/string.c
    21 ./myUser/lib/malloc.c
    92 ./fdt/fdt_wip.c
   852 ./fdt/fdt_ro.c
  2154 ./fdt/libfdt.h
   870 ./fdt/fdt_overlay.c
   102 ./fdt/fdt_addresses.c
   385 ./fdt/fdt_sw.c
   340 ./fdt/fdt.c
   194 ./fdt/libfdt_internal.h
    66 ./fdt/fdt.h
   498 ./fdt/fdt_rw.c
    61 ./fdt/fdt_strerror.c
    38 ./fdt/fdt_empty_tree.c
    96 ./fdt/libfdt_env.h
    95 ./drivers/dmac/dmac_dw_axi.h
   221 ./drivers/dmac/dmac_dw_axi.c
    32 ./drivers/irqchip/plic/plic.h
   168 ./drivers/irqchip/plic/plic.c
    93 ./drivers/irqchip/aia/aplic/aplic_msi.c
    98 ./drivers/irqchip/aia/aplic/aplic.h
     8 ./drivers/irqchip/aia/aplic/aplic_msi.h
   176 ./drivers/irqchip/aia/aplic/aplic.c
    51 ./drivers/irqchip/aia/imsic/imsic.h
    35 ./drivers/irqchip/aia/imsic/imsic_reg.c
    77 ./drivers/irqchip/aia/imsic/imsic_reg.h
   324 ./drivers/irqchip/aia/imsic/imsic.c
   114 ./drivers/imsic-test/imsic_test.c
   158 ./drivers/uart/ns16550a.c
    28 ./drivers/uart/qemu-8250.h
    45 ./drivers/uart/ns16550a.h
    13 ./drivers/uart/uartlite.h
   152 ./drivers/uart/qemu-8250.c
    89 ./drivers/uart/uartlite.c
   159 ./drivers/timer/timer.c
    75 ./drivers/iommu/queue.c
   470 ./drivers/iommu/iommu.c
    69 ./drivers/iommu/iommu_test.c
    10 ./drivers/iommu/queue.h
    17 ./drivers/iommu/io_pgtable.h
   222 ./drivers/iommu/iommu.h
   165 ./drivers/iommu/io_pgtable.c
     9 ./virt/memory_test_emulator.h
   148 ./virt/cpu_tlb.S
     8 ./virt/vcpu_sbi.h
   265 ./virt/machine.c
    91 ./virt/vcpu_aia.c
   117 ./virt/uart_emulator.c
    10 ./virt/vcpu_timer.h
    30 ./virt/memory_test_emulator.c
    14 ./virt/gstage_pgtable.c
    54 ./virt/clint_emulator.c
   168 ./virt/vcpu_switch.S
   146 ./virt/machine.h
    35 ./virt/vcpu_sbi.c
     9 ./virt/memory_emulator.h
    69 ./virt/imsic_emulator.c
    54 ./virt/vcpu_timer.c
     8 ./virt/clint_emulator.h
    57 ./virt/memory_emulator.c
   116 ./virt/vcpu_insn.h
   124 ./virt/vcpu_insn.c
   381 ./virt/virt.c
    72 ./virt/virt_vm_exit.c
     9 ./virt/uart_emulator.h
     9 ./virt/vcpu_aia.h
    11 ./virt/imsic_emulator.h
     6 ./virt/virt_vm_exit.h
    18 ./include/container_of.h
    44 ./include/generated/autoconf.h
    50 ./include/mm.h
   280 ./include/asm/csr.h
   113 ./include/asm/pgtable.h
    26 ./include/asm/mmio.h
    22 ./include/asm/asm-irq.h
    45 ./include/asm/type.h
    50 ./include/asm/trap.h
   193 ./include/asm/asm-offsets.h
    41 ./include/asm/sbi_asm_offsets.h
    45 ./include/asm/ptregs.h
    61 ./include/asm/barrier.h
    70 ./include/asm/sbi.h
    96 ./include/user.h
    20 ./include/string.h
    20 ./include/trap.h
    99 ./include/list.h
    13 ./include/vmap.h
     7 ./include/print.h
    29 ./include/uaccess.h
    33 ./include/spinlocks.h
   148 ./include/virt.h
    73 ./include/cpu_tlb.h
    32 ./include/uapi/swab.h
    13 ./include/uapi/syscall.h
     8 ./include/uapi/align.h
    14 ./include/tlbflush.h
    82 ./myGuest/drivers/clint.c
    27 ./myGuest/drivers/qemu-8250.h
    33 ./myGuest/drivers/page_tlb_test.c
    31 ./myGuest/drivers/qemu-8250.c
   408 ./myGuest/drivers/imsic.c
     6 ./myGuest/drivers/page_tlb_test.h
    24 ./myGuest/include/mm.h
   235 ./myGuest/include/asm/csr.h
   112 ./myGuest/include/asm/pgtable.h
    26 ./myGuest/include/asm/mmio.h
    22 ./myGuest/include/asm/asm-irq.h
    10 ./myGuest/include/asm/type.h
   192 ./myGuest/include/asm/asm-offsets.h
    44 ./myGuest/include/asm/ptregs.h
    61 ./myGuest/include/asm/barrier.h
    68 ./myGuest/include/asm/sbi.h
    12 ./myGuest/include/string.h
    21 ./myGuest/include/irq.h
    14 ./myGuest/include/timer.h
     9 ./myGuest/include/trap.h
    14 ./myGuest/include/uart.h
    12 ./myGuest/include/vmap.h
     8 ./myGuest/include/print.h
    33 ./myGuest/include/spinlocks.h
    35 ./myGuest/include/device.h
    50 ./myGuest/include/command.h
    47 ./myGuest/command/imsic_test.c
    29 ./myGuest/command/memory_test.c
    24 ./myGuest/command/ls.c
    28 ./myGuest/command/hello.c
    39 ./myGuest/command/vs_timer_test.c
   296 ./myGuest/mm/pgtable.c
   150 ./myGuest/mm/mm.c
   145 ./myGuest/mm/vmap.c
   125 ./myGuest/entry/entry.S
    36 ./myGuest/entry/main.c
    38 ./myGuest/entry/trap.c
    45 ./myGuest/core/uart.c
    35 ./myGuest/core/device.c
    57 ./myGuest/core/irq.c
   112 ./myGuest/core/command.c
    59 ./myGuest/core/timer.c
    36 ./myGuest/lib/spinlocks.c
   126 ./myGuest/lib/string.c
   134 ./myGuest/lib/print.c
    51 ./app/command/imsic_test.c
    26 ./app/command/devices.c
    49 ./app/command/plic_set_affinity.c
    43 ./app/command/task_info.c
   406 ./app/command/csr_ctl.c
    26 ./app/command/ls.c
    59 ./app/command/spinlock_test.c
    61 ./app/command/mem_read.c
    26 ./app/command/hello.c
   103 ./app/command/devmm.c
    72 ./app/command/user_run.c
    34 ./app/command/history.c
   292 ./app/command/page_tlb_test.c
    79 ./app/command/dma_test_ext.c
    63 ./app/command/dma_test.c
    74 ./app/command/vcpu_run.c
     6 ./app/shell.h
   293 ./app/command.c
   147 ./app/shell.c
    64 ./app/command.h
   367 ./mm/pgtable.c
    40 ./mm/flush_tlb.c
   228 ./mm/mm.c
   178 ./mm/vmap.c
   121 ./entry/entry.S
    36 ./entry/start.S
     6 ./entry/logo.S
    10 ./entry/dtb.S
    58 ./entry/main.c
    10 ./entry/guest_bin.S
    10 ./entry/user_bin.S
   161 ./entry/trap.c
     6 ./entry/auto_run.S
   298 ./core/device_driver.c
    63 ./core/event.c
   376 ./core/irq/irq.c
    25 ./core/include/dmac.h
    87 ./core/include/irq.h
    23 ./core/include/timer.h
    38 ./core/include/clock.h
    10 ./core/include/uart.h
    46 ./core/include/task.h
   187 ./core/include/device.h
    64 ./core/include/cpu.h
    10 ./core/include/event.h
    29 ./core/include/percpu.h
    58 ./core/include/dma_mapping.h
    10 ./core/include/devicetree.h
    35 ./core/percpu.c
    64 ./core/dmac/dmac.c
    11 ./core/task/idle.c
   251 ./core/task/task.c
   156 ./core/cpu.c
   200 ./core/clock.c
   128 ./core/devicetree/devicetree.c
    78 ./core/uart/uart.c
    90 ./core/dma-mapping/gpa_mem_map.c
    90 ./core/dma-mapping/iova_mem_map.c
   174 ./core/dma-mapping/mapping.c
    88 ./core/timer/timer.c
     7 ./user/user_vmap.h
    77 ./user/user_vmap.c
    56 ./user/user_exception.c
   156 ./user/user_mode_switch.S
   126 ./user/user_memory.c
    61 ./user/syscall.c
   187 ./user/user.c
    19 ./user/user_pgtable.c
    13 ./user/user_memory.h
    19 ./user/user_exception.h
   366 ./mysbi/sbi/sbi.c
    22 ./mysbi/sbi/uart_uartlite.c
    52 ./mysbi/sbi/sbi_clint.c
    11 ./mysbi/sbi/sbi_clint.h
    42 ./mysbi/sbi/uart_ns16550a.h
    27 ./mysbi/sbi/uart_qemu-8250.h
    99 ./mysbi/sbi/sbi_entry.S
    15 ./mysbi/sbi/uart_uartlite.h
    49 ./mysbi/sbi/uart_ns16550a.c
    60 ./mysbi/sbi/sbi_uart.c
    33 ./mysbi/sbi/uart_qemu-8250.c
   134 ./mysbi/include/asm/csr.h
    26 ./mysbi/include/asm/mmio.h
    10 ./mysbi/include/asm/type.h
    41 ./mysbi/include/asm/trap.h
    44 ./mysbi/include/asm/asm-offsets.h
    41 ./mysbi/include/asm/sbi_asm_offsets.h
    28 ./mysbi/include/asm/barrier.h
    76 ./mysbi/include/sbi_trap.h
     7 ./mysbi/include/string.h
     6 ./mysbi/include/print.h
    14 ./mysbi/include/sbi_uart.h
    24 ./mysbi/include/device.h
    22 ./mysbi/include/sbi.h
    36 ./mysbi/entry/init.c
    85 ./mysbi/entry/start.S
    29 ./mysbi/lib/string.c
   128 ./mysbi/lib/print.c
    36 ./lib/spinlocks.c
   224 ./lib/string.c
   146 ./lib/print.c
    14 ./bsp/imsic_data.h
     8 ./bsp/clint.h
    11 ./bsp/plic.h
     6 ./bsp/plic.c
    90 ./bsp/hw-fpga.c
    13 ./bsp/clint.c
    10 ./bsp/imsic_data.c
    15 ./bsp/riscv_iommu_data.h
   171 ./bsp/hw-qemu.c
    12 ./bsp/riscv_iommu_data.c
   110 ./bsp/hw-st-cmn600.c
    29 ./bsp/aplic_data.c
    19 ./bsp/aplic_data.h
 26001 total
```
### （1）mysbi（M模式程序，编译生成mysbi.bin）：
```
mysbi/：M模式程序，编译生成mysbi.bin
  mysbi.lds   —— mysbi链接脚本
  mysbi/entry/（mysbi入口程序）：
    mysbi/entry/start.S   —— mysbi入口，也是整个裸机程序的入口
    mysbi/entry/init.c    —— mysbi c语言入口
  mysbi/sbi/（mysbi初始化代码、运行时代码）：
    mysbi/sbi/sbi.c             —— mysbi sbi ecall处理，状态切换...
    mysbi/sbi/sbi_clint.c       —— mysbi timer驱动
    mysbi/sbi/sbi_uart.c        —— mysbi uart抽象
    mysbi/sbi/uart_ns16550a.c   —— mysbi ns16550a驱动
    mysbi/sbi/uart_qemu-8250.c  —— mysbi qemu-8250驱动
    mysbi/sbi/uart_uartlite.c   —— mysbi uartlite驱动
  mysbi/lib/（mysbi lib）：
    mysbi/lib/print.c    —— 向mysbi提供print接口，向下调用uart抽象
    mysbi/lib/string.c   —— 向mysbi提供字符串处理接口
```
### （2）myUser（U态程序，编译生成myUser.bin）
```
myUser/：U模式程序，编译生成myUser.bin
  myUser/myUser.lds       —— myUser链接脚本
  myUser/entry/（myUser入口程序）:
    myUser/entry/start.S  —— myUser入口
    myUser/entry/main.c   —— myUser c语言入口
  myUser/core/（myUser核心层）:
    myUser/core/syscall.S    —— 提供系统调用接口
    myUser/core/command.c    —— 提供command注册，遍历等接口
    myUser/core/user_print.c —— 提供print接口
  myUser/command/（U态command）: —— 可以参考里面的hello.c创建一个command，Gos Shell输入user_run [command]来执行
    myUser/command/hello.c   —— myUser command示例
    myUser/command/ls.c      —— myUser command ls，显示所有user command
  myUser/lib/（myUser lib）:
    myUser/command/mallo.c     —— 提供内存分配接口
    myUser/command/spinlocks.c —— 提供自旋锁
    myUser/command/string.c    —— 提供string操作接口
```
### （3）myGuest（VS态程序，编译生成myGuest.bin）
```
myGuest/：VS模式程序，编译生成myGuest.bin
  myGuest/myGuest.lds     —— myGuest链接脚本
  myGuest/entry/（myGuest入口程序）：
    myGuest/entry/entry.S —— myGuest入口
    myGuest/entry/main.c  —— myGuest C语言入口
    myGuest/entry/trap.c  —— myGuest vs模式异常处理c入口
  myGuest/core/（myGuest核心层）：
    myGuest/core/command.c   —— 提供command注册，遍历等接口
    myGuest/core/device.c    —— 设备抽象，提供设备驱动接口
    myGuest/core/irq.c       —— 中断抽象，提供中断注册，分发等接口
    myGuest/core/timer.c     —— timer抽象，提供timer接口
    myGuest/core/uart.c      —— uart抽象，提供串口接口
  myGuest/mm/（myGuest内存管理）：
    myGuest/mm.c        —— 提供物理内存分配、释放接口
    myGuest/pgtable.c   —— 提供分页相关接口
    myGuest/vmap.c      —— 提供虚拟内存分配、释放接口
  myGuest/drivers/（myGuest驱动）：
    myGuest/drivers/clint.c     —— riscv timer驱动，向timer子系统注册
    myGuest/drivers/imsic.c     —— aia imsic驱动，向irq子系统注册
    myGuest/drivers/qemu-8250.c —— 8250串口驱动，向uart子系统注册
  myGuest/command/（myGuset command）：
    myGuest/command/hello.c          —— hello world
    myGuest/command/ls.c             —— 显示myGuest下所有command
    myGuest/command/imsic_test.c     —— vs模式msi中断测试case
    myGuest/command/vs_timer_test.c  —— vs模式timer测试case
```
### （4）gos（S态程序，编译生成gos.bin）
**（myUser.bin和myGuest.bin作为其payload，通过命令行加载执行）**
```
gos.lds              —— gos链接脚本
configs/（存放所有defconfig以及dts）
entry/（gos 入口）：
  entry/start.S      —— 汇编入口
  entry/main.c       —— C语言入口
  entry/entry.S      —— 异常处理汇编入口
  entry/trap.c       —— 异常处理C语言入口
  entry/auto_run.S   —— 包含了auto_run.bin（用于记录自启动的command）
  entry/dtb.S        —— 包含了dtb
  entry/logo.S       —— 包含了bosc log文件
  entry/guest_bin.S  —— 包含了myGuest.bin
  entry/user_bin.S   —— 包含了myUser.bin
fdt/（ libfdt源文件）
core/（gos核心层）:
  core/devicetree/devicetree.c       —— 提供设备树解析接口（目前只支持了cpu和memory）
  core/dma-mapping/（dma mapping抽象层）：
    core/dma-mapping/mapping.c       —— 提供dma_alloc以及dma-mapping等接口
    core/dma-mapping/iova_mem_map.c  —— 提供iova分配接口
  core/dmac/dmac.c                   —— dma engine抽象，提供dma_transfer及memcpy_hw（硬件memcpy）接口
  core/irq/irq.c                     —— 中断抽象，提供irqchip注册，中断分配，中断事件分发等接口
  core/task/：
    core/task/task.c                 —— task创建、调度等
    core/task/idle.c                 —— idle task的实现
  core/timer/timer.c                 —— timer接口
  core/uart/uart.c                   —— 串口打印的抽象
  core/clock.c                       —— clock event & clock source接口
  core/cpu.c                         —— cpu热插拔，second cpus bringup等接口
  core/device_driver.c               —— 设备驱动抽象
mm/（gos内存管理）：
  mm/mm.c           —— 提供物理内存分配及释放接口
  mm/vmap.c         —— 提供虚拟内存分配及释放接口
  mm/pgtable.c      —— 分页相关接口
  mm/flush_tlb.c    —— flush tlb接口
bsp/（不同machine上的bsp信息，目前设备相关还没有支持设备树，在bsp/下配置）
lib/（一些基础函数—— spinlocks、string、print）
virt/（虚拟化相关处理逻辑）：
  virt/virt.c            —— 实现vcpu_run主循环
  virt/virt_vm_exit.c    —— 实现vcpu退出的处理
  virt/vcpu_switch.S     —— 实现切换状态，跳转vs模式
  virt/machine.c         —— 实现machine的模拟
  virt/gstage_pgtable.c  —— 实现gstage页表映射
  virt/vcpu_timer.c      —— 实现虚拟timer
  virt/vcpu_aia.c        —— 实现aia中断虚拟化
  virt/vcpu_sbi.c        —— 实现对guest中sbi ecall的模拟
  virt/vcpu_insn.c       —— 实现对gstage缺页的处理
  virt/cpu_tlb.S         —— 实现gvma、vvma tlb flush
  virt/clint_emulator.c  —— 实现clint的模拟
  virt/imsic_emulator.c  —— 实现imsic的模拟
  virt/memory_emulator.c —— 实现memory的模拟
  virt/uart_emulator.c   —— 实现uart的模拟
user/（跳转U态相关处理逻辑）：
  user/user.c               —— 实现user_run主循环
  user/syscall.c            —— 实现系统调用
  user/user_exception.c     —— 实现user异常处理
  user/user_memory.c        —— 实现用户态内存的映射与管理
  user/user_mode_switch.S   —— 实现用户态和S态之间的状态切换
  user/user_pgtable.c       —— 提供用户态内存的映射接口
  user/user_vmap.c          —— 提供用户态虚拟内存接口
app/（shell以及command实现）：
  app/command.c             —— S态command注册与管理
  app/shell.c               —— 实现shell
  app/command/（S态command）： —— command，Shell下键入对应command，可以执行相应操作，可以参考hello.c创建一个新的command
     app/command/csr_ctl.c
     app/command/devices.c
     app/command/devmm.c
     app/command/dma_test.c
     app/command/dma_test_ext.c
     app/command/hello.c
     app/command/history.c
     app/command/imsic_test.c
     app/command/ls.c
     app/command/mem_read.c
     app/command/page_tlb_test.c
     app/command/plic_set_affinity.c
     app/command/spinlock_test.c
     app/command/task_info.c
     app/command/user_run.c
     app/command/vcpu_run.c
```
# 二、编译
gos支持Kbuild构建：
## make命令
make menuconfig      —— menuconfig
make xxx_defconfig   —— 匹配configs/下的defconfig，生成对应配置
make                          —— 全部编译（gos+myGuset+myUser），并打包成Image.bin（位于out/Image.bin）
make myUser_bin      —— 编译myUser，生成myUser.bin（位于out/myUser.bin）
make myGuest_bin    —— 编译myGuest，生成myGuset.bin（位于out/myGuset.bin）
make xxx.dtb             —— 编译设备树，匹配configs/dts下的dtb
make run                   —— 使用qemu/nemu运行
make run-debug       —— 使用qemu -S -s选项debug
make clean                —— 全部clean
make myUser-clean   —— clean User
make myGuest-clean —— clean myGuest
make format              —— 格式化
make fpga                 ——  生成用于fpga烧录的data.txt
建议每次更改config后（make menuconfig/make xxx_defconfig）后先clean再make
## build命令
为了方便对不同相同进行构建，build.sh中打包了make命令：
./build.sh default                —— 编译用于qemu运行的最完整系统
./build.sh fpga                    —— 编译用于fpga运行的最完整系统，并直接生成data.txt
./build.sh vcs-minimum     —— 编译用于vcs运行的最小系统
./build.sh run                      —— qemu/nemu运行
./build.sh run-debug         —— qemu下进行debug
./build.sh clean                  —— clean
# 三、gos
gos.bin运行在S态，是该软件主体。
总体上分三层，core是中间层，是对各个子系统的抽象，向drivers层提供注册接口对接具体设备驱动，向app层提供调用接口；app层原则上不直接调用driver层，而是调用core层提供的抽象接口。
## 1、主要子系统
### （1）mm
mm子系统提供了物理内存分配、虚拟内存分配以及分页的相关接口。
#### 物理内存分配：
采用bitmap的内存管理策略，一个bit代表一个PAGE_SIZE的物理内存页，因此该内存分配器的粒度为PAGE_SIZE（TODO：小粒度内存分配器）。
mm_alloc(size) —— 分配大小为size（UP到PAGE_SIZE粒度）的物理地址，根据是否使能mmu，返回物理/线性映射区的虚拟地址；
mm_free(void *addr, unsigned int size); —— 释放物理内存；
#### 虚拟内分配：
vmap_alloc(unsigned int size); —— 从vmap虚拟地址区域分配虚拟内存
vmap_free(void *addr, unsigned int size); —— 释放虚拟内存
ioremap(void *addr, unsigned int size, int gfp); —— 从vmap虚拟地址区域分配一片内存并映射addr
iounmap(void *addr, unsigned int size); —— 释放ioremap分配的内存
#### 虚拟内存+物理内存分配：
vmem_alloc(void *addr, unsigned int size, int gfp); —— 从vmap虚拟内存区域分配虚拟内存，分配物理内存，并建立映射
vmem_free(void *addr, unsigned int size); —— 释放vmem_alloc中分配的虚拟内存和物理内存
vmem_alloc_lazy(unsigned int size, int gfp); —— 从vmap虚拟内存区域分配虚拟内存，并建立页表，但没有分配物理内存（真正访问引起缺页异常时分配）
#### 分页：
mmu_page_mapping —— 建立页表
mmu_user_page_mapping —— 建立user空间页表
mmu_gstage_page_mapping —— 建立gstage页表
mmu_page_mapping_lazy —— 建立页表，但不真实分配物理页
do_page_fault —— 缺页异常处理
。。。。。。
### （2）device_driver
device的信息放在device_init_entry中，包括：compatible、base address、irq_parent(连接的中断控制器)、硬件中断号（中断控制器域中的）、以及一些私有信息。这些设备描述信息配置在bsp/hw-xxx.c中，并由mysbi传入到gos。
driver通过DRIVER_REGISTER注册，包括用来和设备匹配的compatible以及匹配成功后调用的init_fn，driver的整个结构被放到__driver_init_table段。
gos启动过程中会遍历__driver_init_table段，与device相匹配，匹配成功会创建device和driver，并调用驱动的init_fn函数（传入device_init_entry）。
（TODO：设备描述放到设备树中）
#### 驱动注册：
提供接口：
DRIVER_REGISTER(name, init_fn, compat)；—— 注册一个驱动，当compat与设备匹配时调用init_fn。
#### 设备注册：
在bsp/hw-xxx.c中配置设备描述信息，其中compat与驱动匹配时会调用驱动的init_fn。
#### 其他接口：
open(char *name); —— 打开一个设备，调用driver_ops的open
write(int fd, char *buf, unsigned long offset, unsigned int len)；—— 写一个设备，调用driver_ops的write
read(int fd, char *buf, unsigned long offset, unsigned int len); —— 读一个设备，调用driver_ops的read
ioctl(int fd, unsigned int cmd, void *arg); —— 控制一个设备，调用driver_ops的ioctl
### （3）irq
中断子系统向driver层具体的中断控制器驱动提供中断控制器注册接口（每一个中断控制器在gos中都会创建一个irq_domain结构）；与此同时向外设驱动提供中断处理注册接口；并提供中断处理C语言入口，提供中断分发的功能。
支持线中断与msi中断。
#### 中断控制器初始化：
irqchip_setup —— __irqchip_init_table段中的中断控制器驱动与device_init_entry中的设备信息匹配，匹配成功则运行相应中断控制器驱动的init_fn函数。
#### 中断控制器注册接口：
irq_domain_init —— 注册一个中断控制器
irq_domain_init_cascade —— 注册一个中断控制器，级联在parent上（参数传入）
find_irq_domain —— 根据name等到一个中断控制器的irq_domain
（device_init_entry中需要配置中断控制器连接拓扑，比如aplic->imsic->intc，create_device接口中会调用find_irq_domain得到该中断控制器的parent并赋值到device，中断控制器驱动中只需要调用irq_domain_init_cascade传入device中已赋值好的irq_parent即可）。
msi_domain_init —— 注册一个msi irq domain
msi_domain_init_hierarchy —— 注册一个有层次接口的msi irq domain，比如注册aplic(msi_mode) irq domain的时候，它需要imsic作为其parent，而最终中断分发时，直接在imsic irq domain域就完成中断分发。
#### 中断分发接口：
handle_irq —— 中断处理C语言入口
domain_handle_irq —— 传入irq_domain和hwirq，调用下一层irq handler（有可能是下一级中断控制器，也有可能到设备的中断处理）；
#### 外设注册中断处理接口：
get_hwirq —— 获取外设的线中断硬件中断号
msi_get_hwirq —— 获取外设的msi中断硬件中断号
msi_get_hwirq_affinity —— 获取外设的msi中断硬件中断号，并设置亲缘性
register_device_irq —— 外设驱动注册一个中断handler
#### 其他：
find_irq_info —— 通过irq_domain得到irq_info，irq_info中有hwirq、irq_handler等接口
irq_domain_set_affinity —— 设置中断亲缘性接口
domain_activate_irq —— 中断激活接口
。。。。。。
### （4）dma-mapping
### （5）clock
clock子系统提供了clock_source、clock_event以及其他相关接口：clock_source提供了系统时钟的获取功能；clock_event提供了时钟事件处理功能，实现了一个tickless的高精度时钟，为task调度、timer定时器以及虚拟中断注入提供了支持。
clock子系统向driver层具体的timer驱动提供了clock source以及clock event的注册接口；向task子系统，timer子系统等提供了timer注册接口。
#### clocksource接口：
register_clock_source —— 注册clock source
get_clocksource_counter —— 从clock source上获取时间
get_clock_source_freq —— 获取当前clock source的时钟频率
get_system_tick —— 获取当前system tick
#### clockevent接口：
register_clock_event —— 注册clock event，它是个percpu变量，管理一个cpu上所有的timer事件
register_timer_event —— 注册timer event，所有timer会挂在timer event上，根据到时时间设置下一次clock event的中断
do_clock_event_handler —— 由timer驱动调用，处理到时timer
#### 其他：
cycles_to_ms
ms_to_cycles
### （6）timer
timer子系统提供了timer初始化入口，会调用到driver层具体的timer驱动；并向上层提供设置定时器接口：
#### 设置定时器接口：
set_timer —— 设置定时器，到时时调用传入的回调
#### 系统timer初始化入口：
init_timer —— 系统初始化时被调用。传入device_init_entry，遍历其中的所有设备，与__timer_init_table段中的timer驱动匹配，匹配成功调用timer驱动初始化函数
### （7）task
task子系统提供了创建task的接口，该task被注册后，会在后面某时候被调度运行；并提供了task调度以及上下文切换逻辑。
task子系统会调用clock子系统的register_timer_event接口注册scheduler定时器，以10ms为周期触发，scheduler的event handler中会按顺序调度已注册的task运行。创建的task执行完毕后，会由task子系统的task_fn_wrap负责释放掉task占据的内存并销毁掉进程。
task子系统还提供了一个do_idle函数，在secondary cpus被bringup起来并做好基本的初始化工作后便会进入idle，直到在该cpu上被创建了task。一个cpu上有task时会以10ms的粒度挨个执行这些task，当没有可执行的task时会再次进行idle。
#### task创建接口：
create_task —— 创建一个task，在指定的cpu上执行，执行的函数为参数中传入的fn
#### 查看系统中的task：
walk_task_per_cpu —— 查看某一个cpu上的所有task
walk_task_all_cpu —— 查看所有cpu上的所有task
### （8）uart
uart子系统提供了uart初始化，early print初始化接口，他们会调用到driver层具体的串口驱动，向串口驱动提供注册接口；并向上层的打印函数提供抽象接口。
#### 串口打印接口：
uart_putc —— 串口字符输出
uart_puts —— 串口字符串输出
### （9）virt
virt子系统提供了虚拟化的能力，包括：vcpu_run主循环、machine的模拟、中断虚拟化的处理、timer虚拟化的处理、虚拟机异常的处理、虚拟机和host上下文切换处理等。
目前，系统中只存在一个vcpu，host可以多次向vcpu发送command，vcpu运行在VS模式的大循环中，当host向vcpu发送command时，vcpu会去处理相应的命令（在myGuest中注册一个command）。
vcpu在gos中是作为一个task存在的，当在某个task中第一次调用virt子系统提供的vcpu_run接口时会启动vcpu，当后面再次调用vcpu_run时不会再次创建task，而只是将参数中的command传入myGuest。
#### vcpu注册及运行：
vcpu_create —— 创建一个vcpu
vcpu_run —— vcpu运行
#### 发送request：
vcpu_set_request —— 向vcpu发送request（比如发送hfence.gvma刷新vcpu的gstage tlb）
#### 中断虚拟化：
目前支持aia的中断直通：virt中对imsic interrupt file进行了模拟，myGuest中可以编写驱动，将msi中断直通到VS模式，测试命令：vcpu_run imsic_test。
#### timer虚拟化：
目前通过在host中注册timer，hvip注入timer中断的方式支持timer虚拟化，这需要在virt中模拟一个clint，测试命令：vcpu_run timer_test
### （10）user
user子系统提供了gos进入U态的能力，其实现逻辑和virt大致相同，支持一个task进入U态，并在U态的大循环，等待gos中传入command。
gos中使用user_run便会使该task进入U态，后面再次在其他task中调用user_run，不会进入U态，而是向处于U态中的task传入command，并处理。
virt中提供了user_run大循环、syscall接口、U模式异常处理、U态物理/虚拟内存映射及管理、host和U态上下文切换等功能。
#### user注册及运行：
user_create —— 创建user
user_mode_run —— user运行
#### user页表映射：
user_page_mapping
## 2、Drivers
### dmac：
dw axi dmac
### iommu：
riscv iommu
### irqchip：
plic
aia
### timer：
clint
### uart：
qemu-8250
ns16550a
uartlite
# 四、命令
## 1、gos（S态命令）
### hello
command示例，输出 “Hello Bosc gos Shell!!!”
用法：hello
### ls
列出目前gos下支持的所有command
用法：ls
### devices
列出目前gos下注册的所有device（和driver匹配的）
用法：devices
### devmem（待修复）
mmio读写
用法：devmem [ADDRESS] [WIDTH] [VALUE]]
### history
列出之前执行过的所有命令
用法：history
### mem_read
内存读取
用法：mem_read [ADDRESS] [COUNT]
### task_info
列出目前系统中所有task
用法：
task_info [CPU] —— 列出指定cpu上的所有task
task_fino           —— 列出所有cpu上的所有task
### csr_ctl
hpm csr读写
用法: 
csr_ctl [mode] [csr_num] [value]
mode option:
    	-- 1 (read csr register)
    	-- 2 (write csr register)
    	-- 3 (read mcounteren register)
         	--> csr_ctl  3  0  0
    	-- 4 (write mcounteren register)
         	--> csr_ctl  4  0 value
----------------------------------------------
csr_num option:
    	-- csr register
----------------------------------------------
value :
    	-- write csr_register value
### dma_test
dmac测试
用法：dma_test [dst_addr] [src_addr] [size]
### dma_test_ext
### imsic_test
imsic测试
用法：imsic_test
### page_tlb_test
sfence测试：
用法：
Usage: page_tlb_test [cmd] [param] [control]
cmd option:
    		-- Acc (page table access bit test)
    		-- Lazy (demanding page allocating test)
    		-- sfence.vma_all (sfence.vma test)
    		-- sfence.gvma_all (sfence.gvma test)
    		-- remapping_gstage_memory_test
    		-- satp_bare_test (set satp is bare mode)
    		-- pte_flag_test (modify pte flag bit)
    		-- sfence.addr (flush spec addr)
    		-- sfence.asid (flush spec asid)
param option:
    		-- cmd: pte_flag_test (modify pte flag bit)
   		 param value is 0x1ff, display current pte value, the param flag bits are as follows
    		 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
       		reserved for SW   D   A   G   U   X   W   R   V
control option:
    		-- cmd: pte_flag_test (modify pte flag bit)
       		value=1(After changing the page table, first access the page table and then flush the cache)
       		value=2(After changing the page table, first flush the cache and then access the page table)
### plic_set_affinity
plic亲缘性测试
用法：
plic_set_affinity [hwirq] [cpu_id]
### spinlock_test
自旋锁测试：
用法：spinlock_test
### vcpu_run
运行vcpu/向vcpu发送command
用法：vcpu_run [cmd] [oarams1] [params2] ...
### user_run
运行user/向user发送command
用法：user_run [cmd] [params1] [params2] ...
## 2、myGuest（VS态命令）
### ls
列出myGuset下支持的所有command
vcpu_run ls
### imsic_test
中断虚拟化直通测试
用法：vcpu_run imsic_test
### timer_test
timer虚拟化测试
用法：vcpu_run timer_test
### memory_test
## 3、myUser（U态命令）
### hello
myUser command示例：
用法：user_run hello
### ls
列出myUser下所有command
用法：user_run ls
### pte
user pte测试：
用法：user_run pte
### sct_test
user hpm csr测试
用法：
sct_test [mode] [csr_num]
mode option:
-- 0 (RAV23 profile Zicntr test)
 			--> user_run sct_test 0
-- 1 (RAV23 profile Zihpm test)
--> user_run sct_test 1
-- 2 (read csr register)
----------------------------------------------
csr_num option:
-- csr register
# 五、command自启动
在auto_run下逐个写入要执行的命令即可。

