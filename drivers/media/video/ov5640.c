#include <linux/module.h>
#include <linux/errno.h> 
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h> 
#include <linux/time.h>
#include <mach/gpio.h>
#include <linux/clk.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
//#include <media/sekede_camera.h>
//#include <mach/gpio-skd4x12.h>
#include "ov5640.h"

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#define OV5640_DRIVER_NAME "S5K4ECGX"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct ov5640_mode_info ov5640_mode_info_data[2][ov5640_mode_MAX + 1] = {
	{
		{ov5640_mode_VGA_640_480, 0, 0, NULL, 0},
		{ov5640_mode_QVGA_320_240, 0, 0, NULL, 0},
		{ov5640_mode_NTSC_720_480, 0, 0, NULL, 0},
		{ov5640_mode_PAL_720_576, 0, 0, NULL, 0},
		{ov5640_mode_SVGA_800_600, 0, 0, NULL, 0},
		{ov5640_mode_720P_1280_720, 0, 0, NULL, 0},
		{ov5640_mode_1080P_1920_1080, 1920, 1080, ov5640_setting_15fps_1080P_1920_1080, ARRAY_SIZE(ov5640_setting_15fps_1080P_1920_1080)},
		{ov5640_mode_QSXGA_2592_1944, 2592, 1944, ov5640_setting_15fps_QSXGA_2592_1944, ARRAY_SIZE(ov5640_setting_15fps_QSXGA_2592_1944)},
	},
	{
		{ov5640_mode_VGA_640_480, 640, 480, ov5640_setting_30fps_VGA_640_480, ARRAY_SIZE(ov5640_setting_30fps_VGA_640_480)},
		{ov5640_mode_QVGA_320_240, 320, 240, ov5640_setting_30fps_QVGA_320_240, ARRAY_SIZE(ov5640_setting_30fps_QVGA_320_240)},
		{ov5640_mode_NTSC_720_480, 720, 480, ov5640_setting_30fps_NTSC_720_480, ARRAY_SIZE(ov5640_setting_30fps_NTSC_720_480)},
		{ov5640_mode_PAL_720_576, 720, 576, ov5640_setting_30fps_PAL_720_576, ARRAY_SIZE(ov5640_setting_30fps_PAL_720_576)},
		{ov5640_mode_SVGA_800_600, 800, 600, ov5640_setting_30fps_SVGA_800_600, ARRAY_SIZE(ov5640_setting_30fps_SVGA_800_600)},
		{ov5640_mode_720P_1280_720, 1280, 720, ov5640_setting_30fps_720P_1280_720, ARRAY_SIZE(ov5640_setting_30fps_720P_1280_720)},
    {ov5640_mode_1080P_1920_1080, 1920, 1080, ov5640_setting_15fps_1080P_1920_1080, ARRAY_SIZE(ov5640_setting_15fps_1080P_1920_1080)},
		{ov5640_mode_QSXGA_2592_1944, 2592, 1944, ov5640_setting_15fps_QSXGA_2592_1944, ARRAY_SIZE(ov5640_setting_15fps_QSXGA_2592_1944)},
		{ov5640_mode_setting_init,   640,  480,  ov5640_setting_init,ARRAY_SIZE(ov5640_setting_init)},
	},
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline struct ov5640_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct ov5640_state, sd);
}

//¶Á¼Ä´æÆ÷Öµ
static int ov5640_reg_read(struct i2c_client *client, unsigned short reg, unsigned char *val)
{
	int ret;
	unsigned char data[2] = {0};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= data,
	};

	data[0] = (unsigned char)(reg >> 8);
	data[1] = (unsigned char)(reg & 0xff);

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	msg.flags = I2C_M_RD;
	msg.len = 1;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	*val = data[0];

//printk("%s 0x%04x = 0x%02x\n", __FUNCTION__, reg, data[0]);
	return 0;

err:
	dev_err(&client->dev, "Failed reading register 0x%02x!\n", reg);
	return ret;
}

//Ð´¼Ä´æÆ÷Öµ
static int ov5640_reg_write(struct i2c_client *client, unsigned short reg, unsigned char val)
{
	int ret;
	unsigned char data[3] = { (unsigned char)(reg >> 8), (unsigned char)(reg & 0xff), val };
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

//printk("%s 0x%04x = 0x%02x\n", __func__, reg, val);
	return 0;
}

