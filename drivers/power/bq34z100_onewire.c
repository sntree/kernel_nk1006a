#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include <linux/time.h>
#include <linux/timer.h>


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

#define DEVICE_NAME  "BUTTONS"
 
#define SABRESD_GPIO_BATTERY IMX_GPIO_NR(7, 1)

#define BATTERY_FULL_CAPACITY   2900

static int button_major = 0;

struct timeval tv1, tv2;

struct buttons_dev_t
{
 struct cdev cdev;
} buttons_dev;


static int buttons_open(struct inode *inode, struct file *file)
{
 int ret; 
 unsigned long irqflags;
 return 0;
}

static int buttons_release(struct inode *inode, struct file *file)
{
 
 printk(DEVICE_NAME " buttons_release\n");
 return 0;
}

void set_HDQ_1(void)
{
 gpio_direction_output(SABRESD_GPIO_BATTERY, 0);
 udelay(20);
 gpio_direction_output(SABRESD_GPIO_BATTERY, 1);
 udelay(180);
}

void set_HDQ_0(void)
{
 gpio_direction_output(SABRESD_GPIO_BATTERY, 0);
 udelay(100);
 gpio_direction_output(SABRESD_GPIO_BATTERY, 1);
 udelay(100);
 
}

void set_HDQ_break(void)
{
 gpio_direction_output(SABRESD_GPIO_BATTERY, 0);
 udelay(200);
 gpio_direction_output(SABRESD_GPIO_BATTERY, 1);
 udelay(50);
}

void basic_write_HDQ_byte(u8 addr)
{
 int i;
 set_HDQ_break();

 for(i = 0; i < 8; i++)
 {
  if((addr >> i) & 0x1)
   set_HDQ_1();
 
  else
   set_HDQ_0();
 }
}

u8 read_HDQ_byte(u8 addr)
{
 u8 tmp_buf = 0;
 int	i;
 local_irq_disable();
 basic_write_HDQ_byte(addr);
 gpio_direction_input(SABRESD_GPIO_BATTERY);
 
 udelay(170);
 do_gettimeofday(&tv1);
 
 for(i = 0 ; i < 8; i++)
 {
  udelay(58);
  tmp_buf |= (gpio_get_value(SABRESD_GPIO_BATTERY) << i);
  udelay(150);
 }
 local_irq_enable();
 do_gettimeofday(&tv2);
 udelay(200);
 return tmp_buf;
}


#if 0
static ssize_t show_bat_voltage (struct device *dev,
     struct device_attribute *attr, char *buf)
{
	 int voltage;
	 u8 reg[2];
	 reg[0] = read_HDQ_byte(0x9);
	 reg[1] = read_HDQ_byte(0x8);
	 voltage = reg[0];
	 voltage = (voltage << 8) | reg[1];
	 //printk("voltage = %d\n", voltage);
	 return sprintf(buf, "voltage=%d\n ",voltage);
}

 static ssize_t store_bat_voltage (struct device *dev,
  struct device_attribute *attr, const char *buf, size_t count)
 {
	  int val;
	  val = simple_strtoul(buf, NULL, 10);
	  set_HDQ_break();
	  set_HDQ_1();
	  set_HDQ_0();
	  gpio_direction_input(SABRESD_GPIO_BATTERY);
	  val = count;
	  return val;
 }

 static DEVICE_ATTR(bat_vol, S_IWUGO|S_IRUGO,  show_bat_voltage,
  store_bat_voltage);
 
  static ssize_t show_bat_temperature(struct device *dev,
	  struct device_attribute *attr, char *buf)
 {
 	int temp;
    u8 reg[2];  
	reg[0] = read_HDQ_byte(0xd);
	reg[1] = read_HDQ_byte(0xc);
	temp = reg[0];
	temp = (temp << 8) | reg[1];
	return sprintf(buf, "temp=%d\n ",temp);
 
}

