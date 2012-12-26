/* linux/arch/arm/mach-s5pv210/mach-smdkc110.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
//#include <linux/mfd/max8698.h>
#include <mach/power-domain.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/sysdev.h>
#include <linux/dm9000.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>
#include <linux/usb/ch9.h>
#include <linux/spi/spi.h>
#include <linux/gpio_keys.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/spi-clocks.h>
#include <mach/regs-fb.h>

#ifdef CONFIG_VIDEO_S5K4BA
#include <media/s5k4ba_platform.h>
#undef	CAM_ITU_CH_B
#define	CAM_ITU_CH_A
#endif

#ifdef CONFIG_VIDEO_SAA7113
#include <media/saa7113_platform.h>
#undef	CAM_ITU_CH_B
#define	CAM_ITU_CH_A
#endif

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/s3c64xx-spi.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/ata.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/mfc.h>
#include <plat/s5p-time.h>
#include <plat/sdhci.h>
#include <plat/fimc.h>
#include <plat/csis.h>
#include <plat/jpeg.h>
#include <plat/clock.h> 
#include <plat/regs-otg.h>
#include <plat/otg.h>
#include <plat/ehci.h>
#include <plat/ohci.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <plat/media.h>
#include <mach/media.h>
#include <mach/gpio-smdkc110.h>

#include <mach/cpu-freq-v210.h>
#ifdef CONFIG_TOUCHSCREEN_EGALAX
#include <linux/i2c/egalax.h>
#define EETI_TS_DEV_NAME        "egalax_i2c"

static struct egalax_i2c_platform_data  egalax_platdata  = {
        .gpio_int = EGALAX_IRQ,
        .gpio_en = NULL,
        .gpio_rst = NULL,
};
#endif
/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDKV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDKV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDKV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

extern void s3c_sdhci_set_platdata(void);

static struct s3c2410_uartcfg smdkv210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
};

#ifdef CONFIG_CPU_FREQ
static struct s5pv210_cpufreq_voltage s5pv210_cpufreq_volt[] = {
        {
                .freq   = 1000000,
                .varm   = 1275000,
                .vint   = 1100000,
        }, {
                .freq   =  800000,
                .varm   = 1200000,
                .vint   = 1100000,
        }, {
                .freq   =  400000,
                .varm   = 1050000,
                .vint   = 1100000,
        }, {
                .freq   =  200000,
                .varm   = 1000000,
                .vint   = 1100000,
        }, {
                .freq   =  100000,
                .varm   =  950000,
                .vint   = 1000000,
        },
};

static struct s5pv210_cpufreq_data s5pv210_cpufreq_plat = {
        .volt   = s5pv210_cpufreq_volt,
        .size   = ARRAY_SIZE(s5pv210_cpufreq_volt),
};
#endif

#if defined(CONFIG_REGULATOR_MAX8698)
/* LDO */
static struct regulator_consumer_supply smdkv210_ldo3_consumer[] = {
	REGULATOR_SUPPLY("pd_io", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_io", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_io", "s5p-ehci"),
};

static struct regulator_consumer_supply smdkv210_ldo5_consumer[] = {
	REGULATOR_SUPPLY("AVDD", "0-001b"),
	REGULATOR_SUPPLY("DVDD", "0-001b"),
};

static struct regulator_consumer_supply smdkv210_ldo8_consumer[] = {
	REGULATOR_SUPPLY("pd_core", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_core", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_core", "s5p-ehci"),
};