//¶Á¼Ä´æÆ÷Öµ£¬µ÷ÊÔÊ¹ÓÃ
static void dump_reg(struct i2c_client *client, unsigned short reg_start, unsigned short reg_end)
{
  unsigned short i = 0;
  unsigned char buf;
  printk("%s:\n", __func__);
  for(i=reg_start; i<=reg_end; i++)
  {
    ov5640_reg_read(client, i, &buf);
    printk("0x%04x = 0x%02x\n", i, buf);
  }
};

//¼ì²âOV5640 sensor
static int ov5640_detect(struct i2c_client *client)
{
  unsigned char id_check[2] = {0};
  
//  OV5640_power(0);
  msleep(10);
//  OV5640_power(1);
	ov5640_reg_read(client, 0x300A, &id_check[0]);
	ov5640_reg_read(client, 0x300B, &id_check[1]);

  if((id_check[0] != 0x56) || (id_check[1] != 0x40)){
//  	OV5640_power(0);
  	return -1;
  	}
	dev_info(&client->dev, "Detected a OV5640 chip\n");

  return 0;
}

//³õÊ¼»¯¼Ä´æÆ÷
static int ov5640_init_mode(struct v4l2_subdev *sd, enum ov5640_frame_rate frame_rate, enum ov5640_mode mode)
{
	struct ov5640_state *ov5640_state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct reg_value *pModeSetting = NULL;
	s32 i = 0;
	s32 iModeSettingArySize = 0;
	register u32 Delay_ms = 0;
	register u16 RegAddr = 0;
	register u8 Mask = 0;
	register u8 Val = 0;
	u8 RegVal = 0;
	int retval = 0;

	printk("%s ===================\n",__func__);
	if(mode > ov5640_mode_MAX || mode < ov5640_mode_MIN)
	{
		pr_err("Wrong ov5640 mode detected!\n");
		return -1;
	}

	pModeSetting = ov5640_mode_info_data[frame_rate][mode].init_data_ptr;
	iModeSettingArySize = ov5640_mode_info_data[frame_rate][mode].init_data_size;

	ov5640_state->pix.width = ov5640_mode_info_data[frame_rate][mode].width;
	ov5640_state->pix.height = ov5640_mode_info_data[frame_rate][mode].height;

	if(ov5640_state->pix.width == 0 || ov5640_state->pix.height == 0 || pModeSetting == NULL || iModeSettingArySize == 0)
		return -EINVAL;

	for(i=0; i<iModeSettingArySize; ++i, ++pModeSetting)
  {
		Delay_ms = pModeSetting->u32Delay_ms;
		RegAddr = pModeSetting->u16RegAddr;
		Val = pModeSetting->u8Val;
		Mask = pModeSetting->u8Mask;

		if(Mask)
		{
			retval = ov5640_reg_read(client, RegAddr, &RegVal);
			if (retval < 0)
				goto err;

			RegVal &= ~(u8)Mask;
			Val &= Mask;
			Val |= RegVal;
		}

		retval = ov5640_reg_write(client, RegAddr, Val);
		if (retval < 0)
			goto err;
		if (Delay_ms)
			msleep(Delay_ms);
	}
	ov5640_state->frame_rate = frame_rate;
	ov5640_state->mode = mode;

err:
	return retval;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
static int ov5640_init(struct v4l2_subdev *sd, u32 val)
{
	int ret = 0;
	struct ov5640_state *ov5640_state = to_state(sd);
//	struct i2c_client *client = v4l2_get_subdevdata(sd);
//printk("%s\n", __func__);
//fighter++
struct i2c_client *client = v4l2_get_subdevdata(sd);
	ret = ov5640_detect(client);
	if (ret) {
		dev_info(&client->dev, "ov5640 sensor not found\n");
		return -1;
	}
//fighter--
//	ov5640_state->frame_rate = DEFAULT_FRAME_RATE;
//	ov5640_state->mode = DEFAULT_VIDEO_MODE;
	ov5640_state->pix.pixelformat = V4L2_PIX_FMT_YUYV;
	ov5640_state->pix.width = ov5640_mode_info_data[DEFAULT_FRAME_RATE][DEFAULT_VIDEO_MODE].width;
	ov5640_state->pix.height = ov5640_mode_info_data[DEFAULT_FRAME_RATE][DEFAULT_VIDEO_MODE].height;;

  ov5640_init_mode(sd, DEFAULT_FRAME_RATE, DEFAULT_VIDEO_MODE);

 // dump_reg(client, 0x3035, 0x3039);
	return 0;
}

static int ov5640_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	int err = 0;
printk("%s\n", __func__);
	return err;
}

