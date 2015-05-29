
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/input-polldev.h>

#define	DRIVER_NAME "tmp101"

#define POLL_TIME 200
#define POLL_TIME_MIN 0
#define POLL_TIME_MAX 500


struct tmp101 {
	struct i2c_client *client;
	//struct device *hwmon_dev;
	struct input_polled_dev *poll_dev;
	struct mutex lock;
	u16 config_orig;
	unsigned long last_update;
	int temp[3];
	int enable;
};

static inline int tmp101_set_reg(struct tmp101 *tmp101)
{
	int ret;
	unsigned char tmp[2];
	tmp[0] = 1;
	tmp[1] = ((3 << 5) & 0x60);;

	ret = i2c_master_send(tmp101->client, tmp,2);
	if(ret < 0)
	{
		printk("tmp101_set_reg send failed\n");
		return 1;
	}
/*
	tmp[1] = 0;
	ret = i2c_master_recv(tmp101->client, tmp,1);
	if(ret < 0)
	{
		printk("tmp101_set_reg send failed\n");
		return 1;
	}
	printk("tmp101_set_reg tmp[0] = %d, tmp[1] = %d, ret = %d!\n", tmp[0], tmp[1], ret);
*/
	return 0;

}

static inline int tmp101_read_tmp(struct tmp101 *tmp101)
{
	int ret;
	unsigned char tmp[2];
	tmp[0] = 0;
	tmp[1] = 0;

	ret = i2c_master_send(tmp101->client, tmp, 1);


	ret = i2c_master_recv(tmp101->client, tmp, 2);
	

	tmp101->temp[0] = tmp[0];
	tmp101->temp[1] = tmp[1];
	return 0;
}

static void tmp101_report_data(struct tmp101 *pdata)
{
	int value;
	struct input_polled_dev *poll_dev = pdata->poll_dev;
	mutex_lock(&pdata->lock);
	tmp101_read_tmp(pdata);
	mutex_unlock(&pdata->lock);
	value = pdata->temp[0];
	value = ((value << 4) | (pdata->temp[1] >> 4)) * 625;
	input_report_abs(poll_dev->input, ABS_MISC, value);
	input_sync(poll_dev->input);
}
static void tmp101_dev_poll(struct input_polled_dev *dev)
{
	struct tmp101 *tmp101 = (struct tmp101 *)dev->private;

	tmp101_report_data(tmp101);

}
static struct tmp101 *tmp101_update_device(struct i2c_client *client)
{
	struct tmp101 *tmp101 = i2c_get_clientdata(client);

	mutex_lock(&tmp101->lock);
	tmp101_read_tmp(tmp101);
	mutex_unlock(&tmp101->lock);
	return tmp101;
}

static ssize_t tmp101_show_temp(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int value;
	struct input_polled_dev *poll_dev = dev_get_drvdata(dev);
	struct tmp101 *tmp101 = poll_dev->private;
	tmp101 = tmp101_update_device(tmp101->client);
	
	value = tmp101->temp[0];
	value = ((value << 4) | (tmp101->temp[1] >> 4)) * 625;
	return sprintf(buf, "temp0  %d, temp1  %d, value  %d\n", tmp101->temp[0], tmp101->temp[1], value);
//	return sprintf(buf, "%d", value);
}

static ssize_t tmp101_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct input_polled_dev *poll_dev = dev_get_drvdata(dev);
	struct tmp101 *tmp101 = poll_dev->private;
	return sprintf(buf, "%d\n", tmp101->enable);
}

static ssize_t tmp101_enable_store(struct device *dev,
				struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long enable;
	struct input_polled_dev *poll_dev = dev_get_drvdata(dev);
	struct tmp101 *tmp101 = poll_dev->private;
	
	enable = simple_strtoul(buf, NULL, 10);
	mutex_lock(&tmp101->lock);
	tmp101->enable = enable;
	mutex_unlock(&tmp101->lock);

	return count;
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, tmp101_show_temp, NULL , 0);
static SENSOR_DEVICE_ATTR(enable, S_IWUSR | S_IRUGO, tmp101_enable_show, tmp101_enable_store, 0);


static struct attribute *tmp101_attributes[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_enable.dev_attr.attr,
	NULL
};

static const struct attribute_group tmp101_attr_group = {
	.attrs = tmp101_attributes,
};

//#define TMP102_CONFIG  (TMP102_CONF_TM | TMP102_CONF_EM | TMP102_CONF_CR1)
//#define TMP102_CONFIG_RD_ONLY (TMP102_CONF_R0 | TMP102_CONF_R1 | TMP102_CONF_AL)

