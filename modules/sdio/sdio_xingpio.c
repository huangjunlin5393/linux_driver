/*
 * linux/drivers/mmc/card/sdio_innogpio.c
 *
 * Author:	caihaijun
 * Created:	Oct 17, 2016
 * Copyright:	Innofidei, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/platform_device.h>   
#include <linux/delay.h>  
//#include <linux/gpio.h>
#include <linux/err.h>  
#include <linux/irq.h>  
#include <linux/interrupt.h>  
#include <linux/slab.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <mt-plat/mt_gpio.h>
#include <mt-plat/mt_gpio_core.h>
#endif
#include <linux/mmc/sdio_func.h>
static unsigned int inno_irq = 0;

static const struct of_device_id inno_of_match[] = {
	{ .compatible = "mediatek,gpio_innolte", },
	{},
};

//static sdio_irq_handler_t *lte_sdio_eirq_handler=NULL;
//static void *lte_sdio_eirq_data;
static unsigned int inno_aprdy=0;
//static pm_callback_t lte_sdio_pm_cb;
//static void *lte_sdio_pm_data;
typedef void (*pm_callback_t)(pm_message_t state, void *data);
void lte_sdio_register_pm(pm_callback_t pm_cb, void *data)
{
	printk("lte_sdio_register_pm (0x%p, 0x%p)\n", pm_cb, data);
	/* register pm change callback */
	//lte_sdio_pm_cb = pm_cb;
	//lte_sdio_pm_data = data;
}
EXPORT_SYMBOL(lte_sdio_register_pm);
//void lte_sdio_request_eirq(sdio_irq_handler_t irq_handler, void *data)
//{
//	printk("[lte] request interrupt from %ps\n",__builtin_return_address(0));
	
	//ret = request_irq(lte_sdio_eirq_num, lte_sdio_eirq_handler_stub, IRQF_TRIGGER_LOW, "lte_CCCI", NULL);
	//disable_irq(lte_sdio_eirq_num);
	
//	lte_sdio_eirq_handler = irq_handler;
//	lte_sdio_eirq_data    = data;
//}
//EXPORT_SYMBOL(lte_sdio_request_eirq);

static atomic_t irq_enable_flag;
void lte_sdio_enable_eirq(void)
{
	//printk("[lte] interrupt enable \n");
	//	enable_irq(inno_irq);
	if (atomic_read(&irq_enable_flag))
	{
		printk("[lte] irq has been enabled \n");
	}
	else
	{
		atomic_set(&irq_enable_flag, 1);
		enable_irq(inno_irq);
		//printk("[lte] interrupt enable \n");
		
	}
}
EXPORT_SYMBOL(lte_sdio_enable_eirq);
void lte_sdio_disable_eirq(void)
{
	//printk("[lte] interrupt disable\n", );
	//	disable_irq_nosync(inno_irq);
	if (!atomic_read(&irq_enable_flag))
	{
		printk("[lte] irq has been disabled!\n");
	}
	else
	{
		//printk("[lte] interrupt disable\n", );
		disable_irq_nosync(inno_irq);
		atomic_set(&irq_enable_flag, 0);
	}
}
EXPORT_SYMBOL(lte_sdio_disable_eirq);
typedef int (*gpio_irq_callback)(void);
void gpio_xinlte_apready_set_value(int value)
{
	mt_set_gpio_out_base(inno_aprdy,value);
}
EXPORT_SYMBOL(gpio_xinlte_apready_set_value);
gpio_irq_callback xin_gpio_irq_callbackHandler = NULL;

void gpio_xinlte_register_callback(gpio_irq_callback callback)
{
	xin_gpio_irq_callbackHandler = callback;
}
EXPORT_SYMBOL(gpio_xinlte_register_callback);



