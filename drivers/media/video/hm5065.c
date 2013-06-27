/*
 * Driver for HM5065 CMOS Image Sensor from Omnivision
 *
 * Copyright (C) 2012, Luo Jun <3869714@qq.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/hm5065_platform.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#include <linux/slab.h>

#include "hm5065.h"

//#define TEST_CODE
#define DEBUG_CAMERA

#ifdef DEBUG_CAMERA
#define dev_dbg_cam(dev, format, arg...)		\
	dev_printk(KERN_INFO, dev , format , ## arg)
#else
#define dev_dbg_cam
#endif

#define HM5065_DRIVER_NAME	"HM5065"

/* Default resolution & pixelformat. plz ref hm5065_platform.h */
#define DEFAULT_RES			WVGA	/* Index of resoultion */
#define DEFAUT_FPS_INDEX	HM5065_15FPS
#define DEFAULT_FMT			V4L2_PIX_FMT_NV21  //V4L2_PIX_FMT_YUYV	/* YUV422 */
extern int mi108_cam_i2c(unsigned char addr, unsigned short reg, unsigned char value);

/*
 * Specification
 * Parallel : ITU-R. 656/601 YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Serial : MIPI CSI2 (single lane) YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Resolution : 1280 (H) x 1024 (V)
 * Image control : Brightness, Contrast, Saturation, Sharpness, Glamour
 * Effect : Mono, Negative, Sepia, Aqua, Sketch
 * FPS : 15fps @full resolution, 30fps @VGA, 24fps @720p
 * Max. pixel clock frequency : 48MHz(upto)
 * Internal PLL (6MHz to 27MHz input frequency)
 */

/* Camera functional setting values configured by user concept */
struct hm5065_userset {
	signed int exposure;	/* V4L2_CID_EXPOSURE */
	unsigned int ae_lock;
	unsigned int awb_lock;
	unsigned int auto_wb;	/* V4L2_CID_AUTO_WHITE_BALANCE */
	unsigned int manual_wb;	/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int index_wb;
	unsigned int wb_temp;	/* V4L2_CID_WHITE_BALANCE_TEMPERATURE */
	unsigned int effect;	/* Color FX (AKA Color tone) */
	unsigned int contrast;	/* V4L2_CID_CONTRAST */
	unsigned int brightness;	/* V4L2_CID_BRIGHTNESS */	
	unsigned int saturation;	/* V4L2_CID_SATURATION */
	unsigned int sharpness;		/* V4L2_CID_SHARPNESS */
	unsigned int glamour;
	unsigned int zoom;
	unsigned int af_mode;
	unsigned int flash_mode;
	unsigned int flash_lastStatus;
};

struct hm5065_state {
	struct hm5065_platform_data *pdata;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct hm5065_userset userset;
	int framesize_index;    
	int freq;	/* MCLK in KHz */
	int is_mipi;
	int isize;
	int ver;
	int fps;
	int check_previewdata;    
	struct delayed_work work;	
	//struct delayed_work debugCheck;
	int isinitCall;
};


static struct hm5065_format_struct {
	__u8	*desc;
	__u32	pixelformat;
	int	width;
	int	height;
	int	resolution_width;
	int	resolution_height;    
	int	fps;
	__u8 index;
	int	bpp;   /* bits per pixel */
}
hm5065_formats[] = {
	{
		.desc		= "HM5065 5M",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.width		= 2592,
		.height		= 1936,//1944
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	{
		.desc		= "HM5065 2M",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.width		= 1600,
		.height		= 1200,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},	
	{
		.desc		= "HM5065 1M",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.width		= 1024,
		.height		= 768,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	/*	
	{
		.desc		= "HM5065 VGA",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.width		= 640,
		.height		= 480,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},	
	{
		.desc		= "HM5065 QVGA",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.width		= 320,
		.height		= 240,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},		
	{
		.desc		= "HM5065 1080p",
		.pixelformat	= DEFAULT_FMT,
		.width		= 1920,
		.height		= 1080,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	{
		.desc		= "HM5065 Quad VGA",
		.pixelformat	= DEFAULT_FMT,
		.width		= 1280,
		.height		= 960,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	{
		.desc		= "HM5065 720p",
		.pixelformat	= DEFAULT_FMT,
		.width		= 1280,
		.height		= 720,
		.resolution_width		= HM5065_CAPTURE_WIDTH,
		.resolution_height		= HM5065_CAPTURE_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	{
		.desc		= "HM5065 VGA",
		.pixelformat	= DEFAULT_FMT,
		.width		= 640,
		.height		= 480,
		.resolution_width		= HM5065_PREVIEW_WIDTH,
		.resolution_height		= HM5065_PREVIEW_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	{
		.desc		= "HM5065 QVGA",
		.pixelformat	= DEFAULT_FMT,
		.width		= 320,
		.height		= 240,
		.resolution_width		= HM5065_PREVIEW_WIDTH,
		.resolution_height		= HM5065_PREVIEW_HEIGHT,				
		.fps		= 15,
		.index		= 1,
		.bpp		= 16,
	},
	*/
};

#define N_HM5065_FMTS  (sizeof(hm5065_formats) / sizeof((hm5065_formats)[0]))	


enum 
{
    HM5065_PREVIEW_VGA,
    //HM5065_CAPTRUE_UXGA,
    HM5065_CAPTRUE_5M,
    HM5065_CAPTRUE_2M,
    HM5065_CAPTRUE_1M,
};

struct hm5065_enum_framesize
{
	unsigned int index;
	unsigned int width;
	unsigned int height;
};

struct hm5065_enum_framesize hm5065_framesize_list[] = 
{
    { HM5065_PREVIEW_VGA, HM5065_PREVIEW_WIDTH, HM5065_PREVIEW_HEIGHT },
    //{ HM5065_CAPTRUE_UXGA, HM5065_CAPTURE_WIDTH, HM5065_CAPTURE_HEIGHT }
    { HM5065_CAPTRUE_5M, 2592, 1936},
    { HM5065_CAPTRUE_2M, 1600, 1200},
    { HM5065_CAPTRUE_1M, 1024, 768}
};

static u8 af_pos_h = 0;
static u8 af_pos_l = 0;


#ifdef TEST_CODE
static struct v4l2_subdev *g_sd;
static struct class *HM5065Class = NULL; 

static int hm5065_i2c_reg(struct v4l2_subdev *sd, unsigned short addr, unsigned char value);


int hm5065_test_reg(unsigned short addr, unsigned char value)
{
    return hm5065_i2c_reg(g_sd, addr, value);
}

static ssize_t hm5065_reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    printk("%s\n", __func__);
  	return 0;
}

static ssize_t hm5065_reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned short addr;
    unsigned char value;

    printk("%s, buf=%s\n", __func__, buf);
    
    sscanf(buf, "0x%04x,0x%02x\n", &addr, &value);
    printk("%s, addr is 0x%x, value is 0x%x\n", __func__, addr, value);
    hm5065_test_reg(addr, value);
    return size;
}