static struct regulator_init_data smdkv210_ldo2_data = {
	.constraints	= {
		.name		= "VALIVE_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo3_data = {
	.constraints	= {
		.name		= "VUOTG_D+VUHOST_D_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo3_consumer),
	.consumer_supplies	= smdkv210_ldo3_consumer,
};

static struct regulator_init_data smdkv210_ldo4_data = {
	.constraints	= {
		.name		= "V_MIPI_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo5_data = {
	.constraints	= {
		.name		= "VMMC+VEXT_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo5_consumer),
	.consumer_supplies	= smdkv210_ldo5_consumer,
};

static struct regulator_init_data smdkv210_ldo6_data = {
	.constraints	= {
		.name		= "VCC_2.6V",
		.min_uV		= 2600000,
		.max_uV		= 2600000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	 = {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo7_data = {
	.constraints	= {
		.name		= "VDAC_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo8_data = {
	.constraints	= {
		.name		= "VUOTG_A+VUHOST_A_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo8_consumer),
	.consumer_supplies	= smdkv210_ldo8_consumer,
};

static struct regulator_init_data smdkv210_ldo9_data = {
	.constraints	= {
		.name		= "VADC+VSYS+VKEY_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

/* BUCK */
static struct regulator_consumer_supply smdkv210_buck1_consumer =
	REGULATOR_SUPPLY("vddarm", NULL);

static struct regulator_consumer_supply smdkv210_buck2_consumer =
	REGULATOR_SUPPLY("vddint", NULL);

static struct regulator_init_data smdkv210_buck1_data = {
	.constraints	= {
		.name		= "VCC_ARM",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1250000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &smdkv210_buck1_consumer,
};

static struct regulator_init_data smdkv210_buck2_data = {
	.constraints	= {
		.name		= "VCC_INTERNAL",
		.min_uV		= 950000,
		.max_uV		= 1200000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1100000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &smdkv210_buck2_consumer,
};

static struct regulator_init_data smdkv210_buck3_data = {
	.constraints	= {
		.name		= "VCC_MEM",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.always_on	= 1,
		.apply_uV	= 1,
		.state_mem	= {
			.uV	= 1800000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

static struct max8698_regulator_data smdkv210_regulators[] = {
	{ MAX8698_LDO2,  &smdkv210_ldo2_data },
	{ MAX8698_LDO3,  &smdkv210_ldo3_data },
	{ MAX8698_LDO4,  &smdkv210_ldo4_data },
	{ MAX8698_LDO5,  &smdkv210_ldo5_data },
	{ MAX8698_LDO6,  &smdkv210_ldo6_data },
	{ MAX8698_LDO7,  &smdkv210_ldo7_data },
	{ MAX8698_LDO8,  &smdkv210_ldo8_data },
	{ MAX8698_LDO9,  &smdkv210_ldo9_data },
	{ MAX8698_BUCK1, &smdkv210_buck1_data },
	{ MAX8698_BUCK2, &smdkv210_buck2_data },
	{ MAX8698_BUCK3, &smdkv210_buck3_data },
};

static struct max8698_platform_data smdkv210_max8698_pdata = {
	.num_regulators = ARRAY_SIZE(smdkv210_regulators),
	.regulators     = smdkv210_regulators,

	/* 1GHz default voltage */
	.dvsarm1        = 0xa,  /* 1.25v */
	.dvsarm2        = 0x9,  /* 1.20V */
	.dvsarm3        = 0x6,  /* 1.05V */
	.dvsarm4        = 0x4,  /* 0.95V */
	.dvsint1        = 0x7,  /* 1.10v */
	.dvsint2        = 0x5,  /* 1.00V */
	.set1       = S5PV210_GPH1(6),
	.set2       = S5PV210_GPH1(7),
	.set3       = S5PV210_GPH0(4),
};
#endif

static struct s3c_ide_platdata smdkv210_ide_pdata __initdata = {
	.setup_gpio	= s5pv210_ide_setup_gpio,
};

static uint32_t smdkv210_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_1), KEY(0, 4, KEY_2), KEY(0, 5, KEY_3),
	KEY(0, 6, KEY_4), KEY(0, 7, KEY_5),
	KEY(1, 3, KEY_A), KEY(1, 4, KEY_B), KEY(1, 5, KEY_C),
	KEY(1, 6, KEY_D), KEY(1, 7, KEY_E), KEY(7, 1, KEY_LEFTBRACE)
};

static struct matrix_keymap_data smdkv210_keymap_data __initdata = {
	.keymap		= smdkv210_keymap,
	.keymap_size	= ARRAY_SIZE(smdkv210_keymap),
};

static struct samsung_keypad_platdata smdkv210_keypad_data __initdata = {
	.keymap_data	= &smdkv210_keymap_data,
	.rows		= 8,
	.cols		= 8,
};
#ifdef CONFIG_KEYBOARD_GPIO
static struct gpio_keys_button gpio_buttons[] = { 
  {  
   .gpio  = S5PV210_GPH1(1),  
   .code  = KEY_VOLUMEUP,   
   .desc  = "volume_up",  
   .active_low = 1,  
   //.wakeup  = 1,  
   .debounce_interval =100
 }, 
  {  
   .gpio  = S5PV210_GPH1(2),  
   .code  = KEY_VOLUMEDOWN,   
   .desc  = "volume_down",  
   .active_low = 1,  
   //.wakeup  = 1,  
   .debounce_interval =100
 }, 
  {  
   .gpio  = S5PV210_GPH1(3),  
   .code  = KEY_BACK,   
   .desc  = "key_scan",  
   .active_low = 1,  
   //.wakeup  = 1,  
   .debounce_interval =100
 }, 
}; 
  
static struct gpio_keys_platform_data gpio_button_data = {  
 .buttons = gpio_buttons,  
 .nbuttons = ARRAY_SIZE(gpio_buttons)
};  
  
static struct platform_device s3c_device_gpio_button = {  
 .name  = "gpio-keys",  
 .id  = -1,  
 .dev  = {  
  .platform_data = &gpio_button_data
 }  
};  
#endif

static struct resource smdkv210_dm9000_resources[] = {
	[0] = {
		.start	= S5PV210_PA_SROM_BANK5,
		.end	= S5PV210_PA_SROM_BANK5,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5PV210_PA_SROM_BANK5 + 2,
		.end	= S5PV210_PA_SROM_BANK5 + 2,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT(9),
		.end	= IRQ_EINT(9),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};


#ifdef CONFIG_REGULATOR
static struct regulator_consumer_supply smdkv210_b_pwr_5v_consumers[] = {
        {
                /* WM8580 */
                .supply         = "PVDD",
                .dev_name       = "0-001b",
        },
};

static struct regulator_init_data smdkv210_b_pwr_5v_data = {
        .constraints = {
                .always_on = 1,
        },
        .num_consumer_supplies  = ARRAY_SIZE(smdkv210_b_pwr_5v_consumers),
        .consumer_supplies      = smdkv210_b_pwr_5v_consumers,
};

static struct fixed_voltage_config smdkv210_b_pwr_5v_pdata = {
        .supply_name    = "B_PWR_5V",
        .microvolts     = 5000000,
        .init_data      = &smdkv210_b_pwr_5v_data,
	.gpio		= -1,
};

static struct platform_device smdkv210_b_pwr_5v = {
        .name          = "reg-fixed-voltage",
        .id            = -1,
        .dev = {
                .platform_data = &smdkv210_b_pwr_5v_pdata,
        },
};
#endif
#ifdef CONFIG_TOUCHSCREEN_EGALAX
static struct i2c_gpio_platform_data i2c5_platdata = {
        .sda_pin                = S5PV210_GPB(6),
        .scl_pin                = S5PV210_GPB(7),
        .udelay                 = 2,
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0.
};

//static struct platform_device   s3c_device_i2c5 = {
struct platform_device   s3c_device_i2c5 = {
        .name                   = "i2c-gpio",
        .id                     = 5,
        .dev.platform_data      = &i2c5_platdata,
};

static struct i2c_board_info i2c_devs5[] __initdata = {
        {
                I2C_BOARD_INFO(EETI_TS_DEV_NAME, 0x04),
                .platform_data = &egalax_platdata,
                .irq = IRQ_EINT6,
        },
};
#endif


static struct dm9000_plat_data smdkv210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x00, 0x09, 0xc0, 0xff, 0xec, 0x48 },
};

struct platform_device smdkv210_dm9000 = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smdkv210_dm9000_resources),
	.resource	= smdkv210_dm9000_resources,
	.dev		= {
		.platform_data	= &smdkv210_dm9000_platdata,
	},
};

#ifdef CONFIG_FB_S3C_LTE480WV
#define S5PV210_LCD_WIDTH 480
#define S5PV210_LCD_HEIGHT 800
#define NUM_BUFFER 4
#endif

// xxs
#ifdef CONFIG_FB_S3C_VGA1024768
#define S5PV210_LCD_WIDTH 1024 
#define S5PV210_LCD_HEIGHT 768 
#define NUM_BUFFER 8
#endif

#ifdef CONFIG_FB_S3C_101WA01S
//#define S5PV210_LCD_WIDTH 1366
#define S5PV210_LCD_WIDTH 1024
#define S5PV210_LCD_HEIGHT 768
#endif

#ifdef CONFIG_FB_S3C_TL2796
#define S5PV210_LCD_WIDTH 1024 //1280
#define S5PV210_LCD_HEIGHT 768 //800
#define NUM_BUFFER 8
#endif
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1 (9900 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD (S5PV210_LCD_WIDTH * \
                                             S5PV210_LCD_HEIGHT * NUM_BUFFER * \
                                             (CONFIG_FB_S3C_NR_BUFFERS + \
                                                 (CONFIG_FB_S3C_NUM_OVLY_WIN * \
                                                  CONFIG_FB_S3C_NUM_BUF_OVLY_WIN)))
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG (8192 * SZ_1K)
#define  S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 (1800 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D (8192 * SZ_1K)

static struct s5p_media_device s5pv210_media_devs[] = {
        [0] = {
                .id = S5P_MDEV_MFC,
                .name = "mfc",
                .bank = 0,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0,
                .paddr = 0,
        },
        [1] = {
                .id = S5P_MDEV_MFC,
                .name = "mfc",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1,
                .paddr = 0,
        },
        [2] = {
                .id = S5P_MDEV_FIMC0,
                .name = "fimc0",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0,
                .paddr = 0,
        },
        [3] = {
                .id = S5P_MDEV_FIMC1,
                .name = "fimc1",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1,
                .paddr = 0,
        },
        [4] = {
                .id = S5P_MDEV_FIMC2,
                .name = "fimc2",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2,
                .paddr = 0,
        },
        [5] = {
                .id = S5P_MDEV_JPEG,
                .name = "jpeg",
                .bank = 0,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG,
                .paddr = 0,
        },
	[6] = {
                .id = S5P_MDEV_FIMD,
                .name = "fimd",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD,
                .paddr = 0,
        },
	[7] = {
                .id = S5P_MDEV_PMEM_GPU1,
                .name = "pmem_gpu1",
                .bank = 0, /* OneDRAM */
                .memsize = S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1,
                .paddr = 0,
        },
	[8] = {
		.id = S5P_MDEV_G2D,
		.name = "g2d",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D,
		.paddr = 0,
	},
};

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
        .name = "pmem",
        .no_allocator = 1,
        .cached = 1,
        .start = 0,
        .size = 0,
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
        .name = "pmem_gpu1",
        .no_allocator = 1,
        .cached = 1,
        .buffered = 1,
        .start = 0,
        .size = 0,
};
 
static struct android_pmem_platform_data pmem_adsp_pdata = {
        .name = "pmem_adsp",
        .no_allocator = 1,
        .cached = 1,
        .buffered = 1,
        .start = 0,
        .size = 0,
};      

static struct platform_device pmem_device = {
        .name = "android_pmem",
        .id = 0,
        .dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
        .name = "android_pmem",
        .id = 1,
        .dev = { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_adsp_device = {
        .name = "android_pmem",
        .id = 2,
        .dev = { .platform_data = &pmem_adsp_pdata },
};

static void __init android_pmem_set_platdata(void)
{
        pmem_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM, 0);
        pmem_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM, 0);

        pmem_gpu1_pdata.start =
                (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_GPU1, 0);
        pmem_gpu1_pdata.size =
                (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_GPU1, 0);

        pmem_adsp_pdata.start =
                (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_ADSP, 0);
        pmem_adsp_pdata.size =
                (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_ADSP, 0);
}
#endif


static void smdkv210_lte480wv_set_power(struct plat_lcd_data *pd,
					unsigned int power)
{
	if (power) {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(3), "GPD0");
		gpio_direction_output(S5PV210_GPD0(3), 1);
		gpio_free(S5PV210_GPD0(3));
#endif

		/* fire nRESET on power up */
		gpio_request(S5PV210_GPH0(6), "GPH0");

		gpio_direction_output(S5PV210_GPH0(6), 1);

		gpio_set_value(S5PV210_GPH0(6), 0);
		mdelay(10);

		gpio_set_value(S5PV210_GPH0(6), 1);
		mdelay(10);

		gpio_free(S5PV210_GPH0(6));
	} else {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(3), "GPD0");
		gpio_direction_output(S5PV210_GPD0(3), 0);
		gpio_free(S5PV210_GPD0(3));
#endif
	}
}

static struct plat_lcd_data smdkv210_lcd_lte480wv_data = {
	.set_power	= smdkv210_lte480wv_set_power,
};

static struct platform_device smdkv210_lcd_lte480wv = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &smdkv210_lcd_lte480wv_data,
};

#if 0
static struct s3c_fb_pd_win smdkv210_fb_win0 = {
	.win_mode = {
		.left_margin	= 13,
		.right_margin	= 8,
		.upper_margin	= 7,
		.lower_margin	= 5,
		.hsync_len	= 3,
		.vsync_len	= 1,
		.xres		= 800,
		.yres		= 480,
	},
	.max_bpp	= 32,
	.default_bpp	= 24,
};

static struct s3c_fb_platdata smdkv210_lcd0_pdata __initdata = {
	.win[0]		= &smdkv210_fb_win0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
	.setup_gpio	= s5pv210_fb_gpio_setup_24bpp,
};
#endif 
//#define S5PV210_LCD_WIDTH  800
//#define S5PV210_LCD_HEIGHT 480
#ifdef CONFIG_FB_S3C_LTE480WV
static struct s3cfb_lcd lte480wv = {
        .width = S5PV210_LCD_WIDTH,
        .height = S5PV210_LCD_HEIGHT,
        .bpp = 32,
        .freq = 28,

        .timing = {
                .h_fp = 2,
                .h_bp = 2,
                .h_sw = 2,
                .v_fp = 2,
                .v_fpe = 1,
                .v_bp = 5,
                .v_bpe = 1,
                .v_sw = 2,
        },
        .polarity = {
                .rise_vclk = 1,
                .inv_hsync = 1,
                .inv_vsync = 1,
                .inv_vden = 0,
        },
};
#endif

// xxs
#ifdef CONFIG_FB_S3C_VGA1024768
static struct s3cfb_lcd lte480wv = {
        .width = S5PV210_LCD_WIDTH,
        .height = S5PV210_LCD_HEIGHT,
        .bpp = 32,
        .freq = 60,

        .timing = {
                .h_fp = 2,
                .h_bp = 250,
                .h_sw = 10,
                .v_fp = 5,
                .v_fpe = 1,
                .v_bp = 29,
                .v_bpe = 1,
                .v_sw = 3,
        },
        .polarity = {
                .rise_vclk = 1,
                .inv_hsync = 1,
                .inv_vsync = 1,
                .inv_vden = 0,
        },
};
#endif

#ifdef CONFIG_FB_S3C_TL2796
static struct s3cfb_lcd lte480wv = {
	.width = S5PV210_LCD_WIDTH,
	.height = S5PV210_LCD_HEIGHT,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 44,
		.h_bp = 50,
		.h_sw = 16,
		.v_fp = 10,
		.v_fpe = 0,
		.v_bp = 5,
		.v_bpe = 0,
		.v_sw = 3,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 0,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};
#endif
static void lte480wv_cfg_gpio(struct platform_device *pdev)
{
        int i;

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
        }
        for (i = 0; i < 4; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
        }

        /* mDNIe SEL: why we shall write 0x2 ? */
        writel(0x2, S5P_MDNIE_SEL);

#ifndef CONFIG_FB_S3C_TL2796
        /* drive strength to max */
        writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
        writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
#else
        writel(0xC0, S5PV210_GPF0_BASE + 0xc);
#endif
}



#define S5PV210_GPD_0_0_TOUT_0  (0x2)
#define S5PV210_GPD_0_1_TOUT_1  (0x2 << 4)
#define S5PV210_GPD_0_2_TOUT_2  (0x2 << 8)
#define S5PV210_GPD_0_3_TOUT_3  (0x2 << 12)
static int lte480wv_backlight_on(struct platform_device *pdev)
{
        int err;
#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(4), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(4) for "
                "LVDS PWDN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(4), 1);
        gpio_set_value(S5PV210_GPB(4), 1);
        gpio_free(S5PV210_GPB(4));
        mdelay(100);
#endif
        err = gpio_request(S5PV210_GPD0(3), "GPD0");

        if (err) {
                printk(KERN_ERR "failed to request GPD0 for "
                        "lcd backlight control\n");
                return err;
        }

        gpio_direction_output(S5PV210_GPD0(3), 1);

        s3c_gpio_cfgpin(S5PV210_GPD0(3), S5PV210_GPD_0_3_TOUT_3);

        gpio_free(S5PV210_GPD0(3));

#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(5), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(5) for "
                "LED_EN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(5), 1);
        gpio_set_value(S5PV210_GPB(5), 1);
        gpio_free(S5PV210_GPB(5));
#endif
        return 0;
}


static int lte480wv_backlight_off(struct platform_device *pdev, int onoff)
{
        int err;

#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(5), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(5) for "
                "LED_EN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(5), 0);
        gpio_set_value(S5PV210_GPB(5), 0);
        gpio_free(S5PV210_GPB(5));
#endif

        err = gpio_request(S5PV210_GPD0(3), "GPD0");

        if (err) {
                printk(KERN_ERR "failed to request GPD0 for "
                                "lcd backlight control\n");
                return err;
        }

        gpio_direction_output(S5PV210_GPD0(3), 0);
        gpio_free(S5PV210_GPD0(3));

#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(4), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(4) for "
                "LVDS PWDN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(4), 0);
        gpio_set_value(S5PV210_GPB(4), 0);
        gpio_free(S5PV210_GPB(4));
#endif
        return 0;
}

#ifdef CONFIG_FB_S3C_TL2796
void lcd_cfg_gpio_early_suspend(void)
{
	return;
}

void lcd_cfg_gpio_late_resume(void)
{
	    return;
}
#endif

static int lte480wv_reset_lcd(struct platform_device *pdev)
{
#ifndef CONFIG_FB_S3C_TL2796
        int err;

        err = gpio_request(S5PV210_GPH0(6), "GPH0");
        if (err) {
                printk(KERN_ERR "failed to request GPH0 for "
                                "lcd reset control\n");
                return err;
        }

        gpio_direction_output(S5PV210_GPH0(6), 1);
        mdelay(100);

        gpio_set_value(S5PV210_GPH0(6), 0);
        mdelay(10);

        gpio_set_value(S5PV210_GPH0(6), 1);
        mdelay(10);

        gpio_free(S5PV210_GPH0(6));
#endif
        return 0;
}

static struct s3c_platform_fb lte480wv_fb_data __initdata = {
        .hw_ver = 0x62,
        .clk_name       = "sclk_fimd",
        .nr_wins = 5,
        .default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
        .swap = FB_SWAP_WORD | FB_SWAP_HWORD,

        .lcd = &lte480wv,
        .cfg_gpio       = lte480wv_cfg_gpio,
        .backlight_on   = lte480wv_backlight_on,
        .backlight_onoff    = lte480wv_backlight_off,
        .reset_lcd      = lte480wv_reset_lcd,
};
static int smdkv210_backlight_init(struct device *dev)
{
	int ret;
	//need to check the calling function for this function and remove the call.
	return 0;

	ret = gpio_request(S5PV210_GPD0(3), "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPD for PWM-OUT 3\n");
		return ret;
	}

	/* Configure GPIO pin with S5PV210_GPD_0_3_TOUT_3 */
	s3c_gpio_cfgpin(S5PV210_GPD0(3), S3C_GPIO_SFN(2));

	return 0;
}

static void smdkv210_backlight_exit(struct device *dev)
{
	s3c_gpio_cfgpin(S5PV210_GPD0(3), S3C_GPIO_OUTPUT);
	gpio_free(S5PV210_GPD0(3));
}

static struct platform_pwm_backlight_data smdkv210_backlight_data = {
	.pwm_id		= 3,
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 1000,
	.init		= smdkv210_backlight_init,
	.exit		= smdkv210_backlight_exit,
};

static struct platform_device smdkv210_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent		= &s3c_device_timer[3].dev,
		.platform_data	= &smdkv210_backlight_data,
	},
};

struct s3c_adc_mach_info {
        /* if you need to use some platform data, add in here*/
        int delay;
        int presc;
        int resolution;
};

/*MMA7660gsensor*/
#if defined (CONFIG_GS_MMA7660)
#define MMA8452_INT_PIN   S5PV210_GPH0(7)
#include "../../../drivers/input/gsensor/mma7660.h"
/*
static int mma8452_init_platform_hw(void)
{
	int ret;
	ret = gpio_request(MMA8452_INT_PIN,"GSENSOR_INT");
	if (ret < 0)
	{
		printk("mma8452 int request failed\n");
		return -ENODEV;
	}
	else
	{
		gpio_direction_input(MMA8452_INT_PIN);//as input
		s3c_gpio_setpull(MMA8452_INT_PIN, S3C_GPIO_PULL_NONE);//
		s3c_gpio_cfgpin(MMA8452_INT_PIN, S3C_GPIO_SFN(0xF));//as int
	}
	return 0;
}
*/
static struct gsensor_platform_data mma8452_info = {
	.model = 8452,
	.swap_xy = 0,
	.swap_xyz = 1,
	//.init_platform_hw = mma8452_init_platform_hw,
	.orientation = {-1, 0, 0, 0, 0, 1, 0, -1, 0},
};
#endif

/*MMA8452 gsensor*/
#if defined (CONFIG_GS_MMA8452)
#define MMA8452_INT_PIN   S5PV210_GPH0(7)
#include "../../../drivers/input/gsensor/mma8452.h"
/*
static int mma8452_init_platform_hw(void)
{
	int ret;
	ret = gpio_request(MMA8452_INT_PIN,"GSENSOR_INT");
	if (ret < 0)
	{
		printk("mma8452 int request failed\n");
		return -ENODEV;
	}
	else
	{
		gpio_direction_input(MMA8452_INT_PIN);//as input
		s3c_gpio_setpull(MMA8452_INT_PIN, S3C_GPIO_PULL_NONE);//
		s3c_gpio_cfgpin(MMA8452_INT_PIN, S3C_GPIO_SFN(0xF));//as int
	}

	return 0;
}
*/
static struct gsensor_platform_data mma8452_info = {
	.model = 8452,
	.swap_xy = 0,
	.swap_xyz = 0,
  //.init_platform_hw = mma8452_init_platform_hw,
  .orientation = {-1, 0, 0, 0, 0, 1, 0, -1, 0},
};
#endif

static struct platform_device *smdkv210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_fb,
	&s5p_device_onenand,
#ifdef CONFIG_S3C_DEV_HSMMC
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	//&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	//&s3c_device_hsmmc3,
#endif
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
#ifdef CONFIG_KEYBOARD_GPIO
	&s3c_device_gpio_button,
#endif
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	//&s3c_device_i2c5,
#endif
	&s3c_device_rtc,
	//&s3c_device_ts,
	&s3c_device_wdt,
	&s5pv210_device_ac97,
	&s5pv210_device_iis0,
	&s5pv210_device_spdif,
	&samsung_asoc_dma,
	//&samsung_device_keypad,
	&smdkv210_dm9000,
	&smdkv210_lcd_lte480wv,
	&s3c_device_timer[3],
	&smdkv210_backlight_device,
	&s5p_device_ehci,
	&s5p_device_ohci,
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_VIDEO_FIMC
        &s3c_device_fimc0,
        &s3c_device_fimc1,
        &s3c_device_fimc2,
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	&s3c_device_csis,
#endif
#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif
#ifdef CONFIG_VIDEO_MFC50
        &s3c_device_mfc,
#endif
#ifdef CONFIG_ANDROID_PMEM
	&pmem_gpu1_device,
#endif
#ifdef CONFIG_SPI_S3C64XX
//	&s5pv210_device_spi0,
//	&s5pv210_device_spi1,
#endif
#ifdef CONFIG_REGULATOR
        &smdkv210_b_pwr_5v,
#endif
#ifdef CONFIG_S5PV210_POWER_DOMAIN
	&s5pv210_pd_tv,
        &s5pv210_pd_lcd,
        &s5pv210_pd_g3d,
        &s5pv210_pd_mfc,
        &s5pv210_pd_audio,
        &s5pv210_pd_cam,
#endif
#ifdef CONFIG_CPU_FREQ
        &s5pv210_device_cpufreq,
#endif
	&s3c_device_g3d,
#ifdef CONFIG_VIDEO_G2D
        &s3c_device_g2d,
#endif
#ifdef CONFIG_VIDEO_TV20
        &s5p_device_tvout,
        &s5p_device_cec,
        &s5p_device_hpd,
#endif
};
/*
 * External camera reset
 * Because the most of cameras take i2c bus signal, so that
 * you have to reset at the boot time for other i2c slave devices.
 * This function also called at fimc_init_camera()
 * Do optimization for cameras on your platform.
*/
#ifdef CAM_ITU_CH_A
static int smdkv210_cam0_power(int onoff)
{
	int err;
	/* Camera A */
	err = gpio_request(GPIO_PS_VOUT, "GPH0");
	if (err)
		printk(KERN_ERR "failed to request GPH0 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_PS_VOUT, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_PS_VOUT, 0);
	gpio_direction_output(GPIO_PS_VOUT, 1);
	gpio_free(GPIO_PS_VOUT);

	return 0;
}
#else
static int smdkv210_cam1_power(int onoff)
{
	int err;

	/* S/W workaround for the SMDK_CAM4_type board
	 * When SMDK_CAM4 type module is initialized at power reset,
	 * it needs the cam_mclk.
	 *
	 * Now cam_mclk is set as below, need only set the gpio mux.
	 * CAM_SRC1 = 0x0006000, CLK_DIV1 = 0x00070400.
	 * cam_mclk source is SCLKMPLL, and divider value is 8.
	*/

	/* gpio mux select the cam_mclk */
	err = gpio_request(GPIO_PS_ON, "GPJ1");
	if (err)
		printk(KERN_ERR "failed to request GPJ1 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_PS_ON, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_PS_ON, (0x3<<16));


	/* Camera B */
/*
 * acquire the gpio
 */
	err = gpio_request(GPIO_BUCK_1_EN_A, "GPH0");
	if (err)
		printk(KERN_ERR "failed to request GPH0 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_BUCK_1_EN_A, S3C_GPIO_PULL_NONE);
/*
 * set the direction of the GPQ as output and write the data
 */
	gpio_direction_output(GPIO_BUCK_1_EN_A, 0);
	gpio_direction_output(GPIO_BUCK_1_EN_A, 1);

	udelay(1000);
/*
 * release the acquired GPQ(1) gpio
 */

	gpio_free(GPIO_PS_ON);
	gpio_free(GPIO_BUCK_1_EN_A);

	return 0;
}
#endif
static int s5k5ba_power_en(int onoff)
{
	if (onoff) {
#ifdef CAM_ITU_CH_A
		smdkv210_cam0_power(onoff);
#else
		smdkv210_cam1_power(onoff);
#endif
	} else {
#ifdef CAM_ITU_CH_A
		smdkv210_cam0_power(onoff);
#else
		smdkv210_cam1_power(onoff);
#endif
	}

	return 0;
}

#ifdef CONFIG_VIDEO_S5K4EA
/* Set for MIPI-CSI Camera module Power Enable */
static int smdkv210_mipi_cam_pwr_en(int enabled)
{
	int err;

	err = gpio_request(S5PV210_GPH1(2), "GPH1");
	if (err)
		printk(KERN_ERR "#### failed to request(GPH1)for CAM_2V8\n");

	s3c_gpio_setpull(S5PV210_GPH1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPH1(2), enabled);
	gpio_free(S5PV210_GPH1(2));

	return 0;
}

/* Set for MIPI-CSI Camera module Reset */
static int smdkv210_mipi_cam_rstn(int enabled)
{
	int err;

	err = gpio_request(S5PV210_GPH0(3), "GPH0");
	if (err)
		printk(KERN_ERR "#### failed to reset(GPH0) for MIPI CAM\n");

	s3c_gpio_setpull(S5PV210_GPH0(3), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPH0(3), enabled);
	gpio_free(S5PV210_GPH0(3));

	return 0;
}

/* MIPI-CSI Camera module Power up/down sequence */
static int smdkv210_mipi_cam_power(int on)
{
	if (on) {
		smdkv210_mipi_cam_pwr_en(1);
		mdelay(5);
		smdkv210_mipi_cam_rstn(1);
	} else {
		smdkv210_mipi_cam_rstn(0);
		mdelay(5);
		smdkv210_mipi_cam_pwr_en(0);
	}
	return 0;
}
#endif

#ifdef CONFIG_VIDEO_S5K4BA
static struct s5k4ba_platform_data s5k4ba_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 44000000,
	.is_mipi = 0,
};

static struct i2c_board_info  s5k4ba_i2c_info = {
	I2C_BOARD_INFO("S5K4BA", 0x2d),
	.platform_data = &s5k4ba_plat,
};

static struct s3c_platform_camera s5k4ba = {
	#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
	#else
	.id		= CAMERA_PAR_B,
	#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.i2c_busnum	= 0,
	.info		= &s5k4ba_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	/* .srclk_name	= "xusbxti", */
	.clk_name	= "sclk_cam1",
	.clk_rate	= 44000000,
	.line_length	= 1920,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= s5k5ba_power_en,
};
#endif

#ifdef CONFIG_VIDEO_SAA7113
static struct saa7113_platform_data saa7113_plat = {
	.default_width = 720,
	.default_height = 243,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 0,
};

static struct i2c_board_info  saa7113_i2c_info = {
	I2C_BOARD_INFO("SAA7113", 0x25),
	.platform_data = &saa7113_plat,
};

static struct s3c_platform_camera saa7113 = {
	.id		= CAMERA_PAR_A,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_656_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &saa7113_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam1",
	.clk_rate	= 24000000,
	.line_length	= 1920,
	.width		= 720,
	.height		= 243,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 720,
		.height	= 243,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 0,
	.inv_href	= 0,
	.inv_hsync	= 1,

	.initialized	= 0,
#ifdef CAM_ITU_CH_A
	.cam_power	= smdkv210_cam0_power,
#else
	.cam_power	= smdkv210_cam1_power,
#endif
};
#endif


/* 2 MIPI Cameras */
#ifdef CONFIG_VIDEO_S5K4EA
static struct s5k4ea_platform_data s5k4ea_plat = {
	.default_width = 1920,
	.default_height = 1080,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 1,
};

static struct i2c_board_info  s5k4ea_i2c_info = {
	I2C_BOARD_INFO("S5K4EA", 0x2d),
	.platform_data = &s5k4ea_plat,
};

static struct s3c_platform_camera s5k4ea = {
	.id		= CAMERA_CSI_C,
	.type		= CAM_TYPE_MIPI,
	.fmt		= MIPI_CSI_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &s5k4ea_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam0",
	/* .clk_name	= "sclk_cam1", */
	.clk_rate	= 48000000,
	.line_length	= 1920,
	.width		= 1920,
	.height		= 1080,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1920,
		.height	= 1080,
	},

	.mipi_lanes	= 2,
	.mipi_settle	= 12,
	.mipi_align	= 32,

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= smdkv210_mipi_cam_power,
};
#endif

/* Interface setting */
static struct s3c_platform_fimc fimc_plat_lsi = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc",
	.lclk_name	= "fimc",
	.clk_rate	= 166750000,
#if defined(CONFIG_VIDEO_S5K4EA)
	.default_cam	= CAMERA_CSI_C,
#else
#ifdef CAM_ITU_CH_A
	.default_cam	= CAMERA_PAR_A,
#else
	.default_cam	= CAMERA_PAR_B,
#endif
#endif
	.camera		= {
#ifdef CONFIG_VIDEO_S5K4ECGX
			&s5k4ecgx,
#endif
#ifdef CONFIG_VIDEO_S5KA3DFX
			&s5ka3dfx,
#endif
#ifdef CONFIG_VIDEO_S5K4BA
			&s5k4ba,
#endif
#ifdef CONFIG_VIDEO_SAA7113
			&saa7113,
#endif
#ifdef CONFIG_VIDEO_S5K4EA
			&s5k4ea,
#endif
	},
	.hw_ver		= 0x43,
};

#ifdef CONFIG_VIDEO_JPEG_V2
static struct s3c_platform_jpeg jpeg_plat __initdata = {
	.max_main_width	= 800,
	.max_main_height	= 480,
	.max_thumb_width	= 320,
	.max_thumb_height	= 240,
};
#endif

static void __init smdkv210_dm9000_init(void)
{
	unsigned int tmp;

	gpio_request(S5PV210_MP01(5), "nCS5");
	s3c_gpio_cfgpin(S5PV210_MP01(5), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(5));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC5);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS5__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS5__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);
}

static struct i2c_board_info smdkv210_i2c_devs0[] __initdata = {
//	{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
//	{ I2C_BOARD_INFO("wm8580", 0x1b), },
	{ I2C_BOARD_INFO("ft5x0x_ts", 0x70), },
};

static struct i2c_board_info smdkv210_i2c_devs1[] __initdata = {
#ifdef CONFIG_VIDEO_TV20
        {
                I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),
        },
#endif	
};

