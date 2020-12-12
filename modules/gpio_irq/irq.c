/****************************************************************************
Desc		:	driver for irq0/1/2/3
Version		:	1.0.0
Date 		: 	2013.7.31
Author 		: 	huangjunlin
*****************************************************************************/
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/ioport.h> 
#include <linux/interrupt.h> 
#include <linux/delay.h> 
#include <linux/highmem.h> 
#include <linux/uaccess.h> 
#include <linux/irq.h> 
#include <linux/io.h> 
#include <linux/platform_device.h> 

#define DRIVER_NAME "DEV_IRQ" 
 
#define INTERRUPT_IRQ0_VECTOR_NUM 48 		//IRQ0中断号
#define INTERRUPT_IRQ1_VECTOR_NUM 17 		//IRQ1中断号
#define INTERRUPT_IRQ2_VECTOR_NUM 18 		//IRQ2中断号
//#define INTERRUPT_IRQ3_VECTOR_NUM 19 		//IRQ3中断号

#define IMMRBAR			0xE0000000
#define SICRL				(IMMRBAR+0x00114)
#define SPRIDR				(IMMRBAR+0x00108)

#define IPIC_BASE			(IMMRBAR+0x0700)
#define IPIC_MAP_LEN		0x4F
#define SICFR				IPIC_BASE
#define SIVCR				(0x4)
#define SEPNR				(0x2C)
#define SEMSR				(0x38)
#define SECNR				(0x3C)			//外部中断控制寄存器
#define SEPCR				(0x4C)

static int major = 0; /*  动态分配 major号 */

module_param(major, int, 0);
static struct class *irq_class;

//#define TEST_IRQ1_DEBUG 1

#ifdef TEST_IRQ1_DEBUG 
#define DBG(fmt, args...) printk(KERN_INFO "[%s] " fmt "\n", __func__, ## args) 
#else 
#define DBG(fmt, args...) do {} while (0) 
#endif

volatile u32 __iomem 	*ipicreg;
u8 						irqFlag =0;
static spinlock_t 		lock; //自旋锁 
int icnt =0;
static inline u32 read_ipic_reg(volatile u32 __iomem *addr, u32 reg)
{return in_be32(addr+reg);}
static inline void write_ipic_reg(volatile u32 __iomem *addr,u32 reg, u32 value)
{out_be32(addr+reg, value);}


//中断处理程序 
static irqreturn_t detect_irq(int irq, void *dev_id) 
{ 
	u32 temVal=0;
	spin_lock(&lock);
	
#if 0
	if(icnt++ == 100)
	{
 		printk("IRQ happens:%d!\r\n", irq);
		icnt = 0;
	}
#endif

#if 1
 	temVal = read_ipic_reg(ipicreg, SEPNR);
	irqFlag = temVal>>28;
#endif
	
	write_ipic_reg(ipicreg, SEPNR, 0xF0000000);//清除中断标记
 	spin_unlock(&lock); 
 	return 0; 
}

//以下为文件操作接口定义 
static int irq_open(struct inode *inode, struct file *filp) 
{ return 0; } 

static int irq_relea(struct inode *inode, struct file *filp) 
{ return 0;} 

static int irq_ioctl(struct device *dev, unsigned int cmd,unsigned long arg) 
{
	unsigned long addr = arg;
  	unsigned int ret = 0;

  	DBG(KERN_INFO "addr is 0x%lx\n",addr);		
	
  	if(irqFlag&&copy_to_user((void*)&irqFlag, (void*)addr, sizeof(irqFlag)))
  	{
    	ret =  -EFAULT;
    	DBG(KERN_INFO "ioctl failed\n");
		return ret;
  	}
	
  	irqFlag = 0;//读取此值后对该值进行清除
  	return 0;
}
static struct file_operations irq_fops =
{ 
	.open   = irq_open, 
 	.release = irq_relea,
 	.ioctl  = irq_ioctl, 
 	.owner  = THIS_MODULE 
};