static DEVICE_ATTR(reg, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH,hm5065_reg_show ,hm5065_reg_store );


#endif


//************************************************************//
//extern void setlightOn(int ison);
//extern void mi108_set_flash_status(int ison);


static int hm5065_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl);

static inline struct hm5065_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct hm5065_state, sd);
}

static int hm5065_init(struct v4l2_subdev *sd, u32 val);
static int hm5065_reset(struct v4l2_subdev *sd)
{
	return hm5065_init(sd, 0);
}


static int hm5065_i2c_write(struct v4l2_subdev *sd, unsigned char i2c_data[],
				unsigned char length)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[length], i;
	struct i2c_msg msg = {client->addr, 0, length, buf};
	int ret;

	for (i = 0; i < length; i++)
		buf[i] = i2c_data[i];

	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret != 1)
	{
		printk("i2c error %d\n", ret);
	}
	return (ret == 1) ? 0 : -EIO;
}

static int hm5065_i2c_reg(struct v4l2_subdev *sd, unsigned short addr, unsigned char value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[3];
	int ret;
	struct i2c_msg msg = {client->addr, 0, 3, buf};
	buf[0] = addr >> 8;
	buf[1] = addr  & 0xff;
	buf[2] = value;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret != 1)
	{
		printk("hm5065_i2c_reg error %d\n", ret);
	}
	return (ret == 1) ? 0 : -EIO;
}

static int hm5065_write_regs(struct v4l2_subdev *sd, const HM5065_REG  *regs,
				int size)
{
	int i, err;
	char buf[3];

	for (i = 0; i < size; i++) 
	{
		if(regs[i].reg == 0x0000)
		{
			msleep(regs[i].val);
		}
		else
		{
			buf[0] = regs[i].reg >> 8;
			buf[1] = regs[i].reg  & 0xff;
			buf[2] = regs[i].val;
			err = hm5065_i2c_write(sd, buf,3);
			if (err < 0)
				return -EIO;
		}
	}

	return 0;
}

/*
 * hm5065 register structure : 2bytes address, 2bytes value
 * retry on write failure up-to 5 times
 */
static inline int hm5065_read(struct v4l2_subdev *sd, u16 addr, u8 *val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg[2];
	unsigned char reg[10];
	int err = 0;
	int retry = 0;

	if (!client->adapter)
		return -ENODEV;

again:
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = &reg[0];
	reg[0] = addr >> 8;
	reg[1] = addr & 0xff;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &reg[2];	

	err = i2c_transfer(client->adapter, msg, 2);
	if (err >= 0)
	{
	    *val = reg[2];	
		return err;	/* Returns here on success */
	}
	/* abnormal case: retry 5 times */
	if (retry < 3) {
		dev_err(&client->dev, "%s: address: 0x%02x%02x, " \
			" err:%d\n", __func__, \
			reg[0], reg[1], err);
		retry++;
		goto again;
	}


	dev_dbg_cam(&client->dev, "%s: address: 0x%02x%02x, " \
			" err:%d\n", __func__, \
			reg[0], reg[1], err);

	return err;
}
/*
static int hm5065_read_regs(struct v4l2_subdev *sd, u8 *regs, u8 *val,
				int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

	for (i = 0; i < size; i++) {
		err = hm5065_read(sd, *(regs+i), val);
		if (err < 0)
			v4l_info(client, "%s: register read failed\n", \
			__func__);
	}

	return 0;	
}
*/

/*static int hm5065_setting_update(struct v4l2_subdev *sd, int issnapshot)
{

	struct hm5065_state *state = to_state(sd);

       hm5065_write_regs(sd, \
            	hm5065_regs_special_effect[state->userset.effect], \
            		hm5065_regs_special_effect_size[state->userset.effect]);

	if((state->userset.index_wb == HM5065_AWB_MODE_AUTO_SIMPLE) && issnapshot)
	{
			hm5065_write_regs(sd, \
                		hm5065_regs_light_mode[HM5065_AWB_MODE_AUTO_ADVANCED], \
                		hm5065_regs_light_mode_size[HM5065_AWB_MODE_AUTO_ADVANCED]);

	}
	else
	{
		hm5065_write_regs(sd, \
	                		hm5065_regs_light_mode[state->userset.index_wb], \
	                		hm5065_regs_light_mode_size[state->userset.index_wb]);
	}

	printk("brightness = %d, effect =%d, index_wb = %d\n", state->userset.brightness, state->userset.effect, state->userset.index_wb);

	return 0;

}*/


static int hm5065_flash_control(struct v4l2_subdev *sd, int ison)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm5065_platform_data *pdata = client->dev.platform_data;

	if(ison)
	{
			//state->flash_on = true;
			//state->flash_state_on_previous_capture = true;
			pdata->flash_onoff(1);
	}
	else
	{
		//state->flash_on = false;
		pdata->flash_onoff(0);
	}

	return 0;
}


static int hm5065_preview(struct v4l2_subdev *sd)
{
	struct hm5065_state *state = to_state(sd);
	int err = -EINVAL;

	printk("hm5065_preview \n");
	
    //err = hm5065_write_regs(sd, hm5065_init_setting, hm5065_init_setting_size);
	err = hm5065_write_regs(sd, hm5065_regs_preview, hm5065_regs_preview_size);

 	if (err < 0) {
		state->check_previewdata = 10000;
		printk(
			"hm5065_init: camera initialization failed. err(%d)\n",
			 state->check_previewdata);
		return -EIO;
	}

	msleep(20);

	/* This is preview success */
	state->check_previewdata = 0;
	return 0;
}

