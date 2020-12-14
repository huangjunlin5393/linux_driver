i2c,gpio,spi等低速总线驱动放在用户态


通过mmap映射/dev/mem下的节点，将CPU外围控制资源映射到用户空间，可直接访问
