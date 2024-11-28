# 一、概述
·	irq模块向gos提供了中断注册与中断调用的能力。

# 二、分层结构
## 1、core（core/irq/）
 	irq模块向driver层具体的中断控制器驱动提供中断控制器注册接⼝（每⼀个中断控制器在gos中 都会创建⼀个irq_domain结构）；与此同时向外设驱动提供中断处理注册接⼝；并提供中断处理C语⾔入口，提供中断分发的功能。

⽀持线中断与msi中断。  

## 2、driver（driver/irqchip/）
具体的中断控制器driver会调用core层中提供的中断domain注册接口注册中断控制器。

目前gos中支持的中断控制器有：plic & imsic & aplic（msi mode），驱动路径：

plic：driver/irqchip/plic/

imsic：driver/irqchip/imsic/

aplic：driver/irqchip/aplic/

如果需要添加新的中断控制器驱动，参照其一即可。

# 三、相关接口
##  1、中断控制器初始化：  
## irqchip_setup
__irqchip_init_table段中的中断控制器驱动与device_init_entry中的设备信息匹 配，匹配成功则运⾏相应中断控制器驱动的init_fn函数。

##  2、中断控制器注册接口：  
### irq_domain_init
注册⼀个中断控制器

### irq_domain_init_cascade
注册⼀个中断控制器，级联在parent上（参数传⼊）

### find_irq_domain
根据name等到⼀个中断控制器的irq_domain （device_init_entry中需要配置中断控制器连接拓扑，⽐如aplic->imsic->intc，create_device接 ⼝中会调⽤find_irq_domain得到该中断控制器的parent并赋值到device，中断控制器驱动中只需要调⽤ irq_domain_init_cascade传⼊device中已赋值好的irq_parent即可）

### irq_domain_set_affinity
设置某一个irq_domain中某个hwirq的亲缘性（将中断发给哪个cpu）

### msi_domain_init 
注册⼀个msi irq domain

### msi_domain_init_hierarchy
注 册 ⼀ 个 有 层 次 接 ⼝ 的 msi irq domain ， ⽐ 如 注 册 aplic(msi_mode) irq domain的时候，它需要imsic作为其parent，⽽最终中断分发时，直接在imsic irq domain域就完成中断分发

##  3、中断分发接口：  
###  handle_irq 
中断处理C语⾔⼊⼝

### domain_handle_irq
传⼊irq_domain和hwirq，调⽤下⼀层irq handler（有可能是下⼀级中断 控制器，也有可能到设备的中断处理）  

##  4、外设注册中断处理接口：  
###  get_hwirq
获取外设的线中断硬件中断号

### msi_get_hwirq
获取外设的msi中断硬件中断号

### msi_get_hwirq_affinity
获取外设的msi中断硬件中断号，并设置亲缘性

### register_device_irq
外设驱动注册⼀个中断handler  

# 四、外设驱动程序中注册并使用中断
## 1、线中断
a、在hw-xxx.c中的irq_parent字段配置外设中断信号连接在哪个中断控制器上，在irq字段配置连接的硬件中断号；

b、调用get_hwirq获取中断号；

c、调用register_device_irq并传入获取的中断号注册一个中断handler。

## 2、msi中断
a、调用msi_get_hwirq获取中断号（如果是pci设备的话调用pci_msix_enable）；

b、调用register_device_irq并传入获取的中断号注册一个中断handler。

