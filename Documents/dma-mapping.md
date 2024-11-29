# 一、概述
dma-mapping向用户提供了分配dma内存的功能。

# 二、相关接口
## dma_alloc
分配一片dma内存，如果该设备连接了iommu，其中会调用iommu的相关接口完成对iova和pa的映射，并返回iova

## dma_mapping
将一片物理地址与iova映射

