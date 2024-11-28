# 一、概述
stub模块向gos提供了函数插桩的能力。用户可以通过register_stub向某个函数插入一个stub函数，当该函数执行时，该stub函数被执行。

# 二、相关接口
## register_stub
向某个函数插入一个stub

## unregister_stub
注销某个stub

## register_handle_exception_stub_handler
向handle_exception函数插入stub时stub函数中的参数获取需要一些技巧，因此gos中封装了该接口，使得stub函数中的参数获取和使用register_stub插桩时stub函数拿到的参数无异

## unregister_handle_exception_stub_handler
注销向handle_exception函数插入的stub

## register_ebreak_stub_handler
向ebreak函数插入stub时stub函数中的参数获取需要一些技巧，因此gos中封装了该接口，使得stub函数中的参数获取和使用register_stub插桩时stub函数拿到的参数无异

## unregister_ebreak_stub_handler
注销向ebreak函数插入的stub

## default_stub_handler
一个默认的stub handler，当使用stub命令向任意一个函数插桩并且该函数被调用时default_stub_handler会被调用

# 三、stub命令
gos中提供了一个命令可以向任意函数进行插桩，用法：

stub xxxx（gos中的任意函数名）

