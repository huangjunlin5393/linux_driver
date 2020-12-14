使用方法

# 通过 `modname` 制定待卸载驱动的信息

sudo insmod force_rmmod.ko modname=卸载驱动名称

sudo rmmod 卸载驱动名称



（1）现象

rmmod: ERROR: Module XXX is in use
（2）其本质就是模块的模块的引用计数不为 0, 要解决此类问题, 只需要将模块的引用计数强制置为 0 即可.

    查找到 none_exit 模块的内核模块结构 struct moudle, 可以通过 find_module 函数查找到, 也可以参照 find_module 函数实现.