/*#define Capture_Framerate 375
#define Preview_FrameRate 1500
static int hm5065_check_exp(struct v4l2_subdev *sd)
{
	unsigned char ret_h, ret_m, ret_l,Gain;
	unsigned int preview_exposure, iCapture_Gain,Preview_Maxlines,Capture_MaxLines,Lines_10ms;
	unsigned int ulCapture_Exposure,ulCapture_Exposure_Gain;
	ret_h = ret_m = ret_l = 0;	
	hm5065_read(sd,0x3500, &ret_h);
	hm5065_read(sd,0x3501, &ret_m);
	hm5065_read(sd,0x3502, &ret_l);
	preview_exposure = (ret_h << 12) + (ret_m << 4) + (ret_l >> 4);

	Gain = 0;
	hm5065_read(sd,0x350b, &Gain);


	Preview_Maxlines = 960;
	Capture_MaxLines = 1964;

	Lines_10ms = Capture_Framerate * Capture_MaxLines/10000*13/12;
		
	ulCapture_Exposure =((preview_exposure*(Capture_Framerate)*(Capture_MaxLines))/(((Preview_Maxlines)*(Preview_FrameRate))))*6/5; 


	//iCapture_Gain = Gain ;

	
	iCapture_Gain = (Gain & 0x0f) + 16;

	if (Gain & 0x10)
		iCapture_Gain = iCapture_Gain << 1;

	if (Gain & 0x20)
		iCapture_Gain = iCapture_Gain << 1;

	if (Gain & 0x40)
		iCapture_Gain = iCapture_Gain << 1;

	if (Gain & 0x80)
		iCapture_Gain = iCapture_Gain << 1;

	
	printk("read preview_exposure %d, gain %d\n", preview_exposure, iCapture_Gain);

	ulCapture_Exposure_Gain = ulCapture_Exposure * iCapture_Gain  / 2;


	if(ulCapture_Exposure_Gain <((Capture_MaxLines)*16))
	{
		ulCapture_Exposure = ulCapture_Exposure_Gain/16;
		if (ulCapture_Exposure > Lines_10ms)
		{
			ulCapture_Exposure /= Lines_10ms;
			ulCapture_Exposure *= Lines_10ms;
		}
	}
	else
	{
		ulCapture_Exposure = Capture_MaxLines;
	}

	if(ulCapture_Exposure == 0)
	{
		ulCapture_Exposure = 1;
	}	

	iCapture_Gain = (ulCapture_Exposure_Gain*2/ulCapture_Exposure +1)/2;	
	
	printk("ulCapture exposure %d, Gain %d\n", ulCapture_Exposure, iCapture_Gain);

	ret_l = ((ulCapture_Exposure)<<4)  & 0xff;
	ret_m = (ulCapture_Exposure >> 4) & 0xff;
	ret_h = (ulCapture_Exposure >> 12)  & 0xff;

	
		
	//Gain = iCapture_Gain & 0xff;

	Gain = 0;

	if (iCapture_Gain > 31)
	{
		Gain |= 0x10;
		iCapture_Gain = iCapture_Gain >> 1;
	}

	if (iCapture_Gain > 31)
	{
		Gain |= 0x20;
		iCapture_Gain = iCapture_Gain >> 1;
	}

	if (iCapture_Gain > 31)
	{
		Gain |= 0x40;
		iCapture_Gain = iCapture_Gain >> 1;
	}

	if (iCapture_Gain > 31)
	{
		Gain |= 0x80;
		iCapture_Gain = iCapture_Gain >> 1;
	}

	if (iCapture_Gain > 16)
	{
		Gain |= ((iCapture_Gain -16) & 0x0f);
	}

	if(Gain == 0x10)
	{
		Gain = 0x11;
	}	

	hm5065_i2c_reg(sd,0x350b, Gain);
	hm5065_i2c_reg(sd,0x3502, ret_l);
	hm5065_i2c_reg(sd,0x3501, ret_m);
	hm5065_i2c_reg(sd,0x3500, ret_h);
	return 0;
}*/

static int hm5065_Snapshot(struct v4l2_subdev *sd)
{
	int err;
    u8 value1, value2;

    printk("hm5065_Snapshot \n");

    hm5065_read(sd, 0x06F0, &value1);	//700,af_pos_h
    hm5065_read(sd, 0x06F1, &value2);	//701,af_pos_l
    printk("capture brant read target af pos: %02x %02x.\n", value1, value2);

	err = hm5065_write_regs(sd, hm5065_regs_capture1, hm5065_regs_capture1_size);
    
    hm5065_i2c_reg(sd, 0x0734, value1 & 0xFF);	//af_pos_h
    hm5065_i2c_reg(sd, 0x0735, value2 & 0xFF);	//af_pos_L

    hm5065_read(sd, 0x0700, &value1);
    hm5065_read(sd, 0x0701, &value2);
    printk("capture brant read target af pos 2: %02x %02x.\n", value1, value2);

    err = hm5065_write_regs(sd, hm5065_regs_capture2, hm5065_regs_capture2_size);

	//hm5065_check_exp(sd);
	//hm5065_setting_update(sd, 1);

	if(err < 0)
	{
		return -EIO;
	}
	else
	{
		return 0;
	}
}


static int hm5065_get_sensor_info(struct v4l2_subdev *sd)
{
		unsigned char val[2];
		val[0] = 0;
		val[1] = 0;
        u16 sensor_id = 0;
		hm5065_read(sd,(0x0000),&val[0]);
		hm5065_read(sd,(0x0001),&val[1]);
        sensor_id = val[0] << 8 | val[1];
		printk("hm5065 chip id is %x\n", sensor_id);

        if (HM5065_SENSOR_ID != sensor_id)
        {
            printk("chip id is wrong\n");
            return -1;
        }

        return 0;
}



static const char *hm5065_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};

static const char *hm5065_querymenu_effect_mode[] = {
	"Effect Normal","Effect Sepia", "Effect Aqua", "Effect Monochrome",
	"Effect Negative", "Effect Sketch", NULL
};

static const char *hm5065_querymenu_ev_bias_mode[] = {
	"-3EV",	"-2,1/2EV", "-2EV", "-1,1/2EV",
	"-1EV", "-1/2EV", "0", "1/2EV",
	"1EV", "1,1/2EV", "2EV", "2,1/2EV",
	"3EV", NULL
};