#ifdef CONFIG_KP_AXP20
#include "../../../drivers/power/axp_power/axp-board.h"
#endif

static struct i2c_board_info smdkv210_i2c_devs2[] __initdata = {
#if defined(CONFIG_REGULATOR_MAX8698)
        {
                I2C_BOARD_INFO("max8698", 0xCC >> 1),
                .platform_data  = &smdkv210_max8698_pdata,
        },
#endif
#if defined(CONFIG_KP_AXP20)
  {
		.type = "axp_mfd",
		.addr = 0x34,
		.irq =S5PV210_GPH0(1),
		.platform_data = &axp_pdata,
  },
#endif
#if defined (CONFIG_GS_MMA8452)
	{
		.type	        = "gs_mma8452",
		.addr	        = 0x1d,
		.flags	      = 0,
		.irq	        = MMA8452_INT_PIN,
		.platform_data= &mma8452_info,
	},
#endif
#if defined (CONFIG_GS_MMA7660)
	{
		.type	        = "gs_mma8452",
		.addr	        = 0x4c,
		.flags	      = 0,
		.irq	        = MMA8452_INT_PIN,
		.platform_data= &mma8452_info,
	},
#endif
};

#ifdef CONFIG_SPI_S3C64XX

#define SMDK_MMCSPI_CS 0