static int irq_probe(struct platform_device *pdev) 
{ 
	int ret =0; 
 	struct device *dev; 
 	dev_t devt; 
   	u32 temVal, irqSoftNum=0;
 	DBG(KERN_INFO "%s: start probe\n", __func__);  

	temVal = read_ipic_reg(ipicreg, SECNR);
	
  	DBG(KERN_INFO "read SECNR reg1: 0x%x\n", temVal); 
	
	temVal |= 0x0000F000;
	write_ipic_reg(ipicreg, SECNR,temVal);
	temVal = read_ipic_reg(ipicreg, SECNR);
	
  	DBG(KERN_INFO "read SECNR reg2: 0x%x\n", temVal); 
	
	write_ipic_reg(ipicreg, SEMSR, 0x0FFF0FFF); //unmask the irq and enable irq0
	write_ipic_reg(ipicreg, SEPCR, 0);			   //set the signal low valid 
	write_ipic_reg(ipicreg, SEPNR, 0xF0000000);//清除中断标记

	irqSoftNum = irq_create_mapping(NULL, INTERRUPT_IRQ0_VECTOR_NUM);

	DBG(KERN_INFO "irq_create_mapping0: %d\n",irqSoftNum);
 	ret = request_irq(irqSoftNum,detect_irq, IRQF_DISABLED, pdev->name, pdev); //申请中断
  	if (ret)
	{ 
  		DBG(KERN_INFO "%s: request irq fail0 %x\n", __func__, ret); 
 	}

	irqSoftNum = irq_create_mapping(NULL, INTERRUPT_IRQ1_VECTOR_NUM);
	DBG(KERN_INFO "irq_create_mapping1: %d\n",irqSoftNum);
 	ret = request_irq(irqSoftNum,detect_irq, IRQF_DISABLED, pdev->name, pdev); //申请中断
  	if (ret)
	{ 
  		DBG(KERN_INFO "%s: request irq fail1 %x\n", __func__, ret); 
 	}

	irqSoftNum = irq_create_mapping(NULL, INTERRUPT_IRQ2_VECTOR_NUM);
	DBG(KERN_INFO "irq_create_mapping2: %d\n",irqSoftNum);
	ret = request_irq(irqSoftNum,detect_irq, IRQF_DISABLED, pdev->name, pdev); //申请中断
  	if (ret)
	{ 
  		DBG(KERN_INFO "%s: request irq fail2 %x\n", __func__, ret); 
 	}

#if 0
	irqSoftNum = irq_create_mapping(NULL, INTERRUPT_IRQ3_VECTOR_NUM);
	DBG(KERN_INFO "irq_create_mapping3: %d\n",irqSoftNum);
	ret = request_irq(irqSoftNum,detect_irq, IRQF_DISABLED, pdev->name, pdev); //申请中断
 	if (ret)
	{ 
  		DBG(KERN_INFO "%s: request irq fail3 %x\n", __func__, ret); 
 	}
#endif
 	spin_lock_init(&lock);
 	devt = MKDEV(major, 0); 
 	dev = device_create(irq_class, &pdev->dev, devt, NULL, DRIVER_NAME); 
 	ret = IS_ERR(dev) ? PTR_ERR(dev) : 0; 
 	if (ret) 
 	{
 		DBG(KERN_ERR "unable to create %s class\n", DRIVER_NAME);
  	}
 	return ret; 
}

static int irq_remove(struct platform_device *pdev) 
{ 
  	dev_t devt;  
  	DBG("%s: remove\n", __func__);  
 	devt = MKDEV(major, 0);  
 	device_destroy(irq_class,devt);
	
 	#if 0
 		MCF_EPORT_EPIER = MCF_EPORT_EPIER & (~MCF_EPORT_EPIER_EPIE1); 
 		MCF_EPORT_EPFR = MCF_EPORT_EPFR|MCF_EPORT_EPFR_EPF1; 
 	#endif
	
 	free_irq(INTERRUPT_IRQ0_VECTOR_NUM, pdev);  
	free_irq(INTERRUPT_IRQ1_VECTOR_NUM, pdev);  
 	free_irq(INTERRUPT_IRQ2_VECTOR_NUM, pdev);  
// 	free_irq(INTERRUPT_IRQ3_VECTOR_NUM, pdev);  
 
 return 0; 
} 