static ssize_t store_bat_temperature (struct device *dev,
 struct device_attribute *attr, const char *buf, size_t count)
{
	int val;
	val = simple_strtoul(buf, NULL, 10);
	set_HDQ_break();
	set_HDQ_1();
	set_HDQ_0();
	gpio_direction_input(SABRESD_GPIO_BATTERY);
	val = count;
	return val;

}

static DEVICE_ATTR(bat_temp, S_IWUGO|S_IRUGO, show_bat_temperature,
store_bat_temperature);

#endif

static ssize_t show_bat_remain_capacity(struct device *dev,
	 struct device_attribute *attr, char *buf)
{
    int  rem_cap;
	int  bat_per;
	u8   reg[2];	
	reg[0] = read_HDQ_byte(0x5);
	reg[1] = read_HDQ_byte(0x4);
	rem_cap = reg[0];
	rem_cap = ((rem_cap << 8) | reg[1]);
	printk("Remaining Capacity = %d\n",rem_cap);
	
	if((rem_cap >0)&&(rem_cap <= BATTERY_FULL_CAPACITY))
	{
		bat_per = ((rem_cap*10000)/BATTERY_FULL_CAPACITY)/100;
		printk("bat_per = %d %%\n", bat_per);
		return sprintf(buf, "bat_per =%d\n ",bat_per);
		
	}
	return 0;
}

 static ssize_t store_bat_remain_capacity (struct device *dev,
  struct device_attribute *attr, const char *buf, size_t count)
 {
	 int val;
	 val = simple_strtoul(buf, NULL, 10);
	 set_HDQ_break();
	 set_HDQ_1();
	 set_HDQ_0();
	 gpio_direction_input(SABRESD_GPIO_BATTERY);
	 val = count;
	 return val;
 
 }
 static DEVICE_ATTR(bat_rem_cap, S_IWUGO|S_IRUGO,show_bat_remain_capacity,
  store_bat_remain_capacity);



static struct attribute *btn_attr[] = {
 &dev_attr_bat_rem_cap.attr,
 //&dev_attr_bat_vol.attr,
 //&dev_attr_bat_temp.attr,

 NULL
};

static struct attribute_group btn_attr_grp = {
 .name = "btn_gpio",
 .attrs = btn_attr
};

static struct file_operations buttons_fops = {
 .owner = THIS_MODULE,
 .open = buttons_open,
 .release = buttons_release,
//	.read = buttons_read,
//	.write = buttons_write,
};

static int __init buttons_init(void)
{
 int ret;
 struct device* dev_temp;
 struct class *my_class;
 
 dev_t devno = MKDEV(button_major, 0);
 if(button_major)
  ret = register_chrdev_region(devno, 1, DEVICE_NAME);
 else{
  ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
  button_major = MAJOR(devno);
 }

 my_class = class_create(THIS_MODULE,"buttons_class");
 if(IS_ERR(my_class)){
  printk("Err: failed in creating class.\n");
  return -1;
 }
 
 dev_temp = device_create(my_class,NULL,devno,NULL,"buttons");

 ret = sysfs_create_group(&dev_temp->kobj, &btn_attr_grp);
 
 cdev_init(&buttons_dev.cdev, &buttons_fops);
 buttons_dev.cdev.owner = THIS_MODULE;
 buttons_dev.cdev.ops = &buttons_fops;

 ret = cdev_add(&buttons_dev.cdev, devno, 1);
 if(ret){
  printk(DEVICE_NAME"adding buttons_dev err");
 }
 printk(DEVICE_NAME"  initialized button_major = %d\n", button_major);
 

 ret = gpio_request(SABRESD_GPIO_BATTERY, "GPIO_battery");
 if (ret) {
  printk(DEVICE_NAME "request SABRESD_GPIO_MYKEY1 error!!\n");
  return ret;
  }
 return ret;
}

static void __exit buttons_exit(void)
{
 cdev_del(&buttons_dev.cdev);
 unregister_chrdev(button_major, DEVICE_NAME);
 printk(DEVICE_NAME"  unregister\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guide");
module_init(buttons_init);
module_exit(buttons_exit);