static struct s3c64xx_spi_csinfo smdk_spi0_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(1),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

static struct s3c64xx_spi_csinfo smdk_spi1_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(5),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

static struct spi_board_info s3c_spi_devs[] __initdata = {
	{
		.modalias        = "spidev", /* MMC SPI */
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 0,
		.chip_select     = 0,
		.controller_data = &smdk_spi0_csi[SMDK_MMCSPI_CS],
	},
	{
		.modalias        = "spidev", /* MMC SPI */
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 1,
		.chip_select     = 0,
		.controller_data = &smdk_spi1_csi[SMDK_MMCSPI_CS],
	},
};

#endif

static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
	.delay			= 10000,
	.presc			= 49,
	.oversampling_shift	= 2,
	.cal_x_max              = 800,
        .cal_y_max              = 480,
        .cal_param              = {
                -13357, -85, 53858048, -95, -8493, 32809514, 65536
        },

};


static void __init smdkv210_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(smdkv210_uartcfgs, ARRAY_SIZE(smdkv210_uartcfgs));
#ifndef CONFIG_S5P_HIGH_RES_TIMERS
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
#endif
	s5p_reserve_bootmem(s5pv210_media_devs,
                        ARRAY_SIZE(s5pv210_media_devs), S5P_RANGE_MFC);
	 s5p_device_onenand.name = "s5pc110-onenand";
}

static void __init smdkc110_setup_clocks(void)
{
	struct clk *pclk;
	struct clk *clk;

#ifdef CONFIG_S3C_DEV_HSMMC
	/* set MMC0 clock */
	clk = clk_get(&s3c_device_hsmmc0.dev, "sclk_mmc");
	pclk = clk_get(NULL, "mout_mpll");
	clk_set_parent(clk, pclk);
	clk_set_rate(clk, 50*MHZ);

	pr_info("%s: %s: source is %s, rate is %ld\n",
				__func__, clk->name, clk->parent->name,
				clk_get_rate(clk));
#endif

#ifdef CONFIG_S3C_DEV_HSMMC1
	/* set MMC1 clock */
	clk = clk_get(&s3c_device_hsmmc1.dev, "sclk_mmc");
	pclk = clk_get(NULL, "mout_mpll");
	clk_set_parent(clk, pclk);
	clk_set_rate(clk, 50*MHZ);

	pr_info("%s: %s: source is %s, rate is %ld\n",
				__func__, clk->name, clk->parent->name,
				clk_get_rate(clk));
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
	/* set MMC2 clock */
	clk = clk_get(&s3c_device_hsmmc2.dev, "sclk_mmc");
	pclk = clk_get(NULL, "mout_mpll");
	clk_set_parent(clk, pclk);
	clk_set_rate(clk, 50*MHZ);

	pr_info("%s: %s: source is %s, rate is %ld\n",
				__func__, clk->name, clk->parent->name,
				clk_get_rate(clk));
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
	/* set MMC3 clock */
	clk = clk_get(&s3c_device_hsmmc3.dev, "sclk_mmc");
	pclk = clk_get(NULL, "mout_mpll");
	clk_set_parent(clk, pclk);
	clk_set_rate(clk, 50*MHZ);

	pr_info("%s: %s: source is %s, rate is %ld\n",
				__func__, clk->name, clk->parent->name,
			 clk_get_rate(clk));
#endif
}

/* USB EHCI */
static struct s5p_ehci_platdata smdkv210_ehci_pdata;
static void __init smdkv210_ehci_init(void)
{
        struct s5p_ehci_platdata *pdata = &smdkv210_ehci_pdata;

        s5p_ehci_set_platdata(pdata);
}

/*USB OHCI*/
static struct s5p_ohci_platdata smdkv210_ohci_pdata;
static void __init smdkv210_ohci_init(void)
{
        struct s5p_ohci_platdata *pdata = &smdkv210_ohci_pdata;

        s5p_ohci_set_platdata(pdata);
}

/*USB DEVICE*/
static struct s5p_otg_platdata smdkv210_otg_pdata;
static void __init smdkv210_otg_init(void)
{
        struct s5p_otg_platdata *pdata = &smdkv210_otg_pdata;

        s5p_otg_set_platdata(pdata);
}

#if defined (CONFIG_KP_AXP20)
extern  void axp_power_off(void);
#endif

void s5p_pm_power_off(void)
{
	printk(KERN_ERR "pm_power_off start...\n");
#if defined (CONFIG_KP_AXP20)
	axp_power_off();
#endif
	while (1);
}

void SPI_Initial(void)
{
	int err;
	err = gpio_request(S5PV210_GPJ2(4), "LCD_PWREN");
	if (err)
	{
		printk(KERN_ERR "failed to request GPJ2(4) for LCD_PWREN\n");
	}
	else
	{
		s3c_gpio_cfgpin(S5PV210_GPJ2(4), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPJ2(4), S3C_GPIO_PULL_NONE);
		gpio_direction_output(S5PV210_GPJ2(4), 1);
	}

	err = gpio_request(S5PV210_GPJ2(3), "LCD_RST");
	if (err)
	{
		printk(KERN_ERR "failed to request GPB1 for LCD_RST\n");
	}
	else
	{
		s3c_gpio_cfgpin(S5PV210_GPJ2(3), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPJ2(3), S3C_GPIO_PULL_NONE);
		gpio_direction_output(S5PV210_GPJ2(3), 1);
	}

	err = gpio_request(S5PV210_GPB(1), "SPI_CS");
	if (err)
	{
		printk(KERN_ERR "failed to request GPB1 for spi_cs\n");
	}
	else
	{
		s3c_gpio_cfgpin(S5PV210_GPB(1), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPB(1), S3C_GPIO_PULL_NONE);
		gpio_direction_output(S5PV210_GPB(1), 1);
	}

	err = gpio_request(S5PV210_GPB(0), "SPI_SCL");
	if (err)
	{
		printk(KERN_ERR "failed to request GPB0 for spi_scl\n");
	}
	else
	{
		s3c_gpio_cfgpin(S5PV210_GPB(0), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPB(0), S3C_GPIO_PULL_NONE);
		gpio_direction_output(S5PV210_GPB(0), 1);
	}

	err = gpio_request(S5PV210_GPB(3), "SPI_SDA");
	if (err)
	{
		printk(KERN_ERR "failed to request GPB3 for spi_sda\n");
	}
	else
	{
		s3c_gpio_cfgpin(S5PV210_GPB(3), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV210_GPB(3), S3C_GPIO_PULL_NONE);
		gpio_direction_output(S5PV210_GPB(3), 1);
	}
	
	mdelay(10);
	gpio_direction_output(S5PV210_GPJ2(3), 0);
	mdelay(20);
	gpio_direction_output(S5PV210_GPJ2(3), 1);
	mdelay(20);
}

void SPI_Uninitial(void)
{
	gpio_free(S5PV210_GPJ2(4));
	gpio_free(S5PV210_GPJ2(3));
	gpio_free(S5PV210_GPB(1));
	gpio_free(S5PV210_GPB(0));
	gpio_free(S5PV210_GPB(3));
}