static int __devinit tmp101_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct tmp101 *tmp101;
	struct input_dev *idev;
	struct input_polled_dev *poll_dev;
	int status;
	printk("tmp101_probe!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d\n", client->addr);

	tmp101 = kzalloc(sizeof(*tmp101), GFP_KERNEL);
	if (!tmp101) {
		dev_dbg(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}

	tmp101->client = client;
	tmp101->enable = 0;
	
	i2c_set_clientdata(client, tmp101);

	if(tmp101_set_reg(tmp101) != 0)
	{
		goto fail_free;
	}

	poll_dev = input_allocate_polled_device();
	if(!poll_dev) {
		status = -ENOMEM;
		goto fail_free;
	}
	poll_dev->poll = tmp101_dev_poll;
	poll_dev->poll_interval = POLL_TIME;
	poll_dev->poll_interval_max = POLL_TIME_MAX;
	poll_dev->poll_interval_min = POLL_TIME_MIN;
	poll_dev->private = tmp101;
	
	idev = poll_dev->input;
	if(client->addr == 0x49){
			idev->name = "tmp101_1";
	}else if(client->addr == 0x4a){
			idev->name = "tmp101_2";
	}else{
			idev->name = "tmp101_3";
	}
	idev->id.bustype = BUS_I2C;
	idev->evbit[0] = BIT_MASK(EV_ABS);
	idev->absbit[BIT_WORD(ABS_MISC)] |= BIT_MASK(ABS_MISC);
	input_set_abs_params(idev, ABS_MISC, 0, 65535, 0, 0);

	tmp101->poll_dev = poll_dev;
	status = input_register_polled_device(poll_dev);
	if(status) {
		goto fail_register_polled_device;
	}
	

	status = sysfs_create_group(&idev->dev.kobj, &tmp101_attr_group);
	if (status) {
		dev_dbg(&client->dev, "could not create sysfs files\n");
		goto fail_remove_sysfs;
	}
	
	mutex_init(&tmp101->lock);

	/*tmp101->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(tmp101->hwmon_dev)) {
		dev_dbg(&client->dev, "unable to register hwmon device\n");
		status = PTR_ERR(tmp101->hwmon_dev);
		goto fail_remove_sysfs;
	}*/

	dev_info(&client->dev, "initialized\n");

	return 0;

fail_remove_sysfs:
	input_unregister_polled_device(poll_dev);
fail_register_polled_device:
	input_free_polled_device(poll_dev);
fail_free:
	kfree(tmp101);

	return status;
}

static int __devexit tmp101_remove(struct i2c_client *client)
{
	struct tmp101 *tmp101 = i2c_get_clientdata(client);

	//hwmon_device_unregister(tmp101->hwmon_dev);

	sysfs_remove_group(&client->dev.kobj, &tmp101_attr_group);
	input_unregister_polled_device(tmp101->poll_dev);
	input_free_polled_device(tmp101->poll_dev);
	kfree(tmp101);

	return 0;
}

#if 0
static int tmp102_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	int config;

	config = tmp102_read_reg(client, TMP102_CONF_REG);
	if (config < 0)
		return config;

	config |= TMP102_CONF_SD;
	return tmp102_write_reg(client, TMP102_CONF_REG, config);
}

static int tmp102_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	int config;

	config = tmp102_read_reg(client, TMP102_CONF_REG);
	if (config < 0)
		return config;

	config &= ~TMP102_CONF_SD;
	return tmp102_write_reg(client, TMP102_CONF_REG, config);
}

static const struct dev_pm_ops tmp101_dev_pm_ops = {
	.suspend	= tmp102_suspend,
	.resume		= tmp102_resume,
};

#define TMP101_DEV_PM_OPS (&tmp101_dev_pm_ops)
#else
#define	TMP101_DEV_PM_OPS NULL
#endif /* CONFIG_PM */

static const struct i2c_device_id tmp101_id[] = {
	{ "tmp101", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tmp101_id);

static struct i2c_driver tmp101_driver = {
	.driver.name	= DRIVER_NAME,
	.driver.pm	= TMP101_DEV_PM_OPS,
	.probe		= tmp101_probe,
	.remove		= __devexit_p(tmp101_remove),
	.id_table	= tmp101_id,
};

static int __init tmp101_init(void)
{
	return i2c_add_driver(&tmp101_driver);
}
module_init(tmp101_init);

static void __exit tmp101_exit(void)
{
	i2c_del_driver(&tmp101_driver);
}
module_exit(tmp101_exit);

MODULE_AUTHOR("guide infrared");
MODULE_DESCRIPTION("Texas Instruments TMP101 temperature sensor driver");
MODULE_LICENSE("GPL");


