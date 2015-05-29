/*
 * AMD CS5535/CS5536 GPIO driver
 * Copyright (C) 2006  Advanced Micro Devices, Inc.
 * Copyright (C) 2007-2009  Andres Salomon <dilinger@collabora.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>


#include <linux/init.h>


#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>



#define OUT_PUT 1
#define IN_PUT 0

#define DEBUG_NK 1
#if DEBUG_NK
#define LOG_NK(str) 	\
 {\
 	printk("****************%s****************\n",str);	\
 }
#else
#define LOG_NK(str) 	\
 {\
  \
 }
#endif

struct nk_gpio
{
    const char* name;
    int gpio;
    int direction;
    int value;
    int defualt;
};
struct nk_gpio_platform_data
{
    const struct nk_gpio* gpios;
    int num_gpios;
};


struct nk_gpio *NK_GPIOs;
static int NK_GPIO_NUM = 0;

static int __devinit nk_gpio_probe(struct platform_device *pdev)
{
	struct nk_gpio_platform_data *pdata = pdev->dev.platform_data;
	int i=0;

	LOG_NK("probe nk-gpio");

	NK_GPIO_NUM = pdata->num_gpios;
	if(pdata && pdata->num_gpios)
	{
		NK_GPIOs = (struct nk_gpio *)kzalloc((sizeof(struct nk_gpio))*(NK_GPIO_NUM),GFP_KERNEL);
		if(!NK_GPIOs)
		{
			return -ENOMEM;
		}
		
		for(i=0;i<NK_GPIO_NUM;i++)
		{
			NK_GPIOs[i].name = (pdata->gpios)[i].name;
			NK_GPIOs[i].gpio = (pdata->gpios)[i].gpio;
			NK_GPIOs[i].direction = (pdata->gpios)[i].direction;
			NK_GPIOs[i].value = (pdata->gpios)[i].value;
			NK_GPIOs[i].defualt = (pdata->gpios)[i].defualt;

			gpio_request(NK_GPIOs[i].gpio,NK_GPIOs[i].name);
			if(NK_GPIOs[i].direction == OUT_PUT)
			{
				gpio_direction_output(NK_GPIOs[i].gpio, NK_GPIOs[i].value);
			}
			else
			{
				gpio_direction_input(NK_GPIOs[i].gpio);
			}
			gpio_export(NK_GPIOs[i].gpio,0);

			printk("nk-gpio name:%s  gpio:%d  direction:%d  value:%d\n",
				NK_GPIOs[i].name,
				NK_GPIOs[i].gpio,
				NK_GPIOs[i].direction,
				NK_GPIOs[i].value);
		}
		return 0;
	}
	return -1;
}

static int __exit nk_gpio_remove(struct platform_device *pdev)
{
	int i;
	LOG_NK("remove nk-gpio");
	for(i=0; i<NK_GPIO_NUM; i++)
	{
		if(NK_GPIOs[i].gpio)
		{
			gpio_free(NK_GPIOs[i].gpio);
		}
	}
	return 0;
}

static struct platform_driver nk_gpio_driver =
{
	.driver = {
		.name = "nk-gpio",
	},
	.remove = nk_gpio_remove,
};

static int __init nk_gpio_init(void)
{
	LOG_NK("init nk-gpio");

	return platform_driver_probe(&nk_gpio_driver, nk_gpio_probe);
}

static void __exit nk_gpio_exit(void)
{
	platform_driver_unregister(&nk_gpio_driver);
}


module_init(nk_gpio_init);
module_exit(nk_gpio_exit);
MODULE_AUTHOR("blus");
MODULE_DESCRIPTION("Just for fun");
MODULE_LICENSE("GPL");