void SetCS(int bHigh)
{
	gpio_direction_output(S5PV210_GPB(1), bHigh ? 1 : 0);
}

void SetSCL(int bHigh)
{
	gpio_direction_output(S5PV210_GPB(0), bHigh ? 1 : 0);
}

void SetSDA(int bHigh)
{
	gpio_direction_output(S5PV210_GPB(3), bHigh ? 1 : 0);
}

void Start(void)
{
	SetCS(0);
	udelay(10);
	SetSCL(0);
	udelay(10);
	SetSDA(0);
	udelay(10);
}

void Stop(void)
{
	SetCS(1);
	udelay(10);
	SetSDA(1);
	udelay(10);
	SetSCL(1);
	udelay(50);
}

void NT35582_Write_Reg(u16 reg)
{
	u16 high,low,bit;
	high = (reg>>8)|0x2000;
	low = reg&0xff;
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (high&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (low&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		udelay(10);
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
}

void NT35582_Write_Data(u8 data)
{
	u16 bit,param;
	param = data | 0x4000;
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (param&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		udelay(10);
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
}

void Init_5inch(void)
{
	SPI_Initial();

	NT35582_Write_Reg(0x1100);
	mdelay(2000);

	NT35582_Write_Reg(0xC000); NT35582_Write_Data(0x86);  
	NT35582_Write_Reg(0xC001); NT35582_Write_Data(0x00); 
	NT35582_Write_Reg(0xC002); NT35582_Write_Data(0x86);
	NT35582_Write_Reg(0xC003); NT35582_Write_Data(0x00);
	NT35582_Write_Reg(0xC100); NT35582_Write_Data(0x40);
	NT35582_Write_Reg(0xC200); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xC202); NT35582_Write_Data(0x02);
	NT35582_Write_Reg(0xB600); NT35582_Write_Data(0x30);
	NT35582_Write_Reg(0xB602); NT35582_Write_Data(0x30);
	NT35582_Write_Reg(0xE000); NT35582_Write_Data(0x0E);
	NT35582_Write_Reg(0xE001); NT35582_Write_Data(0x14);
	NT35582_Write_Reg(0xE002); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE003); NT35582_Write_Data(0x35);
	NT35582_Write_Reg(0xE004); NT35582_Write_Data(0x1C);
	NT35582_Write_Reg(0xE005); NT35582_Write_Data(0x2F);
	NT35582_Write_Reg(0xE006); NT35582_Write_Data(0x5F);
	NT35582_Write_Reg(0xE007); NT35582_Write_Data(0x3C);
	NT35582_Write_Reg(0xE008); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xE009); NT35582_Write_Data(0x29);
	NT35582_Write_Reg(0xE00A); NT35582_Write_Data(0x89);
	NT35582_Write_Reg(0xE00B); NT35582_Write_Data(0x17);
	NT35582_Write_Reg(0xE00C); NT35582_Write_Data(0x3A);
	NT35582_Write_Reg(0xE00D); NT35582_Write_Data(0x4F);
	NT35582_Write_Reg(0xE00E); NT35582_Write_Data(0x8B);
	NT35582_Write_Reg(0xE00F); NT35582_Write_Data(0xAE);
	NT35582_Write_Reg(0xE010); NT35582_Write_Data(0x4A);
	NT35582_Write_Reg(0xE011); NT35582_Write_Data(0x4D);
	NT35582_Write_Reg(0xE100); NT35582_Write_Data(0x0E);
	NT35582_Write_Reg(0xE101); NT35582_Write_Data(0x14);
	NT35582_Write_Reg(0xE102); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE103); NT35582_Write_Data(0x35);
	NT35582_Write_Reg(0xE104); NT35582_Write_Data(0x1C);
	NT35582_Write_Reg(0xE105); NT35582_Write_Data(0x2F);
	NT35582_Write_Reg(0xE106); NT35582_Write_Data(0x61);
	NT35582_Write_Reg(0xE107); NT35582_Write_Data(0x3E);
	NT35582_Write_Reg(0xE108); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xE109); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE10A); NT35582_Write_Data(0x89);
	NT35582_Write_Reg(0xE10B); NT35582_Write_Data(0x13);
	NT35582_Write_Reg(0xE10C); NT35582_Write_Data(0x3A);
	NT35582_Write_Reg(0xE10D); NT35582_Write_Data(0x4F);
	NT35582_Write_Reg(0xE10E); NT35582_Write_Data(0x8B);
	NT35582_Write_Reg(0xE10F); NT35582_Write_Data(0xAE);
	NT35582_Write_Reg(0xE110); NT35582_Write_Data(0x4A);
	NT35582_Write_Reg(0xE111); NT35582_Write_Data(0x4D);
	NT35582_Write_Reg(0xE200); NT35582_Write_Data(0x0E);
	NT35582_Write_Reg(0xE201); NT35582_Write_Data(0x14);
	NT35582_Write_Reg(0xE202); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE203); NT35582_Write_Data(0x35);
	NT35582_Write_Reg(0xE204); NT35582_Write_Data(0x1C);
	NT35582_Write_Reg(0xE205); NT35582_Write_Data(0x2F);
	NT35582_Write_Reg(0xE206); NT35582_Write_Data(0x5F);
	NT35582_Write_Reg(0xE207); NT35582_Write_Data(0x3C);
	NT35582_Write_Reg(0xE208); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xE209); NT35582_Write_Data(0x29);
	NT35582_Write_Reg(0xE20A); NT35582_Write_Data(0x89);
	NT35582_Write_Reg(0xE20B); NT35582_Write_Data(0x17);
	NT35582_Write_Reg(0xE20C); NT35582_Write_Data(0x3A);
	NT35582_Write_Reg(0xE20D); NT35582_Write_Data(0x4F);
	NT35582_Write_Reg(0xE20E); NT35582_Write_Data(0x8B);
	NT35582_Write_Reg(0xE20F); NT35582_Write_Data(0xAE);
	NT35582_Write_Reg(0xE210); NT35582_Write_Data(0x4A);
	NT35582_Write_Reg(0xE211); NT35582_Write_Data(0x4D);
	NT35582_Write_Reg(0xE300); NT35582_Write_Data(0x0E);
	NT35582_Write_Reg(0xE301); NT35582_Write_Data(0x14);
	NT35582_Write_Reg(0xE302); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE303); NT35582_Write_Data(0x35);
	NT35582_Write_Reg(0xE304); NT35582_Write_Data(0x1C);
	NT35582_Write_Reg(0xE305); NT35582_Write_Data(0x2F);
	NT35582_Write_Reg(0xE306); NT35582_Write_Data(0x61);
	NT35582_Write_Reg(0xE307); NT35582_Write_Data(0x3E);
	NT35582_Write_Reg(0xE308); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xE309); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE30A); NT35582_Write_Data(0x89);
	NT35582_Write_Reg(0xE30B); NT35582_Write_Data(0x13);
	NT35582_Write_Reg(0xE30C); NT35582_Write_Data(0x3A);
	NT35582_Write_Reg(0xE30D); NT35582_Write_Data(0x4F);
	NT35582_Write_Reg(0xE30E); NT35582_Write_Data(0x8B);
	NT35582_Write_Reg(0xE30F); NT35582_Write_Data(0xAE);
	NT35582_Write_Reg(0xE310); NT35582_Write_Data(0x4A);
	NT35582_Write_Reg(0xE311); NT35582_Write_Data(0x4D);
	NT35582_Write_Reg(0xE400); NT35582_Write_Data(0x0E);
	NT35582_Write_Reg(0xE401); NT35582_Write_Data(0x14);
	NT35582_Write_Reg(0xE402); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE403); NT35582_Write_Data(0x35);
	NT35582_Write_Reg(0xE404); NT35582_Write_Data(0x1C);
	NT35582_Write_Reg(0xE405); NT35582_Write_Data(0x2F);
	NT35582_Write_Reg(0xE406); NT35582_Write_Data(0x5F);
	NT35582_Write_Reg(0xE407); NT35582_Write_Data(0x3C);
	NT35582_Write_Reg(0xE408); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xE409); NT35582_Write_Data(0x29);
	NT35582_Write_Reg(0xE40A); NT35582_Write_Data(0x89);
	NT35582_Write_Reg(0xE40B); NT35582_Write_Data(0x17);
	NT35582_Write_Reg(0xE40C); NT35582_Write_Data(0x3A);
	NT35582_Write_Reg(0xE40D); NT35582_Write_Data(0x4F);
	NT35582_Write_Reg(0xE40E); NT35582_Write_Data(0x8B);
	NT35582_Write_Reg(0xE40F); NT35582_Write_Data(0xAE);
	NT35582_Write_Reg(0xE410); NT35582_Write_Data(0x4A);
	NT35582_Write_Reg(0xE411); NT35582_Write_Data(0x4D);
	NT35582_Write_Reg(0xE500); NT35582_Write_Data(0x0E);
	NT35582_Write_Reg(0xE501); NT35582_Write_Data(0x14);
	NT35582_Write_Reg(0xE502); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE503); NT35582_Write_Data(0x35);
	NT35582_Write_Reg(0xE504); NT35582_Write_Data(0x1C);
	NT35582_Write_Reg(0xE505); NT35582_Write_Data(0x2F);
	NT35582_Write_Reg(0xE506); NT35582_Write_Data(0x61);
	NT35582_Write_Reg(0xE507); NT35582_Write_Data(0x3E);
	NT35582_Write_Reg(0xE508); NT35582_Write_Data(0x21);
	NT35582_Write_Reg(0xE509); NT35582_Write_Data(0x25);
	NT35582_Write_Reg(0xE50A); NT35582_Write_Data(0x89);
	NT35582_Write_Reg(0xE50B); NT35582_Write_Data(0x13);
	NT35582_Write_Reg(0xE50C); NT35582_Write_Data(0x3A);
	NT35582_Write_Reg(0xE50D); NT35582_Write_Data(0x4F);
	NT35582_Write_Reg(0xE50E); NT35582_Write_Data(0x8B);
	NT35582_Write_Reg(0xE50F); NT35582_Write_Data(0xAE);
	NT35582_Write_Reg(0xE510); NT35582_Write_Data(0x4A);
	NT35582_Write_Reg(0xE511); NT35582_Write_Data(0x4D);
	NT35582_Write_Reg(0x3600); NT35582_Write_Data(0x00);
	NT35582_Write_Reg(0x3a00); NT35582_Write_Data(0x77);
	NT35582_Write_Reg(0x3B00); NT35582_Write_Data(0x0B);
	NT35582_Write_Reg(0x0C00); NT35582_Write_Data(0x60);
	mdelay(50);
	NT35582_Write_Reg(0x2900);
}