/*-------------------------------------------------------------------------*/ 
static struct platform_driver irq_driver = { 
 .probe =  irq_probe, 
 .remove = irq_remove, 
 .driver = { 
   .name   = DRIVER_NAME, 
   .owner  = THIS_MODULE, 
 }, 
}; 
 
static void irq_release(struct device *dev) 
{ return;}

static struct platform_device irq_device = { 
 .name = DRIVER_NAME, 
 .id   = -1, 
 .dev = { .release = irq_release, }, 
}; 

static int __init irq_drv_init(void) 
{
	int ret;
 	volatile unsigned int __iomem	*rSICRL, *rSPRIDR;//, *tem
	u32 rVal;
 	DBG(KERN_INFO DRIVER_NAME ": Freescale IRQ Test Program driver init\r\n"); 
 	ret = register_chrdev(major, DRIVER_NAME, &irq_fops);
 	if (ret < 0) 
	{ 
  		DBG(KERN_INFO "register_chrdev: can't get major number\n"); 
  		return ret; 
 	} 
 	DBG(KERN_INFO DRIVER_NAME ": run here1\n"); 
 	if (major == 0)
	{ 
  		major = ret; /* dynamic */ 
 	}
/*#######choose the ieq pin function##############*/
 	rSICRL = ioremap(SICRL, 4);
	rVal = in_be32(rSICRL);
	rVal &= 0xFCFFFFFF;
	out_be32(rSICRL, rVal);
	iounmap(rSICRL);
 	DBG(KERN_INFO DRIVER_NAME ": run here2\n"); 

 	rSPRIDR = ioremap(SPRIDR, 4);/*read the cpu id*/
	rVal = in_be32(rSPRIDR);
 	DBG(KERN_INFO DRIVER_NAME ":SPRIDR:0x%x\n", rVal); 
	iounmap(rSPRIDR);

	ipicreg= ioremap(IPIC_BASE, 0x4F);
 	DBG(KERN_INFO DRIVER_NAME ":ipicreg:0x%x\n",ipicreg); 
 	DBG(KERN_INFO DRIVER_NAME ": run here2.1\n"); 
	
/*################################################*/
// 	DBG(KERN_INFO DRIVER_NAME ": run here3\n"); 

	irq_class = class_create(THIS_MODULE, "class_irq"); 
 	if (IS_ERR(irq_class)) 
	{ 
  		unregister_chrdev(major, DRIVER_NAME);  
  		return PTR_ERR(irq_class); 
 	} 
 	DBG(KERN_INFO DRIVER_NAME ": run here4\n"); 
 	ret = platform_device_register(&irq_device); 
 	if (ret) 
 	{  
  		DBG(KERN_INFO DRIVER_NAME ": register device failed! ret = %d\r\n",ret);  
  		unregister_chrdev(major, DRIVER_NAME);  
  		goto exit; 
 	} 
	
 	ret = platform_driver_register(&irq_driver); 
 	if(ret) 
 	{ 
  		DBG(KERN_INFO DRIVER_NAME ": register driver failed! ret = %d\r\n",ret);     
  		platform_device_unregister(&irq_device);
  		unregister_chrdev(major, DRIVER_NAME);
  		goto exit;  
 	}
exit: 
 return ret; 
} 

static void __exit irq_drv_exit(void) 
{ 
	DBG(KERN_INFO DRIVER_NAME   ": Freescale IRQ Program driver exit\r\n"); 
 	platform_driver_unregister(&irq_driver); 
 	platform_device_unregister(&irq_device); 
 	class_destroy(irq_class);
 	iounmap(ipicreg);
 	unregister_chrdev(major, DRIVER_NAME); 
} 

module_init(irq_drv_init); 
module_exit(irq_drv_exit); 
 
MODULE_AUTHOR("huangjunlin@bj.xinwei.com.cn"); 
MODULE_DESCRIPTION("Freescale 8308 IRQ Program"); 
MODULE_LICENSE("GPL");

