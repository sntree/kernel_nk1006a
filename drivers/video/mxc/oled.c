/* 
	For OLED 1280*960 Driver
 */

#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include "oled.h"


#define OLED_I2C_NAME                "OLED-I2C"

#define OLED_5V_POWER    IMX_GPIO_NR(1, 18)

struct i2c_client * oled_i2c_connect_client = NULL; 

static void OLED_5V_power_switch(unsigned char enable)
{
	if(enable)
	{
		printk("OLED_5V_power enable \n");
		gpio_direction_output(OLED_5V_POWER, 1);
	}
	else
	{
		printk("OLED_5V_power disable \n");
		gpio_direction_output(OLED_5V_POWER, 0);	
	}
}

/*******************************************************
Function:
    Read data from the i2c slave device.
Input:
    client:     i2c device.
    buf[0~1]:   read start address.
    buf[2~len-1]:   read data buffer.
    len:    GTP_ADDR_LENGTH + read bytes count
Output:
    numbers of i2c_msgs to transfer: 
      2: succeed, otherwise: failed
*********************************************************/
s32 oled_i2c_read(struct i2c_client *client, u8 *buf, s32 len)
{
    struct i2c_msg msgs[2];
    s32 ret=-1;
    s32 retries = 0;

    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = client->addr;
    msgs[0].len   = 1;
    msgs[0].buf   = &buf[0];
    //msgs[0].scl_rate = 300 * 1000;    // for Rockchip, etc.
    
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = client->addr;
    msgs[1].len   = len - 2;
    msgs[1].buf   = &buf[2];
    //msgs[1].scl_rate = 300 * 1000;

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, msgs, 2);
        if(ret == 2)break;
        retries++;
    }
    if((retries >= 5))
    {
 		printk("oled i2c read failed\n");
    }
    return ret;
}



/*******************************************************
Function:
    Write data to the i2c slave device.
Input:
    client:     i2c device.
    buf[0~1]:   write start address.
    buf[2~len-1]:   data buffer
    len:    GTP_ADDR_LENGTH + write bytes count
Output:
    numbers of i2c_msgs to transfer: 
        1: succeed, otherwise: failed
*********************************************************/
s32 oled_i2c_write(struct i2c_client *client,u8 *buf,s32 len)
{
    struct i2c_msg msg;
    s32 ret = -1;
    s32 retries = 0;

    msg.flags = !I2C_M_RD;
    msg.addr  = client->addr;
    msg.len   = len;
    msg.buf   = buf;
    //msg.scl_rate = 300 * 1000;    // for Rockchip, etc

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret == 1)break;
        retries++;
    }
    if((retries >= 5))
    {
		printk("oled i2c write failed\n");    
    }
    return ret;
}




/*******************************************************
Function:
    Enter sleep mode.
Input:
    ts: private data.
Output:
    Executive outcomes.
       1: succeed, otherwise failed.
*******************************************************/
static s8 oled_enter_sleep(void)
{
	return 0;
}

/*******************************************************
Function:
    Wakeup from sleep.
Input:
    ts: private data.
Output:
    Executive outcomes.
        >0: succeed, otherwise: failed.
*******************************************************/
static s8 oled_wakeup_sleep(void)
{
	return 0; 
}