static struct v4l2_queryctrl hm5065_controls[] = {
	{
		/*
		 * For now, we just support in preset type
		 * to be close to generic WB system,
		 * we define color temp range for each preset
		 */
		.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "White balance in kelvin",
		.minimum = 0,
		.maximum = 10000,
		.step = 1,
		.default_value = 0,	/* FIXME */
	},
	{
		.id = V4L2_CID_CAMERA_WHITE_BALANCE,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(hm5065_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_AUTO_WHITE_BALANCE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Auto white balance",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Exposure bias",
		.minimum = 0,
		.maximum = ARRAY_SIZE(hm5065_querymenu_ev_bias_mode) - 2,
		.step = 1,
		.default_value = \
			(ARRAY_SIZE(hm5065_querymenu_ev_bias_mode) - 2) / 2,
			/* 0 EV */
	},
	{
		.id = V4L2_CID_CAMERA_EFFECT,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Image Effect",
		.minimum = 0,
		.maximum = ARRAY_SIZE(hm5065_querymenu_effect_mode) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_CAMERA_CONTRAST,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Contrast",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_BRIGHTNESS,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Brightness",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},	
	{
		.id = V4L2_CID_CAMERA_SATURATION,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Saturation",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_SHARPNESS,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Sharpness",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_ZOOM,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Zoom",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 0,
	},

	{
		.id = V4L2_CID_CAMERA_FOCUS_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "AF",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 0,
	}
};

const char **hm5065_ctrl_get_menu(u32 id)
{
	switch (id) {
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return hm5065_querymenu_wb_preset;

	case V4L2_CID_COLORFX:
		return hm5065_querymenu_effect_mode;

	case V4L2_CID_EXPOSURE:
		return hm5065_querymenu_ev_bias_mode;

	default:
		return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *hm5065_find_qctrl(int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hm5065_controls); i++)
		if (hm5065_controls[i].id == id)
			return &hm5065_controls[i];

	return NULL;
}

static int hm5065_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hm5065_controls); i++) {
		if (hm5065_controls[i].id == qc->id) {
			memcpy(qc, &hm5065_controls[i], \
				sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int hm5065_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;

	qctrl.id = qm->id;
	hm5065_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, hm5065_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int hm5065_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	return err;
}

static int hm5065_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
    int index;
    char pf_str[20];
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct v4l2_pix_format *pix = &fmt->fmt.pix;

    //find the required pixelformat in altek8sa05_formats[]
    //
    //Round requested image size down to the nearest
    //we support, but not below the smallest.
    for (index = 0; index < N_HM5065_FMTS; index++){
    	if (hm5065_formats[index].pixelformat == pix->pixelformat){
    		if (pix->width >= hm5065_formats[index].width &&
    				pix->height >= hm5065_formats[index].height){
    			//info->fmt = &(hm5065_formats[index]);	
    			break;
    		}
    	}
    }

    if (index >= N_HM5065_FMTS){
        memcpy(pf_str,(char*)&(fmt->fmt.pix.pixelformat),4);
        pf_str[4] = '\0';
        dev_dbg_cam(&client->dev, "##!!##%s called -end:type:%d,width:%d,height:%d,pixelformat:%s,bytesperline:%d,sizeimage:%d\n"
        		,__func__
        		,fmt->type
        		,fmt->fmt.pix.width
        		,fmt->fmt.pix.height
        		,pf_str
        		,fmt->fmt.pix.bytesperline
        		,fmt->fmt.pix.sizeimage);        
    	dev_dbg_cam(&client->dev,"##!!## %s error!!!\n",__func__);
    	return -EINVAL;
    }
    else{
        pix->width = hm5065_formats[index].resolution_width;
        pix->height = hm5065_formats[index].resolution_height;
    }
        
    memcpy(pf_str,(char*)&(fmt->fmt.pix.pixelformat),4);
    pf_str[4] = '\0';
    dev_dbg_cam(&client->dev, "##!!##%s called -end:type:%d,width:%d,height:%d,pixelformat:%s,bytesperline:%d,sizeimage:%d\n"
    		,__func__
    		,fmt->type
    		,fmt->fmt.pix.width
    		,fmt->fmt.pix.height
    		,pf_str
    		,fmt->fmt.pix.bytesperline
    		,fmt->fmt.pix.sizeimage);
    
    return 0;
}

static int hm5065_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
    int err = 0;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct hm5065_state *state = to_state(sd);
    char pf_str[20];
    int index;
    struct v4l2_pix_format *pix = &fmt->fmt.pix;

    for (index = 0; index < N_HM5065_FMTS; index++){
	printk("index = %d, format %x, width =%d h = %d\n", index, 
		hm5065_formats[index].pixelformat, hm5065_formats[index].width, hm5065_formats[index].height);

    	if (hm5065_formats[index].pixelformat == pix->pixelformat){
    		if (pix->width >= hm5065_formats[index].width &&
    				pix->height >= hm5065_formats[index].height){
    			//info->fmt = &(hm5065_formats[index]);	
    			break;
    		}
    	}
    }

    if (index >= N_HM5065_FMTS){
	//WARN_ON(1);
    	dev_dbg_cam(&client->dev,"##!!## %s %x  h = %d w = %d error!!!\n",__func__,pix->pixelformat, pix->height, pix->width);
    	//return -EINVAL;
    }
   
    memcpy(pf_str,(char*)&(fmt->fmt.pix.pixelformat),4);
    pf_str[4] = '\0';
    dev_dbg_cam(&client->dev, "##!!##%s called:type:%d,width:%d,height:%d,pixelformat:%s,bytesperline:%d,sizeimage:%d\n"
    		,__func__
    		,fmt->type
    		,fmt->fmt.pix.width
    		,fmt->fmt.pix.height
    		,pf_str
    		,fmt->fmt.pix.bytesperline
    		,fmt->fmt.pix.sizeimage);


    dev_dbg_cam(&client->dev, "%s: Set Resolution Size index:%d\n", __func__, hm5065_formats[index].index);
    //err = hm5065_write_regs(sd, (unsigned char *) hm5065_regs_resolution[hm5065_formats[index].index], hm5065_regs_resolution_size[hm5065_formats[index].index]);

    //mdelay(300);

    if(hm5065_formats[index].resolution_width==HM5065_CAPTURE_WIDTH&&
        hm5065_formats[index].resolution_height==HM5065_CAPTURE_HEIGHT)
    {
        //state->framesize_index = HM5065_CAPTRUE_UXGA;
        state->framesize_index = HM5065_CAPTRUE_5M;
        dev_dbg_cam(&client->dev, "%s: Set framesize_index:%d\n", __func__, state->framesize_index);
    }
    else if(hm5065_formats[index].resolution_width==1600&&
        hm5065_formats[index].resolution_height==1200)
    {
        //state->framesize_index = HM5065_CAPTRUE_UXGA;
        state->framesize_index = HM5065_CAPTRUE_2M;
        dev_dbg_cam(&client->dev, "%s: Set framesize_index:%d\n", __func__, state->framesize_index);
    }
    else if(hm5065_formats[index].resolution_width==1024&&
        hm5065_formats[index].resolution_height==768)
    {
        //state->framesize_index = HM5065_CAPTRUE_UXGA;
        state->framesize_index = HM5065_CAPTRUE_1M;
        dev_dbg_cam(&client->dev, "%s: Set framesize_index:%d\n", __func__, state->framesize_index);
    }
    else
    {
        state->framesize_index = HM5065_PREVIEW_VGA;
        dev_dbg_cam(&client->dev, "%s: Set framesize_index:%d\n", __func__, state->framesize_index);
    }    
    
    return err;
}
static int hm5065_enum_framesizes(struct v4l2_subdev *sd, \
					struct v4l2_frmsizeenum *fsize)
{
	struct hm5065_state *state = to_state(sd);
	int num_entries = sizeof(hm5065_framesize_list) /
				sizeof(struct hm5065_enum_framesize);
	struct hm5065_enum_framesize *elem;
	int index = 0;
	int i = 0;