#if 0
void SPI_WriteComm(u16 reg)
{
	u16 high,low,bit;
	high = (reg>>8)|0x2000;
	low = reg&0xff;
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (high&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (low&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		udelay(10);
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
}

void SPI_WriteData(u16 data)
{
	u16 bit;
	data = (data&0x00ff)|0x4000;
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (data&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		udelay(10);
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
}

static void  OTM8018B_init(void)  //**********************************************************************
{
	printk("----------------------OTM8018B_init(void)------------------------------\n");
	SPI_Initial();
	SPI_WriteComm(0xff00);  //
	SPI_WriteData(0x80);
	SPI_WriteComm(0xff01);  // enable EXTC
	SPI_WriteData(0x09);
	SPI_WriteComm(0xff02);  // 
	SPI_WriteData(0x01);

	SPI_WriteComm(0xff80);  // enable Orise mode
	SPI_WriteData(0x80);
	SPI_WriteComm(0xff81); // 
	SPI_WriteData(0x09);

	SPI_WriteComm(0xff03);  // enable SPI+I2C cmd2 read
	SPI_WriteData(0x01);


	//gamma DC                                                                       
	SPI_WriteComm(0xc0b4); //1+2dot inversion 
	SPI_WriteData(0x10); 


	SPI_WriteComm(0xC582); //REG-pump23                                
	SPI_WriteData(0xA3);                                                 
	
	SPI_WriteComm(0xC590);  //Pump setting (3x=D6)-->(2x=96)//v02 01/11
	SPI_WriteData(0x96);      
	
	SPI_WriteComm(0xC591);  //Pump setting(VGH/VGL)                    
	SPI_WriteData(0x76);                                                 
  
	SPI_WriteComm(0xD800); //GVDD=4.5V                                     
	SPI_WriteData(0x75);                                                 
	SPI_WriteComm(0xD801);  //NGVDD=4.5V                               
	SPI_WriteData(0x73);                                                 
	
	//VCOMDC                                                                 
	SPI_WriteComm(0xd900);  // VCOMDC=                                 
	SPI_WriteData(0x4e);   
	
	//Gamma Setting                                              
	//Gamma_W16P(0xE1,9,11,17,15,9,26,11,11,1,5,3,8,14,38,35,24); //2.2+ 				
	//Gamma_W16P(0xE2,9,11,17,15,9,26,11,11,1,5,3,8,14,38,35,24); //2.2- 				

	SPI_WriteComm(0xC181); //Frame rate 65Hz//V02
	SPI_WriteData(0x66);//66

  // RGB I/F setting VSYNC for OTM8018 0x0e

	SPI_WriteComm(0xC1a1); //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
	SPI_WriteData(0x0E);//0E

    //-----------------------------------------------		
   SPI_WriteComm(0xC489); //pre-charge Enable
   SPI_WriteData(0x08);	//04/30 08->00	P-CH ON
   
   SPI_WriteComm(0xC0a2); //pre-charge //PULL-LOW 04/30
   SPI_WriteData(0x1b);
   SPI_WriteComm(0xC0a3); //pre-charge //P-CH	04/30
   SPI_WriteData(0x00);
   SPI_WriteComm(0xC0a4); //pre-charge //P-CH	04/30
   SPI_WriteData(0x02);
   
   SPI_WriteComm(0xC480); //Proch Source Output level
   SPI_WriteData(0x30);
	//-----------------------------------------------


	SPI_WriteComm(0xC481); //source bias //V02
	SPI_WriteData(0x83);

	SPI_WriteComm(0xC592); //Pump45
	SPI_WriteData(0x01);//(01)

	SPI_WriteComm(0xC5B1);  //DC voltage setting ;[0]GVDD output, default: 0xa8
	SPI_WriteData(0xA9);


	//--------------------------------------------------------------------------------
	//		initial setting 2 < tcon_goa_wave >
	//--------------------------------------------------------------------------------
	// CE8x : vst1, vst2, vst3, vst4
	SPI_WriteComm(0xCE80);	// ce81[7:0] : vst1_shift[7:0]
	SPI_WriteData(0x85);		//85->87
	SPI_WriteComm(0xCE81);	// ce82[7:0] : 0000,	vst1_width[3:0]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCE82);	// ce83[7:0] : vst1_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCE83);	// ce84[7:0] : vst2_shift[7:0]
	SPI_WriteData(0x84);		//84 -> 85
	SPI_WriteComm(0xCE84);	// ce85[7:0] : 0000,	vst2_width[3:0]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCE85);	// ce86[7:0] : vst2_tchop[7:0]
	SPI_WriteData(0x00);	                                                                                                                      
	SPI_WriteComm(0xCE86);	// ce87[7:0] : vst3_shift[7:0]                                                                                        
	SPI_WriteData(0x83);	     //83 ->86                                                                                                                 
	SPI_WriteComm(0xCE87);	// ce88[7:0] : 0000,	vst3_width[3:0]                                                                               
	SPI_WriteData(0x03);	                                                                                                                      
	SPI_WriteComm(0xCE88);	// ce89[7:0] : vst3_tchop[7:0]                                                                                        
	SPI_WriteData(0x00);	                                                                                                                      
	SPI_WriteComm(0xCE89);	// ce8a[7:0] : vst4_shift[7:0]                                                                                        
	SPI_WriteData(0x82);		//82 -> 84
	SPI_WriteComm(0xCE8a);	// ce8b[7:0] : 0000,	vst4_width[3:0]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCE8b);	// ce8c[7:0] : vst4_tchop[7:0]
	SPI_WriteData(0x00);

	//CEAx : clka1, clka2
	SPI_WriteComm(0xCEa0);	// cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]
	SPI_WriteData(0x38);
	SPI_WriteComm(0xCEa1);	// cea2[7:0] : clka1_shift[7:0]
	SPI_WriteData(0x02);
	SPI_WriteComm(0xCEa2);	// cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEa3);	// cea4[7:0] : clka1_switch[7:0]
	SPI_WriteData(0x21);
	SPI_WriteComm(0xCEa4);	// cea5[7:0] : clka1_extend[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEa5);	// cea6[7:0] : clka1_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEa6);	// cea7[7:0] : clka1_tglue[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEa7);	// cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]
	SPI_WriteData(0x38);
	SPI_WriteComm(0xCEa8);	// cea9[7:0] : clka2_shift[7:0]
	SPI_WriteData(0x01);
	SPI_WriteComm(0xCEa9);	// ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEaa);	// ceab[7:0] : clka2_switch[7:0]
	SPI_WriteData(0x22);
	SPI_WriteComm(0xCEab);	// ceac[7:0] : clka2_extend
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEac);	// cead[7:0] : clka2_tchop
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEad);	// ceae[7:0] : clka2_tglue
	SPI_WriteData(0x00);

	//CEBx : clka3, clka4
	SPI_WriteComm(0xCEb0);	// ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]
	SPI_WriteData(0x38);
	SPI_WriteComm(0xCEb1);	// ceb2[7:0] : clka3_shift[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEb2);	// ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEb3);	// ceb4[7:0] : clka3_switch[7:0]
	SPI_WriteData(0x23);
	SPI_WriteComm(0xCEb4);	// ceb5[7:0] : clka3_extend[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEb5);	// ceb6[7:0] : clka3_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEb6);	// ceb7[7:0] : clka3_tglue[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEb7);	// ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8]
	SPI_WriteData(0x30);
	SPI_WriteComm(0xCEb8);	// ceb9[7:0] : clka4_shift[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEb9);	// ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEba);	// cebb[7:0] : clka4_switch[7:0]
	SPI_WriteData(0x24);
	SPI_WriteComm(0xCEbb);	// cebc[7:0] : clka4_extend
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEbc);	// cebd[7:0] : clka4_tchop
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEbd);	// cebe[7:0] : clka4_tglue
	SPI_WriteData(0x00);

	//CECx : clkb1, clkb2
	SPI_WriteComm(0xCEc0);	// cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8]
	SPI_WriteData(0x30);
	SPI_WriteComm(0xCEc1);	// cec2[7:0] : clkb1_shift[7:0]
	SPI_WriteData(0x01);
	SPI_WriteComm(0xCEc2);	// cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEc3);	// cec4[7:0] : clkb1_switch[7:0]
	SPI_WriteData(0x25);
	SPI_WriteComm(0xCEc4);	// cec5[7:0] : clkb1_extend[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEc5);	// cec6[7:0] : clkb1_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEc6);	// cec7[7:0] : clkb1_tglue[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEc7);	// cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8]
	SPI_WriteData(0x30);
	SPI_WriteComm(0xCEc8);	// cec9[7:0] : clkb2_shift[7:0]
	SPI_WriteData(0x02);
	SPI_WriteComm(0xCEc9);	// ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEca);	// cecb[7:0] : clkb2_switch[7:0]
	SPI_WriteData(0x26);
	SPI_WriteComm(0xCEcb);	// cecc[7:0] : clkb2_extend
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEcc);	// cecd[7:0] : clkb2_tchop
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEcd);	// cece[7:0] : clkb2_tglue
	SPI_WriteData(0x00);

	//CEDx : clkb3, clkb4
	SPI_WriteComm(0xCEd0);	// ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8]
	SPI_WriteData(0x30);
	SPI_WriteComm(0xCEd1);	// ced2[7:0] : clkb3_shift[7:0]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEd2);	// ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEd3);	// ced4[7:0] : clkb3_switch[7:0]
	SPI_WriteData(0x27);
	SPI_WriteComm(0xCEd4);	// ced5[7:0] : clkb3_extend[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEd5);	// ced6[7:0] : clkb3_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEd6);	// ced7[7:0] : clkb3_tglue[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEd7);	// ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8]
	SPI_WriteData(0x30);
	SPI_WriteComm(0xCEd8);	// ced9[7:0] : clkb4_shift[7:0]
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCEd9);	// ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCEda);	// cedb[7:0] : clkb4_switch[7:0]
	SPI_WriteData(0x28);
	SPI_WriteComm(0xCEdb);	// cedc[7:0] : clkb4_extend
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEdc);	// cedd[7:0] : clkb4_tchop
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCEdd);	// cede[7:0] : clkb4_tglue
	SPI_WriteData(0x00);

	//CFCx :
	SPI_WriteComm(0xCFc0);	// cfc1[7:0] : eclk_normal_width[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc1);	// cfc2[7:0] : eclk_partial_width[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc2);	// cfc3[7:0] : all_normal_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc3);	// cfc4[7:0] : all_partial_tchop[7:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc4);	// cfc5[7:0] : eclk1_follow[3:0], eclk2_follow[3:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc5);	// cfc6[7:0] : eclk3_follow[3:0], eclk4_follow[3:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc6);	// cfc7[7:0] : 00, vstmask, vendmask, 00, dir1, dir2 (0=VGL, 1=VGH)
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc7);	// cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc8);	// cfc9[7:0] : duty_block[3:0], DGPM[3:0]
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCFc9);	// cfca[7:0] : reg_goa_gnd_period[7:0]
	SPI_WriteData(0x00);

	//CFDx :
	SPI_WriteComm(0xCFd0);	// cfd1[7:0] : 0000000, reg_goa_frame_odd_high
	SPI_WriteData(0x00);	// Parameter 1

	//--------------------------------------------------------------------------------
	//		initial setting 3 < Panel setting >
	//--------------------------------------------------------------------------------
	// cbcx
	SPI_WriteComm(0xCBc0);	//cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBc1);	//cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBc2);	//cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBc3);	//cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBc4);	//cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCBc5);	//cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCBc6);	//cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCBc7);	//cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCBc8);	//cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCBc9);	//cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCBca);	//cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBcb);	//cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBcc);	//cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBcd);	//cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBce);	//cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);