struct work_struct worker;
struct workqueue_struct *wq;
static irqreturn_t gpio_xinlte_isr(int irq, void *data)  
{  
	printk("%s\n", __FUNCTION__);
	//if (lte_sdio_eirq_handler)
	//lte_sdio_eirq_handler(lte_sdio_eirq_data);

	queue_work(wq, &worker);
	
	return IRQ_HANDLED;  
}  
static int inno_get_dtspin_info(struct platform_device *pdev)
{

	//struct gpio_drvdata *drvdata = platform_get_drvdata(pdev);
	struct pinctrl_state *pinsinno_default;
	//struct pinctrl_state *pinsrfid_default;
	int ret;
	struct device_node *irq_node=NULL;
	struct pinctrl *pinctrlinno;
	u32 ints[2] = {0,0};
	printk("inno_get_dtspin_info enter\n");
	if(pdev == NULL)
	{
		printk("inno_get_dtspin_info pdev = NULL !\n");
		return 0;
	}

	pinctrlinno = devm_pinctrl_get(&pdev->dev);
	if(pinctrlinno == NULL)
	{
		printk("ptt_get_dtspin_info => devm_pinctrl_get = NULL \n");
		return 0;
	}
	if (IS_ERR(pinctrlinno)) {
		ret = PTR_ERR(pinctrlinno);
		printk(" pinctrl1!\n");
		dev_err(&pdev->dev, "inno!\n");
		return ret;
	}

	pinsinno_default = pinctrl_lookup_state(pinctrlinno, "innodefault");
	if (IS_ERR(pinsinno_default)) {
		ret = PTR_ERR(pinsinno_default);

		dev_err(&pdev->dev, " inno default %d!\n", ret);
	}
	else
	{
		pinctrl_select_state(pinctrlinno, pinsinno_default);
	}
	irq_node= of_find_matching_node(irq_node, inno_of_match);
	if (irq_node) {
		of_property_read_u32_array(irq_node, "debounce", ints, ARRAY_SIZE(ints));
		gpio_request(ints[0], "inno");
	
		gpio_set_debounce(ints[0], ints[1]);
		printk("ints[0] = %d, ints[1] = %d!!\n", ints[0], ints[1]);

		inno_irq = irq_of_parse_and_map(irq_node, 0);
		if (inno_irq < 0) {
			printk("ptt request_irq IRQ LINE NOT AVAILABLE!.");
			return -1;
		}
		printk("inno_irq : %d\n",inno_irq);
		/*gpio_direction_input(alsps_int_gpio_number);*/

		ret = request_irq(inno_irq, (irq_handler_t)gpio_xinlte_isr, IRQ_TYPE_EDGE_FALLING, "RD_REQ_NR-eint", NULL);

		if (ret > 0) {
			printk("inno_irq request_irq IRQ LINE NOT AVAILABLE!.");
			return ret;
		}
		//disable_irq_nosync(inno_irq);
		//atomic_set(&irq_enable_flag, 0);
	}else {
		printk("null irq node!!\n");
		return -EINVAL;
	}
	ret = of_property_read_u32(irq_node,"innoltegpio,apready", &inno_aprdy);

	printk("inno_irq_get_dtspin_info...%s %d inno_aprdy %d \n",__func__,__LINE__,inno_aprdy);

	return 0;
}



static void gpio_innolte_work_func(struct work_struct *ws)
{
	if (xin_gpio_irq_callbackHandler != NULL)
	{
		xin_gpio_irq_callbackHandler();
	}
	else
	{
		printk("ERROR:gpio_xinlte_isr callback is NULL\n");
	}
	//printk("gpio_xinlte_isr \n");
}

static int inno_pindrv_probe(struct platform_device *pdev)
{
	int ret;  
	wq =create_singlethread_workqueue("rd_gpio_wq");
	if (wq == NULL) {
		printk("%s: failed to create workqueue\n", __FUNCTION__);
		return -ENOMEM;
	}

	INIT_WORK(&worker, gpio_innolte_work_func);

	ret = inno_get_dtspin_info(pdev);
	if(ret)
	{
		printk("inno_get_dtspin_info err\r\n");
		return -ENOMEM;
	}
	//ret = gpio_innolte_setup(pdev);
	//if (ret) {
	//	destroy_workqueue(drvdata->wq);
	//	kfree(drvdata);
	//	return ret;
	//}


	
	return ret;  
}

static int inno_pindrv_remove(struct platform_device *pdev)
{
	destroy_workqueue(wq);
	return 0;  
}


static struct platform_driver inno_pindrv = {
	.probe = inno_pindrv_probe,
	.remove = inno_pindrv_remove,
	.driver = {
		   .name = "inno-pin",
		   .owner = THIS_MODULE,
		   .of_match_table = inno_of_match,
		   },
};






static int __init gpio_innolte_init(void)  
{  
	int ret;  

	//ret = platform_driver_register(&gpio_innolte_driver);  
	//if (ret) {
	//	return ret;
	//} 
 
    	//ret =  platform_device_register(&gpio_innolte_device);  
	//if (ret) {
	//	platform_driver_unregister(&gpio_innolte_driver);  
	//	return ret;
	//} 

	ret = platform_driver_register(&inno_pindrv);
	if (ret) {
		printk("register driver failed (%d)\n", ret);
		platform_driver_unregister(&inno_pindrv);  
		return ret;
	}

	return 0;
}

static void __exit gpio_innolte_exit(void)
{

	//platform_device_unregister(&gpio_innolte_device);
	platform_driver_unregister(&inno_pindrv);
}

module_init(gpio_innolte_init); 
module_exit(gpio_innolte_exit);

MODULE_AUTHOR("Innofidei.com.cn");
MODULE_DESCRIPTION("GPIO for innofidei LTE driver");
MODULE_LICENSE("GPL");

