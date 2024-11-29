# 一、概述
提供接口，创建percpu变量，不同的cpu拥有自己单独的副本

# 二、相关接口
## DEFINE_PER_CPU(type, name)
创建一个percpu变量

## per_cpu(var, cpu)
根据传入的cpu值得到对应cpu的副本