// cbdx
	SPI_WriteComm(0xCBd0);	//cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd1);	//cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd2);	//cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd3);	//cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd4);	//cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd5);	//cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd6);	//cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd7);	//cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd8);	//cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBd9);	//cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);	//4
	SPI_WriteComm(0xCBda);	//cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);	//4
	SPI_WriteComm(0xCBdb);	//cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);	//4
	SPI_WriteComm(0xCBdc);	//cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);	//4
	SPI_WriteComm(0xCBdd);	//cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);	//4
	SPI_WriteComm(0xCBde);	//cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x04);	//4

	// cbex
	SPI_WriteComm(0xCBe0);	//cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe1);	//cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe2);	//cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe3);	//cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe4);	//cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe5);	//cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe6);	//cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe7);	//cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe8);	//cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCBe9);	//cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 )
	SPI_WriteData(0x00);

	// cc8x
	SPI_WriteComm(0xCC80);	//cc81[7:0] : reg setting for signal01 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC81);	//cc82[7:0] : reg setting for signal02 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC82);	//cc83[7:0] : reg setting for signal03 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC83);	//cc84[7:0] : reg setting for signal04 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC84);	//cc85[7:0] : reg setting for signal05 selection with u2d mode
	SPI_WriteData(0x0C);
	SPI_WriteComm(0xCC85);	//cc86[7:0] : reg setting for signal06 selection with u2d mode
	SPI_WriteData(0x0A);
	SPI_WriteComm(0xCC86);	//cc87[7:0] : reg setting for signal07 selection with u2d mode
	SPI_WriteData(0x10);
	SPI_WriteComm(0xCC87);	//cc88[7:0] : reg setting for signal08 selection with u2d mode
	SPI_WriteData(0x0E);
	SPI_WriteComm(0xCC88);	//cc89[7:0] : reg setting for signal09 selection with u2d mode
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCC89);	//cc8a[7:0] : reg setting for signal10 selection with u2d mode
	SPI_WriteData(0x04);

	// cc9x
	SPI_WriteComm(0xCC90);	//cc91[7:0] : reg setting for signal11 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC91);	//cc92[7:0] : reg setting for signal12 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC92);	//cc93[7:0] : reg setting for signal13 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC93);	//cc94[7:0] : reg setting for signal14 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC94);	//cc95[7:0] : reg setting for signal15 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC95);	//cc96[7:0] : reg setting for signal16 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC96);	//cc97[7:0] : reg setting for signal17 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC97);	//cc98[7:0] : reg setting for signal18 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC98);	//cc99[7:0] : reg setting for signal19 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC99);	//cc9a[7:0] : reg setting for signal20 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC9a);	//cc9b[7:0] : reg setting for signal21 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC9b);	//cc9c[7:0] : reg setting for signal22 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC9c);	//cc9d[7:0] : reg setting for signal23 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC9d);	//cc9e[7:0] : reg setting for signal24 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCC9e);	//cc9f[7:0] : reg setting for signal25 selection with u2d mode
	SPI_WriteData(0x0B);

	// ccax
	SPI_WriteComm(0xCCa0);	//cca1[7:0] : reg setting for signal26 selection with u2d mode
	SPI_WriteData(0x09);
	SPI_WriteComm(0xCCa1);	//cca2[7:0] : reg setting for signal27 selection with u2d mode
	SPI_WriteData(0x0F);
	SPI_WriteComm(0xCCa2);	//cca3[7:0] : reg setting for signal28 selection with u2d mode
	SPI_WriteData(0x0D);
	SPI_WriteComm(0xCCa3);	//cca4[7:0] : reg setting for signal29 selection with u2d mode
	SPI_WriteData(0x01);
	SPI_WriteComm(0xCCa4);	//cca5[7:0] : reg setting for signal20 selection with u2d mode
	SPI_WriteData(0x02);
	SPI_WriteComm(0xCCa5);	//cca6[7:0] : reg setting for signal31 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCa6);	//cca7[7:0] : reg setting for signal32 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCa7);	//cca8[7:0] : reg setting for signal33 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCa8);	//cca9[7:0] : reg setting for signal34 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCa9);	//ccaa[7:0] : reg setting for signal35 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCaa);	//ccab[7:0] : reg setting for signal36 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCab);	//ccac[7:0] : reg setting for signal37 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCac);	//ccad[7:0] : reg setting for signal38 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCad);	//ccae[7:0] : reg setting for signal39 selection with u2d mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCae);	//ccaf[7:0] : reg setting for signal40 selection with u2d mode
	SPI_WriteData(0x00);

	// ccbx
	SPI_WriteComm(0xCCb0);	//ccb1[7:0] : reg setting for signal01 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCb1);	//ccb2[7:0] : reg setting for signal02 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCb2);	//ccb3[7:0] : reg setting for signal03 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCb3);	//ccb4[7:0] : reg setting for signal04 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCb4);	//ccb5[7:0] : reg setting for signal05 selection with d2u mode
	SPI_WriteData(0x0D);
	SPI_WriteComm(0xCCb5);	//ccb6[7:0] : reg setting for signal06 selection with d2u mode
	SPI_WriteData(0x0F);
	SPI_WriteComm(0xCCb6);	//ccb7[7:0] : reg setting for signal07 selection with d2u mode
	SPI_WriteData(0x09);
	SPI_WriteComm(0xCCb7);	//ccb8[7:0] : reg setting for signal08 selection with d2u mode
	SPI_WriteData(0x0B);
	SPI_WriteComm(0xCCb8);	//ccb9[7:0] : reg setting for signal09 selection with d2u mode
	SPI_WriteData(0x02);
	SPI_WriteComm(0xCCb9);	//ccba[7:0] : reg setting for signal10 selection with d2u mode
	SPI_WriteData(0x01);

	// cccx
	SPI_WriteComm(0xCCc0);	//ccc1[7:0] : reg setting for signal11 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc1);	//ccc2[7:0] : reg setting for signal12 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc2);	//ccc3[7:0] : reg setting for signal13 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc3);	//ccc4[7:0] : reg setting for signal14 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc4);	//ccc5[7:0] : reg setting for signal15 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc5);	//ccc6[7:0] : reg setting for signal16 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc6);	//ccc7[7:0] : reg setting for signal17 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc7);	//ccc8[7:0] : reg setting for signal18 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc8);	//ccc9[7:0] : reg setting for signal19 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCc9);	//ccca[7:0] : reg setting for signal20 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCca);	//cccb[7:0] : reg setting for signal21 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCcb);	//cccc[7:0] : reg setting for signal22 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCcc);	//cccd[7:0] : reg setting for signal23 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCcd);	//ccce[7:0] : reg setting for signal24 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCce);	//cccf[7:0] : reg setting for signal25 selection with d2u mode
	SPI_WriteData(0x0E);

	// ccdx
	SPI_WriteComm(0xCCd0);	//ccd1[7:0] : reg setting for signal26 selection with d2u mode
	SPI_WriteData(0x10);
	SPI_WriteComm(0xCCd1);	//ccd2[7:0] : reg setting for signal27 selection with d2u mode
	SPI_WriteData(0x0A);
	SPI_WriteComm(0xCCd2);	//ccd3[7:0] : reg setting for signal28 selection with d2u mode
	SPI_WriteData(0x0C);
	SPI_WriteComm(0xCCd3);	//ccd4[7:0] : reg setting for signal29 selection with d2u mode
	SPI_WriteData(0x04);
	SPI_WriteComm(0xCCd4);	//ccd5[7:0] : reg setting for signal30 selection with d2u mode
	SPI_WriteData(0x03);
	SPI_WriteComm(0xCCd5);	//ccd6[7:0] : reg setting for signal31 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCd6);	//ccd7[7:0] : reg setting for signal32 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCd7);	//ccd8[7:0] : reg setting for signal33 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCd8);	//ccd9[7:0] : reg setting for signal34 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCd9);	//ccda[7:0] : reg setting for signal35 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCda);	//ccdb[7:0] : reg setting for signal36 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCdb);	//ccdc[7:0] : reg setting for signal37 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCdc);	//ccdd[7:0] : reg setting for signal38 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCdd);	//ccde[7:0] : reg setting for signal39 selection with d2u mode
	SPI_WriteData(0x00);
	SPI_WriteComm(0xCCde);	//ccdf[7:0] : reg setting for signal40 selection with d2u mode
	SPI_WriteData(0x00);

	SPI_WriteComm(0x3600); 
	SPI_WriteData(0x08);

	///=============================

	SPI_WriteComm(0x3A00);  //  RGB 18bits D[17:0]
	SPI_WriteData(0x77);			

	SPI_WriteComm(0x1100);

	mdelay(120);
	
	SPI_WriteComm(0x2900);
	SPI_Uninitial();
	mdelay(50);
}
//#else
void spi_16bit_reg_wr(u8 addr, u8 index)
{
	u16 high,low,bit;
	high = addr | 0x2000;
	low = index & 0xff;
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (high&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (low&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		udelay(10);
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();

}

void spi_16bit_data(u16 param)
{
	u16 bit;
	param = (param & 0xff)|0x4000;
	Start();
	for(bit=0x8000;bit;bit>>=1)
	{
		if (param&bit)
		{
			SetSDA(1);
		}
		else
		{
			SetSDA(0);
		}
		udelay(10);
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
}

void OTM8018B_HSD50_RGB_mode(void)
{
	SPI_Initial();
	spi_16bit_reg_wr(0xff,0x00); spi_16bit_data(0x80);
	spi_16bit_reg_wr(0xff,0x01); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xff,0x02); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xff,0x80); spi_16bit_data(0x80);
	spi_16bit_reg_wr(0xff,0x81); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xff,0x03); spi_16bit_data(0x01);

	spi_16bit_reg_wr(0xc0,0xb4); spi_16bit_data(0x10);
	spi_16bit_reg_wr(0xc4,0x89); spi_16bit_data(0x08);
	spi_16bit_reg_wr(0xc0,0xa3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xc5,0x82); spi_16bit_data(0xa3);
	spi_16bit_reg_wr(0xc5,0x90); spi_16bit_data(0xd6);
	spi_16bit_reg_wr(0xc5,0x91); spi_16bit_data(0x87);

	spi_16bit_reg_wr(0xd8,0x00); spi_16bit_data(0x75);
	spi_16bit_reg_wr(0xd8,0x01); spi_16bit_data(0x73);
	spi_16bit_reg_wr(0xd9,0x00); spi_16bit_data(0x60);

	spi_16bit_reg_wr(0xe1,0x00); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xe1,0x01); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xe1,0x02); spi_16bit_data(0x11);
	spi_16bit_reg_wr(0xe1,0x03); spi_16bit_data(0x0f);
	spi_16bit_reg_wr(0xe1,0x04); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xe1,0x05); spi_16bit_data(0x1a);
	spi_16bit_reg_wr(0xe1,0x06); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xe1,0x07); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xe1,0x08); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xe1,0x09); spi_16bit_data(0x05);
	spi_16bit_reg_wr(0xe1,0x0a); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xe1,0x0b); spi_16bit_data(0x08);
	spi_16bit_reg_wr(0xe1,0x0c); spi_16bit_data(0x0d);
	spi_16bit_reg_wr(0xe1,0x0d); spi_16bit_data(0x26);
	spi_16bit_reg_wr(0xe1,0x0e); spi_16bit_data(0x23);
	spi_16bit_reg_wr(0xe1,0x0f); spi_16bit_data(0x18);

	spi_16bit_reg_wr(0xe2,0x00); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xe2,0x01); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xe2,0x02); spi_16bit_data(0x11);
	spi_16bit_reg_wr(0xe2,0x03); spi_16bit_data(0x0f);
	spi_16bit_reg_wr(0xe2,0x04); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xe2,0x05); spi_16bit_data(0x1a);
	spi_16bit_reg_wr(0xe2,0x06); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xe2,0x07); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xe2,0x08); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xe2,0x09); spi_16bit_data(0x05);
	spi_16bit_reg_wr(0xe2,0x0a); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xe2,0x0b); spi_16bit_data(0x08);
	spi_16bit_reg_wr(0xe2,0x0c); spi_16bit_data(0x0d);
	spi_16bit_reg_wr(0xe2,0x0d); spi_16bit_data(0x26);
	spi_16bit_reg_wr(0xe2,0x0e); spi_16bit_data(0x23);
	spi_16bit_reg_wr(0xe2,0x0f); spi_16bit_data(0x18);

	spi_16bit_reg_wr(0xc1,0x81); spi_16bit_data(0x66);
	spi_16bit_reg_wr(0xc1,0xa1); spi_16bit_data(0x08);
	spi_16bit_reg_wr(0xc4,0x81); spi_16bit_data(0x83);
	spi_16bit_reg_wr(0xc5,0x92); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xc5,0xb1); spi_16bit_data(0xa9);
	spi_16bit_reg_wr(0xc4,0x80); spi_16bit_data(0x30);

	spi_16bit_reg_wr(0xce,0x80); spi_16bit_data(0x85);
	spi_16bit_reg_wr(0xce,0x81); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0x82); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0x83); spi_16bit_data(0x84);
	spi_16bit_reg_wr(0xce,0x84); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0x85); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0x86); spi_16bit_data(0x83);
	spi_16bit_reg_wr(0xce,0x87); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0x88); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0x89); spi_16bit_data(0x82);
	spi_16bit_reg_wr(0xce,0x8a); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0x8b); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xce,0xa0); spi_16bit_data(0x38);
	spi_16bit_reg_wr(0xce,0xa1); spi_16bit_data(0x02);
	spi_16bit_reg_wr(0xce,0xa2); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xa3); spi_16bit_data(0x21);
	spi_16bit_reg_wr(0xce,0xa4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xa5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xa6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xa7); spi_16bit_data(0x38);
	spi_16bit_reg_wr(0xce,0xa8); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xce,0xa9); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xaa); spi_16bit_data(0x22);
	spi_16bit_reg_wr(0xce,0xab); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xac); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xad); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xce,0xb0); spi_16bit_data(0x38);
	spi_16bit_reg_wr(0xce,0xb1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xb2); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xb3); spi_16bit_data(0x23);
	spi_16bit_reg_wr(0xce,0xb4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xb5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xb6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xb7); spi_16bit_data(0x30);
	spi_16bit_reg_wr(0xce,0xb8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xb9); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xba); spi_16bit_data(0x24);
	spi_16bit_reg_wr(0xce,0xbb); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xbc); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xbd); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xce,0xc0); spi_16bit_data(0x30);
	spi_16bit_reg_wr(0xce,0xc1); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xce,0xc2); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xc3); spi_16bit_data(0x25);
	spi_16bit_reg_wr(0xce,0xc4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xc5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xc6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xc7); spi_16bit_data(0x30);
	spi_16bit_reg_wr(0xce,0xc8); spi_16bit_data(0x02);
	spi_16bit_reg_wr(0xce,0xc9); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xca); spi_16bit_data(0x26);
	spi_16bit_reg_wr(0xce,0xcb); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xcc); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xcd); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xce,0xd0); spi_16bit_data(0x30);
	spi_16bit_reg_wr(0xce,0xd1); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xd2); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xd3); spi_16bit_data(0x27);
	spi_16bit_reg_wr(0xce,0xd4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xd5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xd6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xd7); spi_16bit_data(0x30);
	spi_16bit_reg_wr(0xce,0xd8); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xce,0xd9); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xce,0xda); spi_16bit_data(0x28);
	spi_16bit_reg_wr(0xce,0xdb); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xdc); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xce,0xdd); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xcf,0xc0); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc2); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc7); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcf,0xc9); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xcf,0xd0); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xcb,0xc0); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xc1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xc2); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xc3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xc4); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xc5); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xc6); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xc7); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xc8); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xc9); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xca); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xcb); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xcc); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xcd); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xce); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xcb,0xd0); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd2); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd7); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xd9); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xda); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xdb); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xdc); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xdd); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcb,0xde); spi_16bit_data(0x04);

	spi_16bit_reg_wr(0xcb,0xe0); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe2); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe7); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcb,0xe9); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xcc,0x80); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x81); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x82); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x83); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x84); spi_16bit_data(0x0c);
	spi_16bit_reg_wr(0xcc,0x85); spi_16bit_data(0x0a);
	spi_16bit_reg_wr(0xcc,0x86); spi_16bit_data(0x10);
	spi_16bit_reg_wr(0xcc,0x87); spi_16bit_data(0x0e);
	spi_16bit_reg_wr(0xcc,0x88); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xcc,0x89); spi_16bit_data(0x04);

	spi_16bit_reg_wr(0xcc,0x90); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x91); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x92); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x93); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x94); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x95); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x96); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x97); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x98); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x99); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x9a); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x9b); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x9c); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x9d); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0x9e); spi_16bit_data(0x0b);

	spi_16bit_reg_wr(0xcc,0xa0); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xcc,0xa1); spi_16bit_data(0x0f);
	spi_16bit_reg_wr(0xcc,0xa2); spi_16bit_data(0x0d);
	spi_16bit_reg_wr(0xcc,0xa3); spi_16bit_data(0x01);
	spi_16bit_reg_wr(0xcc,0xa4); spi_16bit_data(0x02);
	spi_16bit_reg_wr(0xcc,0xa5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xa6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xa7); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xa8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xa9); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xaa); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xab); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xac); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xad); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xae); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xcc,0xb0); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xb1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xb2); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xb3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xb4); spi_16bit_data(0x0d);
	spi_16bit_reg_wr(0xcc,0xb5); spi_16bit_data(0x0f);
	spi_16bit_reg_wr(0xcc,0xb6); spi_16bit_data(0x09);
	spi_16bit_reg_wr(0xcc,0xb7); spi_16bit_data(0x0b);
	spi_16bit_reg_wr(0xcc,0xb8); spi_16bit_data(0x02);
	spi_16bit_reg_wr(0xcc,0xb9); spi_16bit_data(0x01);

	spi_16bit_reg_wr(0xcc,0xc0); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc1); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc2); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc3); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc4); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc7); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xc9); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xca); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xcb); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xcc); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xcd); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xce); spi_16bit_data(0x0e);

	spi_16bit_reg_wr(0xcc,0xd0); spi_16bit_data(0x10);
	spi_16bit_reg_wr(0xcc,0xd1); spi_16bit_data(0x0a);
	spi_16bit_reg_wr(0xcc,0xd2); spi_16bit_data(0x0c);
	spi_16bit_reg_wr(0xcc,0xd3); spi_16bit_data(0x04);
	spi_16bit_reg_wr(0xcc,0xd4); spi_16bit_data(0x03);
	spi_16bit_reg_wr(0xcc,0xd5); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xd6); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xd7); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xd8); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xd9); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xda); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xdb); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xdc); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xdd); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0xcc,0xde); spi_16bit_data(0x00);

	spi_16bit_reg_wr(0xb2,0x82); spi_16bit_data(0x00);
	spi_16bit_reg_wr(0x11,0x00);
	mdelay(150);
	spi_16bit_reg_wr(0x29,0x00);
}
#endif

