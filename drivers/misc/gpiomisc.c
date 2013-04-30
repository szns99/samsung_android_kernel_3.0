#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/gpio_keys.h>
#include <asm/types.h>
#include <mach/gpio.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/pwm.h>
#include "gpiomisc.h"
#if 1
#define DBG(x...)	printk(KERN_INFO x)
#else
#define DBG(x...)
#endif

#define TRUE 1
#define FALSE 0

#define GPIO_ON				(1<<31)
#define GPS_CTRL			(1<<0)
#define GPS_PWR				(1<<1)
#define BT_CTRL				(1<<2)
#define RS485_CTRL		(1<<3)
#define RS485_PWR			(1<<4)
#define RS232_CTRL		(1<<5)
#define RS232_PWR			(1<<6)
#define SER1_CS1			(1<<7)
#define SER1_CS2			(1<<8)
#define RF_PWR				(1<<9)
#define RF_RST				(1<<10)
#define SCAN_PWR			(1<<11)
#define SCAN_PWDN			(1<<12)
#define SCAN_TRIG			(1<<13)
#define SCAN_RST			(1<<14)
#define IRED_CTRL			(1<<15)


#define GPIOMISC_DEV_NAME      "gpiomisc"
#define GPIOMISC_DEV_MAJOR     0

static int      gpiomisc_major = GPIOMISC_DEV_MAJOR;
static dev_t    dev;
static struct   cdev gpiomisc_cdev;
static struct class* gpiomisc_class;
static struct gpio_platform_data* pgpio_data;
static int flag = GPS_CTRL|GPS_PWR|RS232_CTRL|RS232_PWR|SER1_CS2|SCAN_PWDN|IRED_CTRL;
extern struct pwm_device *ir_led_pwm;


static int gpiomisc_open(struct inode *inode, struct file *filp)
{
  DBG("gpiomisc_open\n");
	
	return 0;
}

static int gpiomisc_release(struct inode *inode, struct file *filp)
{
  DBG("gpiomisc_release\n");

	return 0;
}

static long gpiomisc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	int level = cmd >> 31;//取最高位作为电平状态
	if (level)
	{
		flag |= cmd;
	}
	else
	{
		flag &= ~cmd;
	}
	if (arg){
		*(int*)arg = flag;
	}
	
	DBG("gpiomisc_ioctl: cmd = %d arg = 0x%08x\n",cmd, arg);
	
	if (cmd & GPS_CTRL)
	{
		gpio_direction_output(pgpio_data->gps_io.gpio,level ? pgpio_data->gps_io.active_level : !pgpio_data->gps_io.active_level);
	}
	if (cmd & GPS_PWR)
	{
		gpio_direction_output(pgpio_data->gps_pwr.gpio,level ? pgpio_data->gps_pwr.active_level : !pgpio_data->gps_pwr.active_level);
	}
	if (cmd & BT_CTRL)
	{
		gpio_direction_output(pgpio_data->bt_io.gpio,level ? pgpio_data->bt_io.active_level : !pgpio_data->bt_io.active_level);
	}
	if (cmd & RS485_CTRL)
	{
		gpio_direction_output(pgpio_data->rs485_io.gpio,level ? pgpio_data->rs485_io.active_level : !pgpio_data->rs485_io.active_level);
	}
	if (cmd & RS485_PWR)
	{
		gpio_direction_output(pgpio_data->rs485_pwr.gpio,level ? pgpio_data->rs485_pwr.active_level : !pgpio_data->rs485_pwr.active_level);
	}
	if (cmd & RS232_CTRL)
	{
		gpio_direction_output(pgpio_data->rs232_io.gpio,level ? pgpio_data->rs232_io.active_level : !pgpio_data->rs232_io.active_level);
	}
	if (cmd & RS232_PWR)
	{
		gpio_direction_output(pgpio_data->rs232_pwr.gpio,level ? pgpio_data->rs232_pwr.active_level : !pgpio_data->rs232_pwr.active_level);
	}
	if (cmd & SER1_CS1)
	{
		gpio_direction_output(pgpio_data->ser1_cs1.gpio,level ? pgpio_data->ser1_cs1.active_level : !pgpio_data->ser1_cs1.active_level);
	}
	if (cmd & SER1_CS2)
	{
		gpio_direction_output(pgpio_data->ser1_cs2.gpio,level ? pgpio_data->ser1_cs2.active_level : !pgpio_data->ser1_cs2.active_level);
	}
	if (cmd & RF_PWR)
	{
		gpio_direction_output(pgpio_data->rf_pwr.gpio,level ? pgpio_data->rf_pwr.active_level : !pgpio_data->rf_pwr.active_level);
	}
	if (cmd & RF_RST)
	{
		gpio_direction_output(pgpio_data->rf_rst.gpio,level ? pgpio_data->rf_rst.active_level : !pgpio_data->rf_rst.active_level);
	}
	if (cmd & SCAN_PWR)
	{
		gpio_direction_output(pgpio_data->scan_pwr.gpio,level ? pgpio_data->scan_pwr.active_level : !pgpio_data->scan_pwr.active_level);
	}
	if (cmd & SCAN_PWDN)
	{
		gpio_direction_output(pgpio_data->scan_pwdn.gpio,level ? pgpio_data->scan_pwdn.active_level : !pgpio_data->scan_pwdn.active_level);
	}
	if (cmd & SCAN_TRIG)
	{
		gpio_direction_output(pgpio_data->scan_trig.gpio,level ? pgpio_data->scan_trig.active_level : !pgpio_data->scan_trig.active_level);
	}
	if (cmd & SCAN_RST)
	{
		gpio_direction_output(pgpio_data->scan_rst.gpio,level ? pgpio_data->scan_rst.active_level : !pgpio_data->scan_rst.active_level);
	}
	if (cmd & IRED_CTRL)
	{
		if(level){
			pwm_config(ir_led_pwm, 26316 / 2, 26316);
			pwm_enable(ir_led_pwm);
		}
		else{
			pwm_config(ir_led_pwm, 0, 26316);
			pwm_disable(ir_led_pwm);
		}
	}
	return ret;
}


