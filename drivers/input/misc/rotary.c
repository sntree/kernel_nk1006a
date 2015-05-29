#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include <linux/time.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <asm-generic/bug.h>
#include <asm/mach/irq.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>



#define DEVICE_NAME "ROTARY"
#define DRV_NAME "ROTARY"

#define SABRESD_GPIO_MODE1	IMX_GPIO_NR(6, 31) //EIM_BCLK 
#define SABRESD_GPIO_MODE2	IMX_GPIO_NR(2, 28) //EIM_EB0
#define SABRESD_GPIO_MODE3	IMX_GPIO_NR(3, 0) //EIM_DA0
#define SABRESD_GPIO_MODE4	IMX_GPIO_NR(5, 20) //CSI0_DATA_EN

static int rotary_major = 0;
static int irq_mode1 = 0;
static int irq_mode2 = 0;
static int irq_mode3 = 0;
static int irq_mode4 = 0;

struct rotary_dev_t
{
 struct input_dev* idev;
} rotary_dev;


struct work_struct rotary_work;

void rotary_do_workqueue(unsigned long data)
{
	int val = 0;
	//udelay(200);
	msleep(200);
 	
	val = gpio_get_value(SABRESD_GPIO_MODE1);
	
	val = ((val<<1) | gpio_get_value(SABRESD_GPIO_MODE2));
	
	val = ((val<<1) | gpio_get_value(SABRESD_GPIO_MODE3));
	
	val = ((val<<1) | gpio_get_value(SABRESD_GPIO_MODE4));

	printk(DEVICE_NAME "rotary_do_workqueue val %x!!\n", val);
	
	switch(val)
	{
		case 0x7:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE1111111111!!\n");
			input_event(rotary_dev.idev, EV_KEY, KEY_2,1);
			input_sync(rotary_dev.idev);
			udelay(200);
			input_event(rotary_dev.idev, EV_KEY, KEY_2,0);
			input_sync(rotary_dev.idev);
			break;
		case 0xb:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE2222222222!!\n");
			input_event(rotary_dev.idev, EV_KEY, KEY_3,1);
			input_sync(rotary_dev.idev);
			udelay(200);	
			input_event(rotary_dev.idev, EV_KEY, KEY_3,0);
			input_sync(rotary_dev.idev);
			break;
		case 0x3:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE3333333333!!\n");
			input_event(rotary_dev.idev, EV_KEY, KEY_4,1);
			input_sync(rotary_dev.idev);
			udelay(200);	
			input_event(rotary_dev.idev, EV_KEY, KEY_4,0);
			input_sync(rotary_dev.idev);
			break;
		case 0xf:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE44444444444!!\n");
			input_event(rotary_dev.idev, EV_KEY, KEY_1,1);
			input_sync(rotary_dev.idev);
			udelay(200);	
			input_event(rotary_dev.idev, EV_KEY, KEY_1,0);
			input_sync(rotary_dev.idev);
			break;
		case 0xd:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE55555555555!!\n");
			input_event(rotary_dev.idev, EV_KEY, KEY_5,1);
			input_sync(rotary_dev.idev);
			udelay(200);	
			input_event(rotary_dev.idev, EV_KEY, KEY_5,0);
			input_sync(rotary_dev.idev);
			break;
	}
	enable_irq(irq_mode1);
	enable_irq(irq_mode2);
	enable_irq(irq_mode3);
	enable_irq(irq_mode4);
}
DECLARE_WORK(rotary_work,rotary_do_workqueue);
/*
struct tasklet_struct rotary_tasklet;

void rotary_do_tasklet(unsigned long data)
{
	int val = 0;
	udelay(200);
 	//printk( "rotary_IntHandler:irq=%d \n",irq);
	val = gpio_get_value(SABRESD_GPIO_MODE1);
	//printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE1 val %x!!\n", val);
	val = ((val<<1) | gpio_get_value(SABRESD_GPIO_MODE2));
	//printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE2 val %x!!\n", val);
	val = ((val<<1) | gpio_get_value(SABRESD_GPIO_MODE3));
	//printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE3 val %x!!\n", val);
	val = ((val<<1) | gpio_get_value(SABRESD_GPIO_MODE4));
	//printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE4 val %x!!\n", val);
	switch(val)
	{
		case 0x7:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE1111111111!!\n");
			break;
		case 0xb:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE2222222222!!\n");
			break;
		case 0x3:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE3333333333!!\n");
			break;
		case 0xf:
			printk(DEVICE_NAME "gpio_get_value SABRESD_GPIO_MODE44444444444!!\n");
			break;
	}

}

DECLARE_TASKLET(rotary_tasklet,rotary_do_tasklet,0);
*/
static irqreturn_t rotary_IntHandler(int irq, void *dev_id)