static void __init smdkv210_machine_init(void)
{
	s3c_pm_init();
	pm_power_off = s5p_pm_power_off;
	smdkv210_dm9000_init();
	platform_add_devices(smdkv210_devices, ARRAY_SIZE(smdkv210_devices));

#ifdef CONFIG_ANDROID_PMEM
        android_pmem_set_platdata();
#endif

	//samsung_keypad_set_platdata(&smdkv210_keypad_data);
	//s3c24xx_ts_set_platdata(&s3c_ts_platform);

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, smdkv210_i2c_devs0,
			ARRAY_SIZE(smdkv210_i2c_devs0));
	i2c_register_board_info(1, smdkv210_i2c_devs1,
			ARRAY_SIZE(smdkv210_i2c_devs1));
	i2c_register_board_info(2, smdkv210_i2c_devs2,
		 	ARRAY_SIZE(smdkv210_i2c_devs2));
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));
#endif

#ifdef CONFIG_CPU_FREQ 
	s5pv210_cpufreq_set_platdata(&s5pv210_cpufreq_plat);
#endif
	//s3c_ide_set_platdata(&smdkv210_ide_pdata);

//	s3c_fb_set_platdata(&smdkv210_lcd0_pdata);
	//OTM8018B_init();
	//OTM8018B_HSD50_RGB_mode();
	Init_5inch();
	printk("----------------------s3c_fb_set_platdata(&lte480wv_fb_data)------------------------------\n");
	s3c_fb_set_platdata(&lte480wv_fb_data);

#ifdef CONFIG_S3C_DEV_HSMMC
	s5pv210_default_sdhci0();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s5pv210_default_sdhci1();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	s5pv210_default_sdhci2();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s5pv210_default_sdhci3();
#endif
#ifdef CONFIG_S5PV210_SETUP_SDHCI
	s3c_sdhci_set_platdata();
#endif

#ifdef CONFIG_VIDEO_FIMC
        /* fimc */
        s3c_fimc0_set_platdata(&fimc_plat_lsi);
        s3c_fimc1_set_platdata(&fimc_plat_lsi);
        s3c_fimc2_set_platdata(&fimc_plat_lsi);
#ifdef CAM_ITU_CH_A
	smdkv210_cam0_power(1);
#else
	smdkv210_cam1_power(1);
#endif
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	s3c_csis_set_platdata(NULL);
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	s3c_jpeg_set_platdata(&jpeg_plat);
#endif
#ifdef CONFIG_VIDEO_MFC50
        /* mfc */
        s3c_mfc_set_platdata(NULL);
#endif
        /* spi */
#ifdef CONFIG_SPI_S3C64XX
//	if (!gpio_request(S5PV210_GPB(1), "SPI_CS0")) {
//	    gpio_direction_output(S5PV210_GPB(1), 1);
//	    s3c_gpio_cfgpin(S5PV210_GPB(1), S3C_GPIO_SFN(1));
//	    s3c_gpio_setpull(S5PV210_GPB(1), S3C_GPIO_PULL_UP);
//	    s5pv210_spi_set_info(0, S5PV210_SPI_SRCCLK_PCLK,
//		    ARRAY_SIZE(smdk_spi0_csi));
//	}
//	if (!gpio_request(S5PV210_GPB(5), "SPI_CS1")) {
//	    gpio_direction_output(S5PV210_GPB(5), 1);
//	    s3c_gpio_cfgpin(S5PV210_GPB(5), S3C_GPIO_SFN(1));
//	    s3c_gpio_setpull(S5PV210_GPB(5), S3C_GPIO_PULL_UP);
//	    s5pv210_spi_set_info(1, S5PV210_SPI_SRCCLK_PCLK,
//		    ARRAY_SIZE(smdk_spi1_csi));
//	}
//	spi_register_board_info(s3c_spi_devs, ARRAY_SIZE(s3c_spi_devs));
#endif

	smdkv210_otg_init();
        smdkv210_ehci_init();
        smdkv210_ohci_init();
        clk_xusbxti.rate = 24000000;
	 smdkc110_setup_clocks(); 
}

MACHINE_START(SMDKC110, "SMDKC110")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= s5pv210_init_irq,
	.map_io		= smdkv210_map_io,
	.init_machine	= smdkv210_machine_init,
#ifdef CONFIG_S5P_HIGH_RES_TIMERS
        .timer          = &s5p_systimer,
#else
	.timer		= &s5p_timer,
#endif
MACHINE_END
