# 一、概述
mm模块向gos提供了内存分配（虚拟内存分配 & 物理内存分配）、地址映射（sv39/sv39x4 & sv48/sv48x4 & sv57/sv57x4）、大页内存映射（2M & 1G & 64K）、基于页表的内存属性设置（svpbmt）等接口；

并在gos系统初始化过程中，建立并管理所有可用内存。

# 二、相关接口
## 1、物理内存分配
采用bitmap的内存管理策略，⼀个bit代表⼀个PAGE_SIZE的物理内存页，因此该内存分配器的粒度 为PAGE_SIZE：

### mm_alloc(size) 
分配大小为size（会向上对齐到PAGE_SIZE粒度）的物理地址，根据是否使能mmu， 返回物理/线性映射区的虚拟地址

### mm_alloc_align(align, size) 
分配大小为size，按align大小对齐的内存

### mm_free(void *addr, unsigned int size); 
释放物理内存

## 2、虚拟内分配
### vmap_alloc(unsigned int size); 
从vmap虚拟地址区域分配虚拟内存

### vmap_free(void *addr, unsigned int size); 
释放虚拟内存

### ioremap(void *addr, unsigned int size, int gfp); 
从vmap虚拟地址区域分配一片内存并映射 addr

### iounmap(void *addr, unsigned int size); 
释放ioremap分配的内存  

## 3、虚拟内存+物理内存分配
### vmem_alloc(void *addr, unsigned int size, int gfp); 
从vmap虚拟内存区域分配虚拟内存，分配物理内存，并建立映射 

### vmem_free(void *addr, unsigned int size); 
释放vmem_alloc中分配的虚拟内存和物理内存

### vmem_alloc_lazy(unsigned int size, int gfp); 
从vmap虚拟内存区域分配虚拟内存，并建立页表，但没有分配物理内存（真正访问引起缺页异常时分配）  

## 4、分页
### mmu_page_mapping
建立页表

### mmu_user_page_mapping 
建立user空间页表

### mmu_gstage_page_mapping
建立gstage页表

### mmu_page_mapping_lazy
建立页表，但不真实分配物理页

### do_page_fault
缺页异常处理

### mmu_get_pte
传⼊⼀个虚拟地址，得到其叶子

### pte mmu_get_pte_level
传入⼀个虚拟地址及pte的level，得到对应的pte

### 使用不同分页模式（sv mode）的方法
目前支持sv39、sv48、sv57，默认情况下采⽤sv39

如果使⽤sv48，按如下方法编译gos：

make gos-dualcore-Sv48.dtb

make Sv48_defconfig

make

或者直接使⽤build.sh编译：./build.sh default-Sv48



如果使⽤sv57，按如下⽅法编译gos：

make gos-dualcore-Sv57.dtb

make Sv57_defconfig

make

或者直接使⽤build.sh编译：./build.sh default-Sv57  

## 5、大页内存分配
###  vmem_alloc_huge
第⼆个参数（page_size）传入要分配的大页粒度，目前支持：

PAGE_64K_SIZE：64K大页（对用户是大页，本质上是NAPOT）

PAGE_2M_SIZE：2M大页

PAGE_1G_SIZE：1G大页

## 6、小内存分配
小内存分配器（tiny）从物理内存分配器（mm_alloc）中分配整⻚内存，将其分成8、16、32、 64、128、256、512、1024、2048字节的粒度分配给用户。

### tiny_alloc(size)
分配小内存 

### tiny_free(addr)
释放小内存 当使用mm_alloc分配内存，但输⼊的size参数小于PAGE_SIZE时，mm_alloc会退化成tiny_alloc。  