	/* The camera interface should read this value, this is the resolution
	 * at which the sensor would provide framedata to the camera i/f
	 *
	 * In case of image capture,
	 * this returns the default camera resolution (WVGA)
	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	index = state->framesize_index;

	for (i = 0; i < num_entries; i++) {
		elem = &hm5065_framesize_list[i];
		if (elem->index == index) {
			fsize->discrete.width =
			    hm5065_framesize_list[index].width;
			fsize->discrete.height =
			    hm5065_framesize_list[index].height;
			return 0;
		}
	}

	return -EINVAL;
}

static int hm5065_enum_frameintervals(struct v4l2_subdev *sd,
					struct v4l2_frmivalenum *fival)
{
	int err = 0;

	return err;
}

static int hm5065_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int err = 0;

	return err;
}

static int hm5065_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
    int index;
    char pf_str[20];
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct v4l2_pix_format *pix = &fmt->fmt.pix;

    //find the required pixelformat in altek8sa05_formats[]
    //
    //Round requested image size down to the nearest
    //we support, but not below the smallest.
    for (index = 0; index < N_HM5065_FMTS; index++){
    	if (hm5065_formats[index].pixelformat == pix->pixelformat){
    		if (pix->width >= hm5065_formats[index].width &&
    				pix->height >= hm5065_formats[index].height){
    			//info->fmt = &(hm5065_formats[index]);	
    			break;
    		}
    	}
    }

    if (index >= N_HM5065_FMTS){
    	dev_dbg_cam(&client->dev,"##!!## %s %x error!!!\n",__func__,pix->pixelformat);
    	return -EINVAL;
    }
    memcpy(pf_str,(char*)&(fmt->fmt.pix.pixelformat),4);
    pf_str[4] = '\0';
    dev_dbg_cam(&client->dev, "##!!##%s called -end:type:%d,width:%d,height:%d,pixelformat:%s,bytesperline:%d,sizeimage:%d\n"
    		,__func__
    		,fmt->type
    		,fmt->fmt.pix.width
    		,fmt->fmt.pix.height
    		,pf_str
    		,fmt->fmt.pix.bytesperline
    		,fmt->fmt.pix.sizeimage);
    
    return 0;
}

static int hm5065_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
    int err = 0;

  /*  
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    dev_dbg_cam(&client->dev, "%s: param->type(%x)\n", __func__, param->type);
    switch (param->type) {
        case V4L2_BUF_TYPE_PRIVATE:
        {
            int *orientation = (int *)&param->parm.raw_data;
    	    *orientation = 0;//90;  //TH, change to 0 degree, for video call orientation  
        }    
    	break;
        default:
    	    dev_err(&client->dev, "%s: no such parm\n", __func__);
    	break;
    }
  */  
    return err;
}

static int hm5065_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	struct sec_cam_parm *m_params =
		(struct sec_cam_parm *)&param->parm.raw_data;
	struct v4l2_control ctrl;
	if( m_params->contrast != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_CONTRAST;
		ctrl.value = m_params->contrast;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_CONTRAST %d fail\n", ctrl.value);
			goto out;
		}
	}
	
	if(m_params->effects != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_EFFECT;
		ctrl.value = m_params->effects;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_EFFECT %d fail\n", ctrl.value);
			goto out;
		}
	}


	if(m_params->brightness != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_BRIGHTNESS;
		ctrl.value = m_params->brightness;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_BRIGHTNESS %d fail\n", ctrl.value);
			goto out;
		}
	}

 	if(m_params->flash_mode != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_FLASH_MODE;
		ctrl.value = m_params->flash_mode;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_FLASH_MODE %d fail\n", ctrl.value);
			goto out;
		}
	}

 	if(m_params->focus_mode != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_FOCUS_MODE;
		ctrl.value = m_params->focus_mode;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_FOCUS_MODE %d fail\n", ctrl.value);
			goto out;
		}
	}
	
 	if( m_params->iso != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_ISO;
		ctrl.value =  m_params->iso;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_ISO %d fail\n", ctrl.value);
			goto out;
		}	
	}

 	if(m_params->metering != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_METERING;
		ctrl.value = m_params->metering;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_METERING %d fail\n", ctrl.value);
			goto out;
		}	
	}

 	 if(m_params->saturation != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_SATURATION;
		ctrl.value = m_params->saturation;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_SATURATION %d fail\n", ctrl.value);
			goto out;
		}	
	}

 	 if(m_params->scene_mode != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_SCENE_MODE;
		ctrl.value = m_params->scene_mode;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_SCENE_MODE %d fail\n", ctrl.value);
			goto out;
		}	
	}

 	 if(m_params->sharpness != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_SHARPNESS;
		ctrl.value = m_params->sharpness;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_SHARPNESS %d fail\n", ctrl.value);
			goto out;
		}	
	}

 	 if(m_params->white_balance != -1)
	{
		ctrl.id = V4L2_CID_CAMERA_WHITE_BALANCE;
		ctrl.value = m_params->white_balance;
		err = hm5065_s_ctrl(sd, &ctrl);
		if(err < 0)
		{
			printk("set V4L2_CID_CAMERA_WHITE_BALANCE %d fail\n", ctrl.value);
			goto out;
		}	
	}

	return 0;
out:
	dev_dbg_cam(&client->dev, "%s: return  fail\n", __func__);
	return err;

}

static int focus_state;
static int hm5065_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm5065_state *state = to_state(sd);
	int err = 0;

        dev_dbg(&client->dev, "%s: ctrl->id(%x)\n", __func__, ctrl->id);
        
	switch (ctrl->id) {
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		ctrl->value = state->userset.auto_wb;
		break;
	case V4L2_CID_WHITE_BALANCE_PRESET:
		ctrl->value = state->userset.manual_wb;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = state->userset.effect;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = state->userset.contrast;
		break;
	case V4L2_CID_CAMERA_SATURATION:
		ctrl->value = state->userset.saturation;
		break;
	case V4L2_CID_CAMERA_SHARPNESS:
		ctrl->value = state->userset.sharpness;
		break;
	case V4L2_CID_CAMERA_GET_FLASH_ONOFF:
		ctrl->value = state->userset.flash_lastStatus;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
	case V4L2_CID_CAM_JPEG_MEMSIZE:
	case V4L2_CID_CAM_JPEG_QUALITY:
	case V4L2_CID_CAM_DATE_INFO_YEAR:
	case V4L2_CID_CAM_DATE_INFO_MONTH:
	case V4L2_CID_CAM_DATE_INFO_DATE:
	case V4L2_CID_CAM_SENSOR_VER:
	case V4L2_CID_CAM_FW_MINOR_VER:
	case V4L2_CID_CAM_FW_MAJOR_VER:
	case V4L2_CID_CAM_PRM_MINOR_VER:
	case V4L2_CID_CAM_PRM_MAJOR_VER:
	case V4L2_CID_ESD_INT:
	case V4L2_CID_CAMERA_GET_ISO:
	case V4L2_CID_CAMERA_GET_SHT_TIME:
	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
	case V4L2_CID_CAMERA_SMART_AUTO_STATUS:        
		ctrl->value = 0;
		break;
	//case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
       //ctrl->value = focus_state;
       //break;
	case V4L2_CID_EXPOSURE:
		ctrl->value = state->userset.exposure;
		break;
#if 0
    case V4L2_CID_CAMERA_RETURN_SENSOR_ORIENTATION:
		ctrl->value = 270;
		break;       
#endif
	default:
		dev_err(&client->dev, "%s: no such ctrl\n", __func__);
		break;
	}

	return err;
}