static struct file_operations gpiomisc_fops = {
	.owner   = THIS_MODULE,
	.open    = gpiomisc_open,
	.release    = gpiomisc_release,
	.unlocked_ioctl   = gpiomisc_ioctl,
};

static int gpiomisc_probe(struct platform_device *pdev)
{
	int err;
	printk("%s:gpiomisc initialized\n",__FUNCTION__);
	pgpio_data = (struct gpio_platform_data*)pdev->dev.platform_data;
	if (pgpio_data == NULL)
	{
		return -1;
	}
	err = gpio_request(pgpio_data->gps_io.gpio, "gps_io");
	if (err)
	{
		DBG("gpio_request gps_io failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->gps_io.gpio,0);
		s5p_gpio_set_drvstr(pgpio_data->gps_io.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->gps_io.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->gps_io.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->gps_pwr.gpio, "gps_pwr");
	if (err)
	{
		DBG("gpio_request gps_pwr failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->gps_pwr.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->gps_pwr.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->gps_pwr.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->gps_pwr.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->bt_io.gpio, "bt_io");
	if (err)
	{
		DBG("gpio_request bt_io failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->bt_io.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->bt_io.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->bt_io.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->bt_io.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->rs485_io.gpio, "rs485_io");
	if (err)
	{
		DBG("gpio_request rs485_io failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->rs485_io.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->rs485_io.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->rs485_io.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->rs485_io.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->rs485_pwr.gpio, "rs485_pwr");
	if (err)
	{
		DBG("gpio_request rs485_pwr failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->rs485_pwr.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->rs485_pwr.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->rs485_pwr.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->rs485_pwr.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->rs232_io.gpio, "rs232_io");
	if (err)
	{
		DBG("gpio_request rs232_io failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->rs232_io.gpio,0);
		s5p_gpio_set_drvstr(pgpio_data->rs232_io.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->rs232_io.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->rs232_io.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->rs232_pwr.gpio, "rs232_pwr");
	if (err)
	{
		DBG("gpio_request rs232_pwr failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->rs232_pwr.gpio,0);
		s5p_gpio_set_drvstr(pgpio_data->rs232_pwr.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->rs232_pwr.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->rs232_pwr.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->ser1_cs1.gpio, "ser1_cs1");
	if (err)
	{
		DBG("gpio_request ser1_cs1 failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->ser1_cs1.gpio,0);
		s5p_gpio_set_drvstr(pgpio_data->ser1_cs1.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->ser1_cs1.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->ser1_cs1.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->ser1_cs2.gpio, "ser1_cs2");
	if (err)
	{
		DBG("gpio_request ser1_cs2 failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->ser1_cs2.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->ser1_cs2.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->ser1_cs2.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->ser1_cs2.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->rf_pwr.gpio, "rf_pwr");
	if (err)
	{
		DBG("gpio_request rf_pwr failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->rf_pwr.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->rf_pwr.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->rf_pwr.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->rf_pwr.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->rf_rst.gpio, "rf_rst");
	if (err)
	{
		DBG("gpio_request rf_rst failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->rf_rst.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->rf_rst.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->rf_rst.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->rf_rst.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->scan_pwr.gpio, "scan_pwr");
	if (err)
	{
		DBG("gpio_request scan_pwr failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->scan_pwr.gpio,0);
		s5p_gpio_set_drvstr(pgpio_data->scan_pwr.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->scan_pwr.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->scan_pwr.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->scan_pwdn.gpio, "scan_pwdn");
	if (err)
	{
		DBG("gpio_request scan_pwdn failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->scan_pwdn.gpio,0);
		s5p_gpio_set_drvstr(pgpio_data->scan_pwdn.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->scan_pwdn.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->scan_pwdn.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->scan_trig.gpio, "scan_trig");
	if (err)
	{
		DBG("gpio_request scan_trig failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->scan_trig.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->scan_trig.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->scan_trig.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->scan_trig.gpio, S3C_GPIO_SFN(1));
	}
	err = gpio_request(pgpio_data->scan_rst.gpio, "scan_rst");
	if (err)
	{
		DBG("gpio_request scan_rst failed!\n");
	}
	else
	{
		gpio_direction_output(pgpio_data->scan_rst.gpio,1);
		s5p_gpio_set_drvstr(pgpio_data->scan_rst.gpio,S5P_GPIO_DRVSTR_LV4);
		s3c_gpio_setpull(pgpio_data->scan_rst.gpio, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(pgpio_data->scan_rst.gpio, S3C_GPIO_SFN(1));
	}

	return 0;
}

static int gpiomisc_remove(struct platform_device *pdev)
{
	gpio_free(pgpio_data->gps_io.gpio);
	gpio_free(pgpio_data->gps_pwr.gpio);
	gpio_free(pgpio_data->bt_io.gpio);
	gpio_free(pgpio_data->rs485_io.gpio);
	gpio_free(pgpio_data->rs485_pwr.gpio);
	gpio_free(pgpio_data->rs232_io.gpio);
	gpio_free(pgpio_data->rs232_pwr.gpio);
	gpio_free(pgpio_data->ser1_cs1.gpio);
	gpio_free(pgpio_data->ser1_cs2.gpio);
	gpio_free(pgpio_data->rf_pwr.gpio);
	gpio_free(pgpio_data->rf_rst.gpio);
	gpio_free(pgpio_data->scan_pwr.gpio);
	gpio_free(pgpio_data->scan_pwdn.gpio);
	gpio_free(pgpio_data->scan_trig.gpio);
	gpio_free(pgpio_data->scan_rst.gpio);
	return 0;
}

static struct platform_driver gpiomisc_driver = {
	.probe	= gpiomisc_probe,
	.remove = gpiomisc_remove,
	.driver	= {
		.name	= GPIOMISC_DEV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init gpiomisc_init(void)
{
  int result;  
	platform_driver_register(&gpiomisc_driver);
  
	if (0 == gpiomisc_major)
	{
		/* auto select a major */
		result = alloc_chrdev_region(&dev, 0, 1, GPIOMISC_DEV_NAME);
		gpiomisc_major = MAJOR(dev);
	}
	else
	{
		/* use load time defined major number */
		dev = MKDEV(gpiomisc_major, 0);
		result = register_chrdev_region(dev, 1, GPIOMISC_DEV_NAME);
	}


	/* initialize our char dev data */
	cdev_init(&gpiomisc_cdev, &gpiomisc_fops);

	/* register char dev with the kernel */
	result = cdev_add(&gpiomisc_cdev, dev, 1);
    
	if (0 != result)
	{
		unregister_chrdev_region(dev,1);
		printk("Error registrating mali device object with the kernel\n");
		return result;
	}

    gpiomisc_class = class_create(THIS_MODULE, GPIOMISC_DEV_NAME);
    device_create(gpiomisc_class, NULL, MKDEV(gpiomisc_major, MINOR(dev)), NULL,
                  GPIOMISC_DEV_NAME);

	printk("gpiomisc device driver module init\n");
	return 0;
}

static void __exit gpiomisc_exit(void)
{
    device_destroy(gpiomisc_class, MKDEV(gpiomisc_major, 0));
    class_destroy(gpiomisc_class);

    cdev_del(&gpiomisc_cdev);
    unregister_chrdev_region(dev, 1);
    platform_driver_unregister(&gpiomisc_driver);
}

module_init(gpiomisc_init);
module_exit(gpiomisc_exit);
MODULE_DESCRIPTION ("gpiomisc driver");
MODULE_AUTHOR("5404385@qq.com");
MODULE_LICENSE("GPL");
