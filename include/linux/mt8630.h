#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>

struct modem_dev
{
	const char *name;
	struct miscdevice miscdev;
	struct work_struct work;
};

struct s5p_mt8630_data {
	struct device *dev;
	int (*io_init)(void);
	int (*io_deinit)(void);
	unsigned int bp_power;
	unsigned int bp_reset;
	unsigned int modem_power_en;
	unsigned int irq;
};

#define MODEM_NAME "mt8630"