{
	//tasklet_schedule(&rotary_tasklet);
	disable_irq_nosync(irq_mode1);
	disable_irq_nosync(irq_mode2);
	disable_irq_nosync(irq_mode3);
	disable_irq_nosync(irq_mode4);
	schedule_work(&rotary_work);
    return IRQ_HANDLED;

}



static int register_gpio(void)
{
	int ret = 0;
	ret = gpio_request(SABRESD_GPIO_MODE1, "GPIO_MODE1");
	if (ret) {
		printk(DEVICE_NAME "unable to request GPIO SABRESD_GPIO_MODE1 \n");
		return ret;
	}

	ret = gpio_request(SABRESD_GPIO_MODE2, "GPIO_MODE2");
 	if (ret) {
		printk(DEVICE_NAME "unable to request GPIO SABRESD_GPIO_MODE2 \n");
		goto exit_free_gpio_1;
	}

	ret = gpio_request(SABRESD_GPIO_MODE3, "GPIO_MODE3");
 	if (ret) {
		printk(DEVICE_NAME "unable to request GPIO SABRESD_GPIO_MODE3 \n");
		goto exit_free_gpio_2;
	}

	ret = gpio_request(SABRESD_GPIO_MODE4, "GPIO_MODE4");
 	if (ret) {
		printk(DEVICE_NAME "unable to request GPIO SABRESD_GPIO_MODE4 \n");
		goto exit_free_gpio_3;
	}	
	return 0;

exit_free_gpio_3 :
	gpio_free(SABRESD_GPIO_MODE3);
exit_free_gpio_2 :
	gpio_free(SABRESD_GPIO_MODE2);
exit_free_gpio_1 :
	gpio_free(SABRESD_GPIO_MODE1);
	return ret;
}


static int __devinit rotary_probe(struct platform_device *pdev)

