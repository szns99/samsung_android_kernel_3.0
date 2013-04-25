#ifndef _GPIO_KEYS_H
#define _GPIO_KEYS_H

struct gpio_keys_button {
	/* Configuration parameters */
	unsigned int code;	/* input event code (KEY_*, SW_*) */
	int gpio;
	int active_low;
	const char *desc;
	unsigned int type;	/* input event type (EV_KEY, EV_SW, EV_ABS) */
	int wakeup;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool can_disable;
	int value;		/* axis value for EV_ABS */
};

struct gpio_keys_platform_data {
	struct gpio_keys_button *buttons;
	int nbuttons;
	unsigned int poll_interval;	/* polling interval in msecs -
					   for polling driver only */
	unsigned int rep:1;		/* enable input subsystem auto repeat */
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
	const char *name;		/* input device name */
};

struct gpio_ops {
	int gpio;
	int active_level;
};

struct gpio_platform_data {
	struct gpio_ops gps_io;
	struct gpio_ops gps_pwr;
	struct gpio_ops bt_io;
	struct gpio_ops rs485_io;
	struct gpio_ops rs485_pwr;
	struct gpio_ops rs232_io;
	struct gpio_ops rs232_pwr;
	struct gpio_ops ser1_cs1;
	struct gpio_ops ser1_cs2;
	struct gpio_ops rf_pwr;
	struct gpio_ops rf_rst;
	struct gpio_ops scan_pwr;
	struct gpio_ops scan_pwdn;
	struct gpio_ops scan_trig;
	struct gpio_ops scan_rst;
};


#endif