static int ov5640_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int err = 0;
printk("%s\n", __func__);
	return err;
}

static int ov5640_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err = 0;
	printk("%s,%d\n", __func__,ctrl->id-V4L2_CID_PRIVATE_BASE);
	 switch(ctrl->id)
  {
//  	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
//  		ctrl->value = 1;
//  		break;
//  	case V4L2_CID_CAMERA_GET_FLASH_ONOFF:
//			printk("V4L2_CID_CAMERA_GET_FLASH_ONOFF++++++++++++++++++\n");
//			break;
//  	case V4L2_CID_CAMERA_RETURN_FOCUS:
//    	printk("V4L2_CID_CAMERA_RETURN_FOCUS++++++++++++++++++\n");	
//    	break;
    case V4L2_CID_CAMERA_CHECK_DATALINE:
    case V4L2_CID_CAM_PREVIEW_ONOFF:

//    case V4L2_CID_ESD_INT:
//  
  		
		  return 0;
		default:
		  break;
  }
	return 0;
	return err;
}
static int32_t ov5640_set__focus(struct i2c_client *client)
{     
	int8_t buf=0;
	int32_t  rc = 0;
	ov5640_reg_write(client,0x3023, 0x01);
	ov5640_reg_write(client,0x3022, 0x03);		
	return  rc;
}
static int ov5640_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
printk("%s\n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		printk("%s: V4L2_CID_CAMERA_SET_AUTO_FOCUS %d\n", __func__, ctrl->id-V4L2_CID_PRIVATE_BASE );
		ov5640_set__focus(client);
		return 0;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		printk("%s: V4L2_CID_CAMERA_FLASH_MODE %d\n", __func__, ctrl->id-V4L2_CID_PRIVATE_BASE );
		return 0;
		break;
	case V4L2_CID_CAMERA_BRIGHTNESS:
			printk("ctrl->value = %d +++++++++++\n",ctrl->value);
		switch(ctrl->id){
		
			case 8:
				//brightness +2
				ov5640_reg_write(client, 0x5001, 0xff);
				ov5640_reg_write(client, 0x5587, 0x20);
				ov5640_reg_write(client, 0x5580, 0x04);
				ov5640_reg_write(client, 0x5588, 0x01);
				break;
			case 6:
				//brightness +1
				ov5640_reg_write(client, 0x5001, 0xff);
				ov5640_reg_write(client, 0x5587, 0x10);
				ov5640_reg_write(client, 0x5580, 0x04);
				ov5640_reg_write(client, 0x5588, 0x01);
				break;
			case 4:
				//brightness 0
				ov5640_reg_write(client, 0x5001, 0xff);
				ov5640_reg_write(client, 0x5587, 0x00);
				ov5640_reg_write(client, 0x5580, 0x04);
				ov5640_reg_write(client, 0x5588, 0x01);
				break;
			case 2:
				//brightness -1
				ov5640_reg_write(client, 0x5001, 0xff);
				ov5640_reg_write(client, 0x5587, 0x10);
				ov5640_reg_write(client, 0x5580, 0x04);
				ov5640_reg_write(client, 0x5588, 0x09);				
				break;
			case 0:
				//brightness -2
				ov5640_reg_write(client, 0x5001, 0xff);
				ov5640_reg_write(client, 0x5587, 0x20);
				ov5640_reg_write(client, 0x5580, 0x04);
				ov5640_reg_write(client, 0x5588, 0x09);				
				break;
	
						}
		break;
	case V4L2_CID_CAMERA_WDR:
	case V4L2_CID_CAMERA_FACE_DETECTION:
	case V4L2_CID_CAMERA_FOCUS_MODE:
	case V4L2_CID_CAM_JPEG_QUALITY:
	case V4L2_CID_CAMERA_SCENE_MODE:
	case V4L2_CID_CAMERA_GPS_LATITUDE:
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
	case V4L2_CID_CAMERA_FRAME_RATE:
	case V4L2_CID_CAMERA_ISO:
	case V4L2_CID_CAMERA_METERING:

	case V4L2_CID_CAMERA_CHECK_DATALINE:
	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
//	case V4L2_CID_CAMERA_RETURN_FOCUS:
  case V4L2_CID_CAM_PREVIEW_ONOFF:
		break;
	default:
printk("%s: no such control %d\n", __func__, ctrl->id-V4L2_CID_PRIVATE_BASE );
		return -EPERM;
	}