/*static void listCameraReg(struct v4l2_subdev *sd)
{
	unsigned char value = 0;
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	//struct hm5065_state *state = to_state(sd);
	
	unsigned short i;
	//list light mode
	for(i = 0x3400; i <= 0x3406; i++)
	{
		hm5065_read(sd, i, &value);
		printk("reg %x value %x\n", i, value);
	}
}*/

static int hm5065_checkAufoFlash(struct v4l2_subdev *sd)
{
#if 0
	unsigned char value1 = 0, value2 = 0;
    u16 value = 0;
	struct hm5065_state *state = to_state(sd);

	hm5065_read(sd, 0x0118, &value1);
    hm5065_read(sd, 0x0119, &value2);
    value = ((value1 << 8) & 0xFF00) | (value2 & 0x00FF);
	printk("%s, avg value is 0x%x\n", __func__, value);

	
	if(state->userset.flash_mode == FLASH_MODE_AUTO)
	{
	
		mi108_set_flash_status(0);
		if(value > 0x4880)
		{
			setlightOn(0);
			state->userset.flash_lastStatus = 0;
		}
		else if(value <= 0x4880)
		{
			setlightOn(1);
			state->userset.flash_lastStatus = 1;
		}


	}
#endif	
	return 0;
}



static int hm5065_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm5065_state *state = to_state(sd);
	int err = 0;
    int index=0;

    printk("hm5065_s_ctrl %x, %d\n", ctrl->id, ctrl->value);
    
	switch (ctrl->id) {
	case V4L2_CID_CAMERA_ISO:
	case V4L2_CID_CAMERA_METERING:    
	case V4L2_CID_CAMERA_WDR:
	case V4L2_CID_CAMERA_FACE_DETECTION:
	case V4L2_CID_CAM_JPEG_QUALITY:
	case V4L2_CID_CAMERA_GPS_LATITUDE:
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
	case V4L2_CID_CAMERA_FRAME_RATE:       
	case V4L2_CID_CAMERA_CHECK_DATALINE:
	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:        
	case V4L2_CID_CAMERA_RETURN_FOCUS:
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		{
#if 1
			cancel_delayed_work(&state->work);
			state->userset.flash_mode = ctrl->value;
			switch(ctrl->value)
			{
				case FLASH_MODE_OFF:
					printk("FLASH_MODE_OFF");
					hm5065_flash_control(sd,0);
					//mi108_set_flash_status(0);
					//setlightOn(0);
					state->userset.flash_lastStatus = 0;
					break;
				case FLASH_MODE_AUTO:
					printk("FLASH_MODE_AUTO");
					//mi108_set_flash_status(1);
					schedule_delayed_work(&state->work, 500);
					break;
				case FLASH_MODE_ON:
					printk("FLASH_MODE_ON");
					//mi108_set_flash_status(1);
					hm5065_flash_control(sd,1);
					//mi108_set_flash_status(0);					
					//setlightOn(1);
					state->userset.flash_lastStatus = 1;
				 	break;
				case FLASH_MODE_TORCH:
					printk("FLASH_MODE_TORCH");
					//mi108_set_flash_status(0);					
					//setlightOn(1);
					state->userset.flash_lastStatus = 1;
					break;
			}
#endif
	}
		break;
	case V4L2_CID_CAMERA_SCENE_MODE:
		//do not support
		break;
	case V4L2_CID_CAM_PREVIEW_ONOFF:
        dev_dbg_cam(&client->dev, "%s: V4L2_CID_CAM_PREVIEW_ONOFF, value:%d, state->framesize_index:%d\n", \
			__func__,ctrl->value,state->framesize_index);
		cancel_delayed_work(&state->work);
		//if(state->framesize_index == HM5065_CAPTRUE_UXGA)
		if(state->framesize_index == HM5065_CAPTRUE_5M)
		
		{
			err = hm5065_Snapshot(sd);
		}
		else
		{
			err = hm5065_preview(sd);

		}
		schedule_delayed_work(&state->work, 2000);
		break;     
	case V4L2_CID_CAMERA_RESET:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_RESET\n", __func__);
		err = hm5065_reset(sd);
		break;        
	case V4L2_CID_EXPOSURE:
        state->userset.exposure = ctrl->value;
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_WHITE_BALANCE_PRESET, value:%d\n", \
			__func__,ctrl->value);
		switch(ctrl->value)
		{
			case WHITE_BALANCE_AUTO:
			default:
                index = HM5065_AWB_MODE_AUTO_SIMPLE;
				hm5065_i2c_reg(sd, 0x01A0,0x01);
				break;
			case WHITE_BALANCE_SUNNY:
                index = HM5065_AWB_MODE_MANUAL_DAY;
				hm5065_i2c_reg(sd, 0x01A0,0x03);//MWB
    			hm5065_i2c_reg(sd, 0x01A1,0xC0);//Rgain
    			hm5065_i2c_reg(sd, 0x01A2,0x40);//Ggain
    			hm5065_i2c_reg(sd, 0x01A3,0x00);//Bgain
				break;
			case WHITE_BALANCE_CLOUDY:
                index = HM5065_AWB_MODE_MANUAL_CLOUDY;
				hm5065_i2c_reg(sd, 0x01A0,0x03);//MWB
    			hm5065_i2c_reg(sd, 0x01A1,0xff);//Rgain
    			hm5065_i2c_reg(sd, 0x01A2,0x40);//Ggain
    			hm5065_i2c_reg(sd, 0x01A3,0x00);//Bgain
				break;
			case WHITE_BALANCE_TUNGSTEN:
                index = HM5065_AWB_MODE_MANUAL_A;
				hm5065_i2c_reg(sd, 0x01A0,0x03);//MWB
    			hm5065_i2c_reg(sd, 0x01A1,0x90);//Rgain
    			hm5065_i2c_reg(sd, 0x01A2,0x40);//Ggain
    			hm5065_i2c_reg(sd, 0x01A3,0x40);//Bgain
				break;
			case WHITE_BALANCE_FLUORESCENT:
                index = HM5065_AWB_MODE_MANUAL_CWF;
				hm5065_i2c_reg(sd, 0x01A0,0x03);//MWB
    			hm5065_i2c_reg(sd, 0x01A1,0xA0);//Rgain
    			hm5065_i2c_reg(sd, 0x01A2,0x40);//Ggain
    			hm5065_i2c_reg(sd, 0x01A3,0x30);//Bgain			
				break;
		}
        state->userset.index_wb = index;
		//schedule_delayed_work(&state->debugCheck, 2000);
    	break;

	case V4L2_CID_CAMERA_EFFECT:
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_CAMERA_EFFECT, value:%d,\n", __func__,ctrl->value);
        switch(ctrl->value)
        {
            case IMAGE_EFFECT_NONE:
                index = 0;
          hm5065_i2c_reg(sd, 0x0380,0x00);
    			hm5065_i2c_reg(sd, 0x0381,0x00);
    			hm5065_i2c_reg(sd, 0x0382,0x00);
    			hm5065_i2c_reg(sd, 0x0384,0x00);
                break;
				
            case IMAGE_EFFECT_BNW:
                index = 1;
          hm5065_i2c_reg(sd, 0x0380,0x00);
    			hm5065_i2c_reg(sd, 0x0381,0x00);
    			hm5065_i2c_reg(sd, 0x0382,0x00);
    			hm5065_i2c_reg(sd, 0x0384,0x05);
                break;
				
            case IMAGE_EFFECT_SEPIA:
                index = 3;
          hm5065_i2c_reg(sd, 0x0380,0x00);
    			hm5065_i2c_reg(sd, 0x0381,0x00);
    			hm5065_i2c_reg(sd, 0x0382,0x00);
    			hm5065_i2c_reg(sd, 0x0384,0x06);
                break;
          
            case IMAGE_EFFECT_NEGATIVE:
                index = 6;
          hm5065_i2c_reg(sd, 0x0380,0x01);
    			hm5065_i2c_reg(sd, 0x0381,0x00);
    			hm5065_i2c_reg(sd, 0x0382,0x00);
    			hm5065_i2c_reg(sd, 0x0384,0x00);
                break;
          
            default:
                dev_err(&client->dev, "%s: V4L2_CID_CAMERA_EFFECT, un-support effect, value:%d,\n", __func__,ctrl->value);
                goto out;
                break;
        }
       	state->userset.effect = index;
		break;

	case V4L2_CID_CAMERA_CONTRAST:
		state->userset.contrast = ctrl->value;
		break;
	case V4L2_CID_CAMERA_BRIGHTNESS:
	
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_CAMERA_BRIGHTNESS,value:%d,\n", __func__,ctrl->value);
		state->userset.brightness= ctrl->value;
		break;		

	case V4L2_CID_CAMERA_SATURATION:
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_SATURATION,value:%d,\n", __func__,ctrl->value);
		state->userset.saturation= ctrl->value;
		break;

	case V4L2_CID_CAMERA_SHARPNESS:
		state->userset.sharpness= ctrl->value;
		break;

	case V4L2_CID_CAMERA_ZOOM:
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_ZOOM_ABSOLUTE,value:%d,current zoom is:%d\n", __func__, ctrl->value, state->userset.zoom);
#if 0        
        if (state->userset.zoom == ctrl->value)
        {
            break;
        }
        else if (state->userset.zoom > ctrl->value)
        {
            int i = 0;
            for (i = 0; i < (state->userset.zoom - ctrl->value); i++)
            {
                hm5065_i2c_reg(sd, 0x0020, 0x04);
            }
        }
        else
        {
            int i = 0;
            for (i = 0; i < (ctrl->value - state->userset.zoom); i++)
            {
                hm5065_i2c_reg(sd, 0x0020, 0x03);
            }
        }
#endif        
		state->userset.zoom= ctrl->value;
		break; 

		
	case V4L2_CID_CAMERA_FOCUS_MODE:
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_CAMERA_FOCUS_MODE,value:%d,\n", __func__,ctrl->value);
		err = 0;
		{
			switch(ctrl->value)
			{
				case FOCUS_MODE_AUTO :
					focus_state = 0;
					state->userset.af_mode = FOCUS_MODE_AUTO;
					break;
				case FOCUS_MODE_INFINITY :
					focus_state = 1;
					state->userset.af_mode = FOCUS_MODE_INFINITY;
					break;
				case FOCUS_MODE_MACRO:
					focus_state = 1;
					state->userset.af_mode = FOCUS_MODE_MACRO;	
					break;
				default :
					dev_dbg_cam(&client->dev, "%s(unmatched af mode(%d) fail)", __func__, ctrl->value);
					break;							
			}
		}

		break;
		//case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
				//ctrl->value = focus_state;
        //dev_dbg_cam(&client->dev, "%s: V4L2_CID_CAMERA_AUTO_FOCUS_RESULT,value:%d,\n", __func__,ctrl->value);
		//break;
    case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		dev_dbg_cam(&client->dev, "%s: V4L2_CID_CAMERA_SET_AUTO_FOCUS,value:%d,\n", __func__,ctrl->value);
		err = 0;
		switch(ctrl->value)
		{
			case AUTO_FOCUS_ON:
			{
				
				if(state->userset.af_mode==FOCUS_MODE_AUTO)
				{
				    int i;
					unsigned char value = 0;
					focus_state = 0;
                    
                    hm5065_i2c_reg(sd, 0x070A,0x03);
                	mdelay(200);
                    hm5065_i2c_reg(sd, 0x070B,0x01);
                	mdelay(100);
                    hm5065_i2c_reg(sd, 0x070B,0x02);
                	mdelay(100);

                    do
					{
						msleep(10);
						err = hm5065_read(sd, 0x07AE, &value);
						if (err < 0)
                        {     
                            printk("read 0x07AE fail\n");
							goto out;
                        }
						i++;
					}while(value != 1 && i < 200);

                    hm5065_i2c_reg(sd, 0x070A,0x00);

                    if(value != 1)
					{
						focus_state = 2;
						printk("fail !!!!!!auto focus status %x\n", value);

					}
					else
					{
						printk("sucess !!!!!!!focus eara %x\n", value);
						focus_state = 1;

                        hm5065_read(sd, 0x06F0, &af_pos_h);
                        hm5065_read(sd, 0x06F1, &af_pos_l);
                		printk("brant read current af pos: %02x%02x.\n", af_pos_h, af_pos_l);
					}

                }

                
                
			}
			break;
			case AUTO_FOCUS_OFF:
				focus_state = 0;				
				break;				
			default :
				dev_dbg_cam(&client->dev, "%s(unmatched af mode(%d) fail)", __func__, ctrl->value);
				break;							
		}
		break;
	default:
		dev_err(&client->dev, "%s: no such control\n", __func__);
		break;
	}

	if (err < 0)
		goto out;
	else
		return 0;