static s32 oled_i2c_init(struct i2c_client *client)
{

	s8 ret = -1;
	u8 i2c_control_buf1[2] = {0xF0, 0X45};
	u8 i2c_control_buf2[2] = {0x9D, 0X0F};
	u8 i2c_control_buf3[2] = {0xF0, 0X01};
	u8 i2c_control_buf4[2] = {0x55, 0X11};

	u8 oled_bright[2] = {0x50, 0X3F};
	u8 oled_gamma[2] = {0x54, 0X09};
	

	u8 vscale_step_low[2] = {0x07, 0Xab};
	u8 vscale_step_high[2] = {0x06, 0X02};
	u8 hscale_step_low[2] = {0x0a, 0X00};
	u8 hscale_step_high[2] = {0x09, 0X02};
	//u8 hv_flip[2] = {0xd0, 0X03};
	/*u8 vscale_step_low[2] = {0x07, 0X00};
	u8 vscale_step_high[2] = {0x06, 0X04};
	u8 hscale_step_low[2] = {0x0a, 0X00};
	u8 hscale_step_high[2] = {0x09, 0X04};*/
	

	printk("oled_i2c_init.............................\n");
	
	ret = oled_i2c_write(client, i2c_control_buf1, 2);
	if(ret < 0)
	{
		printk("failed to set olde register 1\n");
	}

	ret = oled_i2c_write(client, i2c_control_buf2, 2);
	if(ret < 0)
	{
		printk("failed to set olde register 2\n");
	}

	ret = oled_i2c_write(client, i2c_control_buf3, 2);
	if(ret < 0)
	{
		printk("failed to set olde register 3\n");
	}
	
	ret = oled_i2c_write(client, i2c_control_buf4, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register 4\n");
	}

	msleep(10);
	
	ret = oled_i2c_write(client, oled_bright, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register oled_bright\n");
	}

	ret = oled_i2c_write(client, oled_gamma, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register oled_gamma\n");
	}
	

	ret = oled_i2c_write(client, vscale_step_low, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register vscale_step_low\n");
	}
	ret = oled_i2c_write(client, vscale_step_high, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register vscale_step_high\n");
	}

	ret = oled_i2c_write(client, hscale_step_low, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register hscale_step_low\n");
	}


	ret = oled_i2c_write(client, hscale_step_high, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register hscale_step_high\n");
	}

	/*ret = oled_i2c_write(client, hv_flip, 2);	
	if(ret < 0)
	{
		printk("failed to set olde register hv_flip\n");
	}*/

	msleep(10);

    return ret;
}

/*******************************************************
Function:
    I2c probe.
Input:
    client: i2c device struct.
    id: device id.
Output:
    Executive outcomes. 
        0: succeed.
*******************************************************/
static int OLED_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    s32 ret = -1;
    u16 version_info;
    
    
    //do NOT remove these logs
    printk("OLED Driver Version: %s\n", "V1.0");
    printk("OLED Driver Built@%s, %s\n", __TIME__, __DATE__);
    printk("OLED I2C Address: 0x%02x\n", client->addr);

    oled_i2c_connect_client = client;
    
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
    {
        printk("I2C check functionality failed.\n");
        return -ENODEV;
    }
    
    ret = oled_i2c_init(client);
    if (ret < 0)
    {
        printk("oled init failed.");
    }
    
    return 0;
}


static int OLED_remove(struct i2c_client *client)
{


    return 0;
}


static const struct i2c_device_id oled_id[] = {
    { OLED_I2C_NAME, 0 },
    { }
};

static struct i2c_driver oled_driver = {
    .probe      = OLED_probe,
    .remove     = OLED_remove,
    .id_table   = oled_id,
    .driver = {
        .name     = OLED_I2C_NAME,
        .owner    = THIS_MODULE,
    },
};

/*******************************************************    
Function:
    Driver Install function.
Input:
    None.
Output:
    Executive Outcomes. 0---succeed.
********************************************************/
static int __devinit oled_init(void)
{
    s32 ret;

    printk("OLED driver installing...\n");

    ret = i2c_add_driver(&oled_driver);
    return ret; 
}

/*******************************************************    
Function:
    Driver uninstall function.
Input:
    None.
Output:
    Executive Outcomes. 0---succeed.
********************************************************/
static void __exit oled_exit(void)
{
    printk("oled driver exited.\n");
    i2c_del_driver(&oled_driver);

}

late_initcall(oled_init);
module_exit(oled_exit);

MODULE_AUTHOR("Captain");
MODULE_DESCRIPTION("OLED I2C Driver");
MODULE_LICENSE("GPL");