printk("%s: no such control process %d\n", __func__, ctrl->id-V4L2_CID_PRIVATE_BASE );
	return 0;
}

static int ov5640_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
printk("%s\n", __func__);
	return err;
}
static int ov5640_resolution(struct v4l2_subdev *sd, struct ov5640_state *ov5640_state)
{
	printk("update resolution %d %d\n", ov5640_state->pix.width, ov5640_state->pix.height);

	if ((640 == ov5640_state->pix.width) && (480 == ov5640_state->pix.height)) {
		if(ov5640_mode_VGA_640_480 != ov5640_state->mode){
			printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_VGA_640_480);
			ov5640_state->mode = ov5640_mode_VGA_640_480;
		}
	} else if ((320 == ov5640_state->pix.width) && (240 == ov5640_state->pix.height)) {
		if(ov5640_mode_QVGA_320_240 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_QVGA_320_240);
			ov5640_state->mode = ov5640_mode_QVGA_320_240;
		}
	} else if ((720 == ov5640_state->pix.width) && (480 == ov5640_state->pix.height)) {
		if(ov5640_mode_NTSC_720_480 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_NTSC_720_480);
			ov5640_state->mode = ov5640_mode_NTSC_720_480;
		}
	} else if ((720 == ov5640_state->pix.width) && (576 == ov5640_state->pix.height)) {
		if(ov5640_mode_PAL_720_576 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_PAL_720_576);
			ov5640_state->mode = ov5640_mode_PAL_720_576;
		}
	} else if ((800 == ov5640_state->pix.width) && (600 == ov5640_state->pix.height)) {
		if(ov5640_mode_SVGA_800_600 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_SVGA_800_600);
			ov5640_state->mode = ov5640_mode_SVGA_800_600;
		}
	} else if ((1280 == ov5640_state->pix.width) && (720 == ov5640_state->pix.height)) {
		if(ov5640_mode_720P_1280_720 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_720P_1280_720);
			ov5640_state->mode = ov5640_mode_720P_1280_720;
		}
	} else if ((1920 == ov5640_state->pix.width) && (1080 == ov5640_state->pix.height)) {
		if(ov5640_mode_1080P_1920_1080 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_1080P_1920_1080);
			ov5640_state->mode = ov5640_mode_1080P_1920_1080;
		}
	} else if ((2592 == ov5640_state->pix.width) && (1944 == ov5640_state->pix.height)) {
		if(ov5640_mode_QSXGA_2592_1944 != ov5640_state->mode){
				printk(" %d %d ++++++++++++++++++++++\n", ov5640_state->pix.width, ov5640_state->pix.height);
			ov5640_init_mode(sd, DEFAULT_FRAME_RATE, ov5640_mode_QSXGA_2592_1944);
			ov5640_state->mode = ov5640_mode_QSXGA_2592_1944;
		}
	} else {
		printk("nothing start ...");
	}

	return 0;
}
static int ov5640_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt)
{
	int err = 0;
	
	
	struct ov5640_state *ov5640_state = to_state(sd);
//	err = ov5640_try_fmt(sd, fmt);
//	if (err < 0)
//		return err;
//	ov5640_state->pix.width = fmt->width;
//	ov5640_state->pix.height = fmt->height;
	ov5640_state->pix.width = 640;
	ov5640_state->pix.height = 480;

	ov5640_resolution(sd, ov5640_state);

	return err;
}
static int ov5640_s_stream(struct v4l2_subdev *sd, int enable)
{
	
  printk( "%s %d\n", __func__, enable);
	return 0;
}
static int ov5640_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	struct ov5640_state *ov5640_state = to_state(sd);