out:
	dev_dbg_cam(&client->dev, "%s: vidioc_s_ctrl failed\n", __func__);
	return err;

}


static int hm5065_init(struct v4l2_subdev *sd, u32 val)
{
	int err = 0;
	struct hm5065_state *state = to_state(sd);

    err = hm5065_get_sensor_info(sd);

    if (err != 0)
    {
        goto exit;
    }

    err = hm5065_write_regs(sd, hm5065_init_setting, hm5065_init_setting_size);
    //err = hm5065_write_regs(sd, hm5065_regs_af_init, hm5065_regs_af_init_size);
    printk("write hm5065 init setting, ret is %d\n", err);

	state->isinitCall = 1;

exit:    
	return err;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize every single opening time
 * therefor, it is not necessary to be initialized on probe time.
 * except for version checking.
 * NOTE: version checking is optional
 */
static int hm5065_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm5065_state *state = to_state(sd);
	struct hm5065_platform_data *pdata;

	dev_info(&client->dev, "fetching platform data\n");

	pdata = client->dev.platform_data;

	if (!pdata) {
		dev_err(&client->dev, "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	if (!(pdata->default_width && pdata->default_height)) {
		/* TODO: assign driver default resolution */
	} else {
		state->pix.width = pdata->default_width;
		state->pix.height = pdata->default_height;
	}

	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = 24000000;	/* 24MHz default */
	else
		state->freq = pdata->freq;

	if (!pdata->is_mipi) {
		state->is_mipi = 0;
		dev_info(&client->dev, "parallel mode\n");
	} else
		state->is_mipi = pdata->is_mipi;

	return 0;
}

static const struct v4l2_subdev_core_ops hm5065_core_ops = 
{
	.init = hm5065_init,	/* initializing API */
	.s_config = hm5065_s_config,	/* Fetch platform data */
	.queryctrl = hm5065_queryctrl,
	.querymenu = hm5065_querymenu,
	.g_ctrl = hm5065_g_ctrl,
	.s_ctrl = hm5065_s_ctrl,
};

static const struct v4l2_subdev_video_ops hm5065_video_ops = 
{
	.s_crystal_freq = hm5065_s_crystal_freq,
	.g_fmt = hm5065_g_fmt,
	.s_fmt = hm5065_s_fmt,
	.enum_framesizes = hm5065_enum_framesizes,
	.enum_frameintervals = hm5065_enum_frameintervals,
	.enum_fmt = hm5065_enum_fmt,
	.try_fmt = hm5065_try_fmt,
	.g_parm = hm5065_g_parm,
	.s_parm = hm5065_s_parm,
};

static const struct v4l2_subdev_ops hm5065_ops = 
{
	.core = &hm5065_core_ops,
	.video = &hm5065_video_ops,
};


static void hm5065_detectled_func(struct work_struct *work)
{
	struct hm5065_state *state = container_of((struct delayed_work *)work,
			struct hm5065_state, work);
	hm5065_checkAufoFlash(&state->sd);
	//schedule_delayed_work(&state->work, 500);
}

/*static void hm5065_debugcheck(struct work_struct *work)
{
	struct hm5065_state *state = container_of((struct delayed_work *)work,
			struct hm5065_state, work);
	listCameraReg(&state->sd);
}*/


static int hm5065_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct hm5065_state *state = to_state(sd);	
	cancel_delayed_work(&state->work);
	//cancel_delayed_work(&state->debugCheck);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}


/*
 * hm5065_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
extern int mi108_cam1_hm5065_power(int onoff);
static int hm5065_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct hm5065_state *state;
	struct v4l2_subdev *sd;
	struct hm5065_platform_data *pdata = client->dev.platform_data;
	
	if ((pdata == NULL) || (pdata->flash_onoff == NULL)) {
		printk( "%s: bad platform data\n", __func__);
		return -ENODEV;
	}

	state = kzalloc(sizeof(struct hm5065_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

    printk("hm5065 probe enter\n");

	//default value
	state->userset.index_wb = 1;
	state->userset.effect = 0;
	state->userset.brightness= 4;


	INIT_DELAYED_WORK(&state->work, hm5065_detectled_func);
	//INIT_DELAYED_WORK(&state->debugCheck, hm5065_debugcheck);



	sd = &state->sd;
	strcpy(sd->name, HM5065_DRIVER_NAME);
    
	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &hm5065_ops);

#if 0
    mi108_cam1_hm5065_power(1);
    if (0 != hm5065_get_sensor_info(sd))
    {
        hm5065_remove(client);
        mi108_cam1_hm5065_power(0);
        return -ENODEV;
    }
#endif    

#ifdef TEST_CODE
    {
        struct device* HM5065RegDev;
        
        if (!HM5065Class)
    	{
    		HM5065Class = class_create(THIS_MODULE, "hm5065cam");
    		if (IS_ERR(HM5065Class)) {
    			dev_dbg_cam(&client->dev, "%s, couldn't create sysfs class(%ld)\n", __func__, IS_ERR(HM5065Class));
    		}
    	}

        if (HM5065Class)
        {
        	HM5065RegDev = device_create(HM5065Class,NULL,0,NULL,"dev");
        	if (IS_ERR(HM5065RegDev)) {
        		dev_dbg_cam(&client->dev, "%s, Failed to create device(HM5065Reg)!= %ld\n", __func__, IS_ERR(HM5065RegDev));
        	}
            else
            {
                if (device_create_file(HM5065RegDev , &dev_attr_reg) < 0) {
            		dev_dbg_cam(&client->dev, "%s, Failed to create device file(%s)!\n", __func__, dev_attr_reg.attr.name);	
            	}
            }
        }
        
        g_sd = sd;
    }
#endif
    
	dev_info(&client->dev, "hm5065 has been probed\n");
	return 0;
}


static const struct i2c_device_id hm5065_id[] = 
{
	{ HM5065_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, hm5065_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = 
{
	.name = HM5065_DRIVER_NAME,
	.probe = hm5065_probe,
	.remove = hm5065_remove,
	.id_table = hm5065_id,
};

MODULE_DESCRIPTION("hm5065 camera driver");
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
