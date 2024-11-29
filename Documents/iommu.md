# 一、概述
Iommu模块向gos提供外设连接iommu时地址映射的能力。用户通过hw-xxx.c文件中的配置告知gos外设连接iommu的情况，Iommu模块提供接口，使得device模块可以通过该信息在设备创建时attach到相应的iommu。除此之外，iommu模块还提供了iommu驱动注册，dma地址映射，io虚拟化直通group划分等功能接口。

# 二、相关接口
## iommu_register
通常由iommu驱动调用，向iommu模块注册一个iommu，device模块创建设备时会根据hw-xxx.c中配置的信息，从iommu模块找到对应的iommu。

## iommu_attach_device
通常由device模块在创建设备时调用，设备和iommu相关联

## iommu_dettach_device
设备和iommu取消关联

## iommu_device_attach_group
设备关联到一个iommu group，处于同一个group的iommu共享同一地址空间

## iommu_device_dettach_group
设备和一个iommu group取消关联

## iommu_alloc_group
分配一个iommu group

## iommu_get_group
获取一个iommu group

## iommu_map_pages
iommu地址映射（建立页表）

# 三、iommu mode
目前支持S1 only与S2 only，S1 + S2处于ToDo状态......

用户对于iommu无感，当hw-xxx.c中配置了设备与iommu的连接关系，当该外设分配一段dma内存时（详见dma-mapping.md）接口内部会调用iommu接口建立iova和pa的映射关系（Stage 1）。

在io虚拟化直通中，如果在vcpu_create_force接口时传入了要直通的设备（或者直接使用vcpu_run_ext命令时通过"pt="传入要直通的设备），在virt模块中会建立gpa和pa的映射关系（Stage 2）。当直通的设备支持msi时，msi的地址也在会在iommu中建立S2的映射关系。