printk("%s\n", __func__);
	
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
//	fsize->discrete.width = ov5640_mode_info_data[ov5640_state->frame_rate][ov5640_state->mode].width;
//	fsize->discrete.height = ov5640_mode_info_data[ov5640_state->frame_rate][ov5640_state->mode].height;
	fsize->discrete.width = ov5640_mode_info_data[1][ov5640_state->mode].width;
	fsize->discrete.height = ov5640_mode_info_data[1][ov5640_state->mode].height;
	return 0;
}

static int ov5640_enum_frameintervals(struct v4l2_subdev *sd, struct v4l2_frmivalenum *fival)
{
	int err = 0;
printk("%s\n", __func__);
	return err;
}

static int ov5640_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int err = 0;
printk("%s\n", __func__);
	return err;
}

static int ov5640_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
printk("%s\n", __func__);
	return err;
}

static int ov5640_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	dev_dbg(&client->dev, "%s\n", __func__);
	return err;
}

static int ov5640_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	dev_dbg(&client->dev, "%s: numerator %d, denominator: %d\n", \
		__func__, param->parm.capture.timeperframe.numerator, \
		param->parm.capture.timeperframe.denominator);

	return err;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const struct v4l2_subdev_core_ops ov5640_core_ops = {
	.init = ov5640_init,
//	.s_config = ov5640_s_config,
	.queryctrl = ov5640_queryctrl,
	.g_ctrl = ov5640_g_ctrl,
	.s_ctrl = ov5640_s_ctrl,
};

static const struct v4l2_subdev_video_ops ov5640_video_ops = {
//	.g_fmt = ov5640_g_fmt,
	.s_mbus_fmt = ov5640_s_fmt,
	.enum_framesizes = ov5640_enum_framesizes,
	.s_stream = ov5640_s_stream,
//	.enum_frameintervals = ov5640_enum_frameintervals,
//	.enum_fmt = ov5640_enum_fmt,
//	.try_fmt = ov5640_try_fmt,
//	.g_parm = ov5640_g_parm,
//	.s_parm = ov5640_s_parm,
};

static const struct v4l2_subdev_ops ov5640_ops = {
	.core = &ov5640_core_ops,
	.video = &ov5640_video_ops,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int ov5640_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ov5640_state *ov5640_state;
	struct v4l2_subdev *sd;
	int ret;

printk("%s\n", __func__);
	ov5640_state = kzalloc(sizeof(struct ov5640_state), GFP_KERNEL);
	if (ov5640_state == NULL)
		return -ENOMEM;

	ov5640_state->i2c_client = client;
	sd = &ov5640_state->sd;
	strcpy(sd->name, OV5640_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &ov5640_ops);
	
	ret = ov5640_detect(client);
	if (ret) {
		dev_info(&client->dev, "ov5640 sensor not found\n");
		goto err;
	}

	dev_info(&client->dev, "OV5640 has been probed\n");
	return 0;
err:
	kfree(ov5640_state);
	return ret;
}

static int ov5640_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id ov5640_id[] = {
	{ OV5640_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, ov5640_id);

static struct i2c_driver ov5640_driver = {
	.driver = {
		.owner = THIS_MODULE,
	  .name = OV5640_DRIVER_NAME,
	},
	.probe = ov5640_probe,
	.remove = ov5640_remove,
	.id_table = ov5640_id,
};

static __init int init_ov5640_sensor(void)
{
	return i2c_add_driver(&ov5640_driver);
}

static __exit void exit_ov5640_sensor(void)
{
  i2c_del_driver(&ov5640_driver);
}

module_init(init_ov5640_sensor);
module_exit(exit_ov5640_sensor);

MODULE_DESCRIPTION("ov5640 camera driver");
MODULE_AUTHOR("ApolloYang <Apollo5520@hotmail.com>");
MODULE_LICENSE("GPL");

