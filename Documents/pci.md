# 一、概述
pci模块为gos提供了驱动pcie设备的能力。简单分三部分：pci_device_driver、pci core以及pci msi。

# 二、pci device driver
pci device driver（core/pci_device_driver.c）提供了pci驱动程序注册，pci设备创建，pci设备驱动匹配的功能。

## PCI_DRIVER_REGISTER
注册一个pci driver，其定义为：

PCI_DRIVER_REGISTER(name, init_fn, vid, did)

当pci core中枚举到一个pci设备，并且其vid & did与PCI_DRIVER_REGISTER中配置的vid & did匹配时，init_fn函数（pci设备驱动程序的入口）会被调用。

## pci_register_device
注册一个pci device，该接口通常在pci core中枚举到一个设备时被调用，用户一般不涉及。

## pci_probe_driver
当pci core中枚举到一个pci设备，并且其vid & did与PCI_DRIVER_REGISTER中配置的vid & did匹配时，调用pci driver的入口函数。

## walk_pci_devices
遍历并打印所有的pci 设备，如果传入参数print_conf为1时，打印每个设备的config空间。

# 三、pci core
pci core（drivers/pci/pci.c & drivers/pci/msi.c）向pci controller驱动提供了pci总线枚举以及msi相关接口，其中还会调用pci devicce driver模块中的接口创建pci设备并匹配pci驱动程序。

与此同时，它还向pci设备驱动程序提供或者pci resource、msi中断等能力。

## pci_root_bus_init
通常由pci控制器驱动调用，初始化root bus

## pci_probe_root_bus
通常由pci控制器驱动调用，在pci总线上枚举pci设备

## pci_write_config_dword
pci设备配置空间双字写

## pci_write_config_word
pci设备配置空间单字写

## pci_write_config_byte
pci设备配置空间单字节写

## pci_read_config_dword
pci设备配置空间双字读

## pci_read_config_word
pci设备配置空间单字读

## pci_read_config_byte
pci设备配置空间单字节读

## pci_find_capability
获取某个pci设备的capability

## pci_enable_resource
使能某个pci设备的resource

## pci_get_resource
获取某个pci设备的resource

## pci_set_master
是能某个pci设备的master能力

## pci_msi_init
初始化某个pci设备的msi

## pci_msix_init
初始化某个pci设备的msix

## pci_msix_enable
是能某个pci设备的msix

## pci_msix_get_vec_count
获取某个pci设备的msix中断数量

## pci_get_config
读取某个pci设备的config空间内容

# 四、pci控制器驱动
代码位于drivers/pci/controller

目前支持pci ecam generic，该驱动可以配合qemu virt machine的pci controller使用。

该驱动主要是调用了pci core中的接口进行总线c枚举。

如果需要添加新的rc驱动，可以参照该驱动。

# 五、pci设备驱动
参照drivers/dmac/my_pci_dmaengine.c添加即可。