{
	int ret;
	unsigned long irqflags;
	struct input_dev *input;

	input = input_allocate_device();
	if (!input) {
		return -ENOMEM;
	} 
	rotary_dev.idev = input;
	ret = register_gpio();
	if(ret) {
		return -ENOENT;
	}
	
	gpio_direction_input(SABRESD_GPIO_MODE1);
 	gpio_direction_input(SABRESD_GPIO_MODE2);
	gpio_direction_input(SABRESD_GPIO_MODE3);
 	gpio_direction_input(SABRESD_GPIO_MODE4);

//==========================
	gpio_export(SABRESD_GPIO_MODE1,0);
	gpio_export(SABRESD_GPIO_MODE2,0);
    gpio_export(SABRESD_GPIO_MODE3,0);
    gpio_export(SABRESD_GPIO_MODE4,0);
//==========================
	irq_mode1 = gpio_to_irq (SABRESD_GPIO_MODE1);
 	printk(DEVICE_NAME "SABRESD_GPIO_MODE1 gpio_to_irq return %d\n", irq_mode1);
	
	irq_mode2 = gpio_to_irq (SABRESD_GPIO_MODE2);
 	printk(DEVICE_NAME "SABRESD_GPIO_MODE2 gpio_to_irq return %d\n", irq_mode2);

	irq_mode3 = gpio_to_irq (SABRESD_GPIO_MODE3);
 	printk(DEVICE_NAME "SABRESD_GPIO_MODE3 gpio_to_irq return %d\n", irq_mode3);

	irq_mode4 = gpio_to_irq (SABRESD_GPIO_MODE4);
 	printk(DEVICE_NAME "SABRESD_GPIO_MODE4 gpio_to_irq return %d\n", irq_mode4);

	irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;// | IRQF_SHARED;
	
	ret = request_irq(irq_mode1, rotary_IntHandler,irqflags, "irq_mode1", NULL);
	if (ret) {
		printk(DEVICE_NAME "SABRESD_GPIO_MODE1: couldn't get irq ret = %d\n",ret);
		goto exit_free_input;
	}

	ret = request_irq(irq_mode2, rotary_IntHandler,irqflags, "irq_mode2", NULL);
	if (ret) {
		printk(DEVICE_NAME "SABRESD_GPIO_MODE2: couldn't get irq ret = %d\n",ret);
		goto exit_free_irq_1;
	}

	ret = request_irq(irq_mode3, rotary_IntHandler,irqflags, "irq_mode3", NULL);
	if (ret) {
		printk(DEVICE_NAME "SABRESD_GPIO_MODE3: couldn't get irq ret = %d\n",ret);
		goto exit_free_irq_2;
	}

	ret = request_irq(irq_mode4, rotary_IntHandler,irqflags, "irq_mode4", NULL);
	if (ret) {
		printk(DEVICE_NAME "SABRESD_GPIO_MODE4: couldn't get irq ret = %d\n",ret);
		goto exit_free_irq_3;
	}

	input->name = DEVICE_NAME;
	input->id.bustype = BUS_HOST;
	input->dev.parent = &pdev->dev;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	__set_bit(EV_KEY, input->evbit);
   	__set_bit(KEY_1, input->keybit);
    __set_bit(KEY_2, input->keybit);
    __set_bit(KEY_3, input->keybit);
    __set_bit(KEY_4, input->keybit);
	__set_bit(KEY_5, input->keybit);
    //input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	//input_set_capability(input, EV_KEY, button->code);
	ret = input_register_device(input);
    if( ret )
    {
        printk("dhole2440_keys_probe input_register_device error!\n");
		goto exit_free_irq_4;
    }
	return 0;
	
exit_free_irq_4 :
	free_irq(irq_mode4, NULL);
exit_free_irq_3 :
	free_irq(irq_mode3, NULL);
exit_free_irq_2 :
	free_irq(irq_mode2, NULL);
exit_free_irq_1 :
	free_irq(irq_mode1, NULL);
exit_free_input :
	input_free_device(input);
	return ret;



}
static int __devexit rotary_remove(struct platform_device *pdev)
{
	free_irq(irq_mode1, NULL);
	free_irq(irq_mode2, NULL);
	free_irq(irq_mode3, NULL);
	free_irq(irq_mode4, NULL);
	gpio_free(SABRESD_GPIO_MODE1);
	gpio_free(SABRESD_GPIO_MODE2);
	gpio_free(SABRESD_GPIO_MODE3);
	gpio_free(SABRESD_GPIO_MODE4);
	input_unregister_device(rotary_dev.idev);

	return 0;
}


static struct platform_driver rotary_driver = {
	.probe		= rotary_probe,
	.remove		= __devexit_p(rotary_remove),
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	}
};


static int __init rotary_init(void)
{
  return platform_driver_register(&rotary_driver); 
}

static void __exit rotary_exit(void)
{
	platform_driver_unregister(&rotary_driver);
}

MODULE_ALIAS("platform:" DRV_NAME);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuhan Guide Infrared Co., Ltd");
module_init(rotary_init);
module_exit(rotary_exit);

