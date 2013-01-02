#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/circ_buf.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <mach/iomux.h>
#include <mach/gpio.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/mt8630.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>

MODULE_LICENSE("GPL");

#define DEBUG
#ifdef DEBUG
#define MODEMDBG(x...) printk(x)
#else
#define MODEMDBG(fmt,argss...)
#endif
#define SLEEP 1
#define READY 0
static struct wake_lock modem_wakelock;
//#define IRQ_BB_WAKEUP_AP_TRIGGER    IRQF_TRIGGER_FALLING
#define IRQ_BB_WAKEUP_AP_TRIGGER    IRQF_TRIGGER_RISING
#define MT8630_RESET 0x01
struct s5p_mt8630_data *gpdata = NULL;
struct class *modem_class = NULL; 
static int modem_status;
int modem_poweron_off(int on_off)
{
	struct s5p_mt8630_data *pdata = gpdata;		
  if(on_off)
  {
		gpio_set_value(pdata->bp_reset, GPIO_HIGH);
		msleep(100);
		gpio_set_value(pdata->bp_reset, GPIO_LOW);
		gpio_set_value(pdata->bp_power, GPIO_LOW);
		msleep(1000);
		gpio_set_value(pdata->bp_power, GPIO_HIGH);
		msleep(2000);
		gpio_set_value(pdata->bp_power, GPIO_LOW);
  }
  else
  {
		gpio_set_value(pdata->bp_power, GPIO_LOW);
		gpio_set_value(pdata->bp_power, GPIO_HIGH);
		msleep(2500);
		gpio_set_value(pdata->bp_power, GPIO_LOW);
  }

  return 0;
}
static int mt8630_open(struct inode *inode, struct file *file)
{
	struct s5p_mt8630_data *pdata = gpdata;
	device_init_wakeup(pdata->dev, 1);
	return 0;
}
static ssize_t mt8630_write(struct file *file, const char __user *buf,size_t len, loff_t *off)
{
	static char cmd[2];
	int ret = 0;
	if (len > 2) 
	{
		return -EINVAL;
	}
	ret = copy_from_user(&cmd, buf, len);
	if (ret != 0) {
		return -EFAULT;
	}
	printk(" received cmd = %c\n",cmd[0]);
	if (cmd[0] == '0')
	{
	}	
	if (cmd[0] == '1')
	{
	}
	if (cmd[0] == '2')
	{
	}
	if (cmd[0] == '3')
	{
	}
	if (cmd[0] == '4')
	{
	}if (cmd[0] == '5')
	{
	}
	return len;
}
static int mt8630_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long mt8630_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct s5p_mt8630_data *pdata = gpdata;
	switch(cmd)
	{
		case MT8630_RESET:					
			modem_poweron_off(0);
			msleep(10);
			modem_poweron_off(1);
			break;
		default:
			break;
	}
	return 0;
}

static struct file_operations mt8630_fops = {
	.owner = THIS_MODULE,
	.open = mt8630_open,
	.write = mt8630_write,
	.release = mt8630_release,
	.unlocked_ioctl = mt8630_ioctl
};

static struct miscdevice mt8630_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MODEM_NAME,
	.fops = &mt8630_fops
};
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
static ssize_t modem_status_read(struct class *cls, struct class_attribute *attr, char *_buf)
#else
static ssize_t modem_status_read(struct class *cls, char *_buf)
#endif
{

	return sprintf(_buf, "%d\n", modem_status);
	
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
static ssize_t modem_status_write(struct class *cls, struct class_attribute *attr, const char *_buf, size_t _count)
#else
static ssize_t modem_status_write(struct class *cls, const char *_buf, size_t _count)
#endif
{
    int new_state = simple_strtoul(_buf, NULL, 16);
   if(new_state == modem_status) return _count;
   if (new_state == 1){
    printk("%s, c(%d), open modem \n", __FUNCTION__, new_state);
    modem_poweron_off(1);
   }else if(new_state == 0){
     printk("%s, c(%d), close modem \n", __FUNCTION__, new_state);
     modem_poweron_off(0);
   }else{
     printk("%s, invalid parameter \n", __FUNCTION__);
   }
	modem_status = new_state;
    return _count; 
}
static CLASS_ATTR(modem_status, 0777, modem_status_read, modem_status_write);
static int mt8630_probe(struct platform_device *pdev)
{
	struct s5p_mt8630_data *pdata = gpdata = pdev->dev.platform_data;
	struct modem_dev *mt8630_data = NULL;
	int result, irq = 0;	
	pdata->dev = &pdev->dev;
	if(pdata->io_init)
		pdata->io_init();
	mt8630_data = kzalloc(sizeof(struct modem_dev), GFP_KERNEL);
	if(mt8630_data == NULL)
	{
		printk("failed to request mt8630_data\n");
		goto err0;
	}
	platform_set_drvdata(pdev, mt8630_data);	
	result = gpio_request(pdata->modem_power_en,"modem_power_en");
	if(result){
		printk("failed to request modem_power_en gpio\n");
		goto err1;
	}
	gpio_set_value(pdata->modem_power_en, GPIO_HIGH);
	msleep(2000);
  	result = gpio_request(pdata->bp_power,"modem_power");
  	if(result){
  		printk("failed to request modem_power gpio\n");
		goto err2;
  	}
	result = gpio_request(pdata->bp_reset, "bp_reset");
	if (result < 0) {
		printk("failed to request bp_reset gpio\n");	
		goto err3;
	}

	modem_poweron_off(1);
	modem_status = 1;
	 
	result = misc_register(&mt8630_misc);
	if(result)
	{
		printk("misc_register err\n");
	}	
	return result;
err0:
	kfree(mt8630_data);
err1:
	gpio_free(pdata->modem_power_en);
err2:
	gpio_free(pdata->bp_power);
err3:
	gpio_free(pdata->bp_reset);
	return 0;
}

int mt8630_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

int mt8630_resume(struct platform_device *pdev)
{
	return 0;
}

void mt8630_shutdown(struct platform_device *pdev)
{
	struct s5p_mt8630_data *pdata = pdev->dev.platform_data;
	struct modem_dev *mt8630_data = platform_get_drvdata(pdev);
	
	modem_poweron_off(0);

	if(pdata->io_deinit)
		pdata->io_deinit();
	gpio_free(pdata->modem_power_en);
	gpio_free(pdata->bp_power);
	gpio_free(pdata->bp_reset);
	kfree(mt8630_data);
}

static struct platform_driver mt8630_driver = {
	.probe	= mt8630_probe,
	.shutdown	= mt8630_shutdown,
	.suspend  	= mt8630_suspend,
	.resume		= mt8630_resume,
	.driver	= {
		.name	= "mt8630",
		.owner	= THIS_MODULE,
	},
};

static int __init mt8630_init(void)
{
	int ret ;
	modem_class = class_create(THIS_MODULE, "s5p_modem");
	ret =  class_create_file(modem_class, &class_attr_modem_status);
	if (ret)
	{
		printk("Fail to class s5p_modem.\n");
	}
	return platform_driver_register(&mt8630_driver);
}

static void __exit mt8630_exit(void)
{
	platform_driver_unregister(&mt8630_driver);
	class_remove_file(modem_class, &class_attr_modem_status);
}

module_init(mt8630_init);

module_exit(mt8630_exit);
