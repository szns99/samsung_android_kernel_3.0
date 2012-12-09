/* linux/drivers/media/video/saa7113.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 * 		http://www.samsung.com/
 *
 * Driver for SAA7113 (UXGA camera) from Samsung Electronics
 * 1/4" 2.0Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define SAA7113_COMPLETE
#ifndef __SAA7113_H__
#define __SAA7113_H__

struct saa7113_reg {
	unsigned char addr;
	unsigned char val;
};

struct saa7113_regset_type {
	unsigned char *regset;
	int len;
};

/*
 * Macro
 */
#define REGSET_LENGTH(x)	(sizeof(x)/sizeof(saa7113_reg))

/*
 * User defined commands
 */
/* S/W defined features for tune */
#define REG_DELAY	0xFF00	/* in ms */
#define REG_CMD		0xFFFF	/* Followed by command */

/* Following order should not be changed */
enum image_size_saa7113 {
	/* This SoC supports upto UXGA (1600*1200) */
#if 0
	QQVGA,	/* 160*120*/
	QCIF,	/* 176*144 */
	QVGA,	/* 320*240 */
	CIF,	/* 352*288 */
	VGA,	/* 640*480 */
#endif
    SQVGA,	/* 320*320*/
	SVGA,	/* 800*600 */
#if 0
	HD720P,	/* 1280*720 */
	SXGA,	/* 1280*1024 */
	UXGA,	/* 1600*1200 */
#endif
};

/*
 * Following values describe controls of camera
 * in user aspect and must be match with index of saa7113_regset[]
 * These values indicates each controls and should be used
 * to control each control
 */
enum saa7113_control {
	SAA7113_INIT,
	SAA7113_EV,
	SAA7113_AWB,
	SAA7113_MWB,
	SAA7113_EFFECT,
	SAA7113_CONTRAST,
	SAA7113_SATURATION,
	SAA7113_SHARPNESS,
};

#define SAA7113_REGSET(x)	{	\
	.regset = x,			\
	.len = sizeof(x)/sizeof(saa7113_reg),}


#define R_00_CHIP_VERSION                             0x00
/* Video Decoder */
	/* Video Decoder - Frontend part */
#define R_01_INC_DELAY                                0x01
#define R_02_INPUT_CNTL_1                             0x02
#define R_03_INPUT_CNTL_2                             0x03
#define R_04_INPUT_CNTL_3                             0x04
#define R_05_INPUT_CNTL_4                             0x05
	/* Video Decoder - Decoder part */
#define R_06_H_SYNC_START                             0x06
#define R_07_H_SYNC_STOP                              0x07
#define R_08_SYNC_CNTL                                0x08
#define R_09_LUMA_CNTL                                0x09
#define R_0A_LUMA_BRIGHT_CNTL                         0x0a
#define R_0B_LUMA_CONTRAST_CNTL                       0x0b
#define R_0C_CHROMA_SAT_CNTL                          0x0c
#define R_0D_CHROMA_HUE_CNTL                          0x0d
#define R_0E_CHROMA_CNTL_1                            0x0e
#define R_0F_CHROMA_GAIN_CNTL                         0x0f
#define R_10_CHROMA_CNTL_2                            0x10
#define R_11_MODE_DELAY_CNTL                          0x11
#define R_12_RT_SIGNAL_CNTL                           0x12
#define R_13_RT_X_PORT_OUT_CNTL                       0x13
#define R_14_ANAL_ADC_COMPAT_CNTL                     0x14
#define R_15_VGATE_START_FID_CHG                      0x15
#define R_16_VGATE_STOP                               0x16
#define R_17_MISC_VGATE_CONF_AND_MSB                  0x17
#define R_18_RAW_DATA_GAIN_CNTL                       0x18
#define R_19_RAW_DATA_OFF_CNTL                        0x19
#define R_1A_COLOR_KILL_LVL_CNTL                      0x1a
#define R_1B_MISC_TVVCRDET                            0x1b
#define R_1C_ENHAN_COMB_CTRL1                         0x1c
#define R_1D_ENHAN_COMB_CTRL2                         0x1d
#define R_1E_STATUS_BYTE_1_VD_DEC                     0x1e
#define R_1F_STATUS_BYTE_2_VD_DEC                     0x1f

/* Component processing and interrupt masking part */
#define R_23_INPUT_CNTL_5                             0x23
#define R_24_INPUT_CNTL_6                             0x24
#define R_25_INPUT_CNTL_7                             0x25
#define R_29_COMP_DELAY                               0x29
#define R_2A_COMP_BRIGHT_CNTL                         0x2a
#define R_2B_COMP_CONTRAST_CNTL                       0x2b
#define R_2C_COMP_SAT_CNTL                            0x2c
#define R_2D_INTERRUPT_MASK_1                         0x2d
#define R_2E_INTERRUPT_MASK_2                         0x2e
#define R_2F_INTERRUPT_MASK_3                         0x2f

/* Audio clock generator part */
#define R_30_AUD_MAST_CLK_CYCLES_PER_FIELD            0x30
#define R_34_AUD_MAST_CLK_NOMINAL_INC                 0x34
#define R_38_CLK_RATIO_AMXCLK_TO_ASCLK                0x38
#define R_39_CLK_RATIO_ASCLK_TO_ALRCLK                0x39
#define R_3A_AUD_CLK_GEN_BASIC_SETUP                  0x3a

/* General purpose VBI data slicer part */
#define R_40_SLICER_CNTL_1                            0x40
#define R_41_LCR_BASE                                 0x41
#define R_58_PROGRAM_FRAMING_CODE                     0x58
#define R_59_H_OFF_FOR_SLICER                         0x59
#define R_5A_V_OFF_FOR_SLICER                         0x5a
#define R_5B_FLD_OFF_AND_MSB_FOR_H_AND_V_OFF          0x5b
#define R_5D_DID                                      0x5d
#define R_5E_SDID                                     0x5e
#define R_60_SLICER_STATUS_BYTE_0                     0x60
#define R_61_SLICER_STATUS_BYTE_1                     0x61
#define R_62_SLICER_STATUS_BYTE_2                     0x62

/* X port, I port and the scaler part */
	/* Task independent global settings */
#define R_80_GLOBAL_CNTL_1                            0x80
#define R_81_V_SYNC_FLD_ID_SRC_SEL_AND_RETIMED_V_F    0x81
#define R_83_X_PORT_I_O_ENA_AND_OUT_CLK               0x83
#define R_84_I_PORT_SIGNAL_DEF                        0x84
#define R_85_I_PORT_SIGNAL_POLAR                      0x85
#define R_86_I_PORT_FIFO_FLAG_CNTL_AND_ARBIT          0x86
#define R_87_I_PORT_I_O_ENA_OUT_CLK_AND_GATED         0x87
#define R_88_POWER_SAVE_ADC_PORT_CNTL                 0x88
#define R_8F_STATUS_INFO_SCALER                       0x8f
	/* Task A definition */
		/* Basic settings and acquisition window definition */
#define R_90_A_TASK_HANDLING_CNTL                     0x90
#define R_91_A_X_PORT_FORMATS_AND_CONF                0x91
#define R_92_A_X_PORT_INPUT_REFERENCE_SIGNAL          0x92
#define R_93_A_I_PORT_OUTPUT_FORMATS_AND_CONF         0x93
#define R_94_A_HORIZ_INPUT_WINDOW_START               0x94
#define R_95_A_HORIZ_INPUT_WINDOW_START_MSB           0x95
#define R_96_A_HORIZ_INPUT_WINDOW_LENGTH              0x96
#define R_97_A_HORIZ_INPUT_WINDOW_LENGTH_MSB          0x97
#define R_98_A_VERT_INPUT_WINDOW_START                0x98
#define R_99_A_VERT_INPUT_WINDOW_START_MSB            0x99
#define R_9A_A_VERT_INPUT_WINDOW_LENGTH               0x9a
#define R_9B_A_VERT_INPUT_WINDOW_LENGTH_MSB           0x9b
#define R_9C_A_HORIZ_OUTPUT_WINDOW_LENGTH             0x9c
#define R_9D_A_HORIZ_OUTPUT_WINDOW_LENGTH_MSB         0x9d
#define R_9E_A_VERT_OUTPUT_WINDOW_LENGTH              0x9e
#define R_9F_A_VERT_OUTPUT_WINDOW_LENGTH_MSB          0x9f
		/* FIR filtering and prescaling */
#define R_A0_A_HORIZ_PRESCALING                       0xa0
#define R_A1_A_ACCUMULATION_LENGTH                    0xa1
#define R_A2_A_PRESCALER_DC_GAIN_AND_FIR_PREFILTER    0xa2
#define R_A4_A_LUMA_BRIGHTNESS_CNTL                   0xa4
#define R_A5_A_LUMA_CONTRAST_CNTL                     0xa5
#define R_A6_A_CHROMA_SATURATION_CNTL                 0xa6
		/* Horizontal phase scaling */
#define R_A8_A_HORIZ_LUMA_SCALING_INC                 0xa8
#define R_A9_A_HORIZ_LUMA_SCALING_INC_MSB             0xa9
#define R_AA_A_HORIZ_LUMA_PHASE_OFF                   0xaa
#define R_AC_A_HORIZ_CHROMA_SCALING_INC               0xac
#define R_AD_A_HORIZ_CHROMA_SCALING_INC_MSB           0xad
#define R_AE_A_HORIZ_CHROMA_PHASE_OFF                 0xae
#define R_AF_A_HORIZ_CHROMA_PHASE_OFF_MSB             0xaf
		/* Vertical scaling */
#define R_B0_A_VERT_LUMA_SCALING_INC                  0xb0
#define R_B1_A_VERT_LUMA_SCALING_INC_MSB              0xb1
#define R_B2_A_VERT_CHROMA_SCALING_INC                0xb2
#define R_B3_A_VERT_CHROMA_SCALING_INC_MSB            0xb3
#define R_B4_A_VERT_SCALING_MODE_CNTL                 0xb4
#define R_B8_A_VERT_CHROMA_PHASE_OFF_00               0xb8
#define R_B9_A_VERT_CHROMA_PHASE_OFF_01               0xb9
#define R_BA_A_VERT_CHROMA_PHASE_OFF_10               0xba
#define R_BB_A_VERT_CHROMA_PHASE_OFF_11               0xbb
#define R_BC_A_VERT_LUMA_PHASE_OFF_00                 0xbc
#define R_BD_A_VERT_LUMA_PHASE_OFF_01                 0xbd
#define R_BE_A_VERT_LUMA_PHASE_OFF_10                 0xbe
#define R_BF_A_VERT_LUMA_PHASE_OFF_11                 0xbf
	/* Task B definition */
		/* Basic settings and acquisition window definition */
#define R_C0_B_TASK_HANDLING_CNTL                     0xc0
#define R_C1_B_X_PORT_FORMATS_AND_CONF                0xc1
#define R_C2_B_INPUT_REFERENCE_SIGNAL_DEFINITION      0xc2
#define R_C3_B_I_PORT_FORMATS_AND_CONF                0xc3
#define R_C4_B_HORIZ_INPUT_WINDOW_START               0xc4
#define R_C5_B_HORIZ_INPUT_WINDOW_START_MSB           0xc5
#define R_C6_B_HORIZ_INPUT_WINDOW_LENGTH              0xc6
#define R_C7_B_HORIZ_INPUT_WINDOW_LENGTH_MSB          0xc7
#define R_C8_B_VERT_INPUT_WINDOW_START                0xc8
#define R_C9_B_VERT_INPUT_WINDOW_START_MSB            0xc9
#define R_CA_B_VERT_INPUT_WINDOW_LENGTH               0xca
#define R_CB_B_VERT_INPUT_WINDOW_LENGTH_MSB           0xcb
#define R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH             0xcc
#define R_CD_B_HORIZ_OUTPUT_WINDOW_LENGTH_MSB         0xcd
#define R_CE_B_VERT_OUTPUT_WINDOW_LENGTH              0xce
#define R_CF_B_VERT_OUTPUT_WINDOW_LENGTH_MSB          0xcf
		/* FIR filtering and prescaling */
#define R_D0_B_HORIZ_PRESCALING                       0xd0
#define R_D1_B_ACCUMULATION_LENGTH                    0xd1
#define R_D2_B_PRESCALER_DC_GAIN_AND_FIR_PREFILTER    0xd2
#define R_D4_B_LUMA_BRIGHTNESS_CNTL                   0xd4
#define R_D5_B_LUMA_CONTRAST_CNTL                     0xd5
#define R_D6_B_CHROMA_SATURATION_CNTL                 0xd6
		/* Horizontal phase scaling */
#define R_D8_B_HORIZ_LUMA_SCALING_INC                 0xd8
#define R_D9_B_HORIZ_LUMA_SCALING_INC_MSB             0xd9
#define R_DA_B_HORIZ_LUMA_PHASE_OFF                   0xda
#define R_DC_B_HORIZ_CHROMA_SCALING                   0xdc
#define R_DD_B_HORIZ_CHROMA_SCALING_MSB               0xdd
#define R_DE_B_HORIZ_PHASE_OFFSET_CRHOMA              0xde
		/* Vertical scaling */
#define R_E0_B_VERT_LUMA_SCALING_INC                  0xe0
#define R_E1_B_VERT_LUMA_SCALING_INC_MSB              0xe1
#define R_E2_B_VERT_CHROMA_SCALING_INC                0xe2
#define R_E3_B_VERT_CHROMA_SCALING_INC_MSB            0xe3
#define R_E4_B_VERT_SCALING_MODE_CNTL                 0xe4
#define R_E8_B_VERT_CHROMA_PHASE_OFF_00               0xe8
#define R_E9_B_VERT_CHROMA_PHASE_OFF_01               0xe9
#define R_EA_B_VERT_CHROMA_PHASE_OFF_10               0xea
#define R_EB_B_VERT_CHROMA_PHASE_OFF_11               0xeb
#define R_EC_B_VERT_LUMA_PHASE_OFF_00                 0xec
#define R_ED_B_VERT_LUMA_PHASE_OFF_01                 0xed
#define R_EE_B_VERT_LUMA_PHASE_OFF_10                 0xee
#define R_EF_B_VERT_LUMA_PHASE_OFF_11                 0xef

/* second PLL (PLL2) and Pulsegenerator Programming */
#define R_F0_LFCO_PER_LINE                            0xf0
#define R_F1_P_I_PARAM_SELECT                         0xf1
#define R_F2_NOMINAL_PLL2_DTO                         0xf2
#define R_F3_PLL_INCREMENT                            0xf3
#define R_F4_PLL2_STATUS                              0xf4
#define R_F5_PULSGEN_LINE_LENGTH                      0xf5
#define R_F6_PULSE_A_POS_LSB_AND_PULSEGEN_CONFIG      0xf6
#define R_F7_PULSE_A_POS_MSB                          0xf7
#define R_F8_PULSE_B_POS                              0xf8
#define R_F9_PULSE_B_POS_MSB                          0xf9
#define R_FA_PULSE_C_POS                              0xfa
#define R_FB_PULSE_C_POS_MSB                          0xfb
#define R_FF_S_PLL_MAX_PHASE_ERR_THRESH_NUM_LINES     0xff



unsigned char saa7113_init_reg[][2] = {
	{R_01_INC_DELAY, 0x08},
	{R_02_INPUT_CNTL_1, 0xc2},
	{R_03_INPUT_CNTL_2, 0x30},
	{R_04_INPUT_CNTL_3, 0x00},
	{R_05_INPUT_CNTL_4, 0x00},
	{R_06_H_SYNC_START, 0x89},
	{R_07_H_SYNC_STOP, 0x0d},
	{R_08_SYNC_CNTL, 0x88},
	{R_09_LUMA_CNTL, 0x01},
	{R_0A_LUMA_BRIGHT_CNTL, 0x80},//80
	{R_0B_LUMA_CONTRAST_CNTL, 0x47},//47
	{R_0C_CHROMA_SAT_CNTL, 0x40},
	{R_0D_CHROMA_HUE_CNTL, 0x00},
	{R_0E_CHROMA_CNTL_1, 0x01},
	{R_0F_CHROMA_GAIN_CNTL, 0x2a},
	{R_10_CHROMA_CNTL_2, 0x08},
	{R_11_MODE_DELAY_CNTL, 0x0c},
	{R_12_RT_SIGNAL_CNTL, 0x07},
	{R_13_RT_X_PORT_OUT_CNTL, 0x00},
	{R_14_ANAL_ADC_COMPAT_CNTL, 0x00},
	{R_15_VGATE_START_FID_CHG, 0x00},
	{R_16_VGATE_STOP, 0x00},
	{R_17_MISC_VGATE_CONF_AND_MSB, 0x00},
		/* Turn off VBI */
	{R_40_SLICER_CNTL_1, 0x20}, //20            /* No framing code errors allowed. */
	{R_41_LCR_BASE, 0xff},
	{R_41_LCR_BASE+1, 0xff},
	{R_41_LCR_BASE+2, 0xff},
	{R_41_LCR_BASE+3, 0xff},
	{R_41_LCR_BASE+4, 0xff},
	{R_41_LCR_BASE+5, 0xff},
	{R_41_LCR_BASE+6, 0xff},
	{R_41_LCR_BASE+7, 0xff},
	{R_41_LCR_BASE+8, 0xff},
	{R_41_LCR_BASE+9, 0xff},
	{R_41_LCR_BASE+10, 0xff},
	{R_41_LCR_BASE+11, 0xff},
	{R_41_LCR_BASE+12, 0xff},
	{R_41_LCR_BASE+13, 0xff},
	{R_41_LCR_BASE+14, 0xff},
	{R_41_LCR_BASE+15, 0xff},
	{R_41_LCR_BASE+16, 0xff},
	{R_41_LCR_BASE+17, 0xff},
	{R_41_LCR_BASE+18, 0xff},
	{R_41_LCR_BASE+19, 0xff},
	{R_41_LCR_BASE+20, 0xff},
	{R_41_LCR_BASE+21, 0xff},
	{R_41_LCR_BASE+22, 0xff},
	{R_58_PROGRAM_FRAMING_CODE, 0x40},//40
	{R_59_H_OFF_FOR_SLICER, 0x47},//47
	{R_5B_FLD_OFF_AND_MSB_FOR_H_AND_V_OFF, 0x83},//83
//	{R_5D_DID, 0xbd},
	{R_5E_SDID, 0x35}, //35

	//R_02_INPUT_CNTL_1, 0x84,		/* input tuner -> input 4, amplifier active */
	{R_02_INPUT_CNTL_1, 0xC0},		/* input tuner -> input 1, amplifier active */


};


#define SAA7113_INIT_REGS	\
	(sizeof(saa7113_init_reg) / sizeof(saa7113_init_reg[0]))
/*
 * EV bias
 */

static const struct saa7113_reg saa7113_ev_m6[] = {
};

static const struct saa7113_reg saa7113_ev_m5[] = {
};

static const struct saa7113_reg saa7113_ev_m4[] = {
};

static const struct saa7113_reg saa7113_ev_m3[] = {
};

static const struct saa7113_reg saa7113_ev_m2[] = {
};

static const struct saa7113_reg saa7113_ev_m1[] = {
};

static const struct saa7113_reg saa7113_ev_default[] = {
};

static const struct saa7113_reg saa7113_ev_p1[] = {
};

static const struct saa7113_reg saa7113_ev_p2[] = {
};

static const struct saa7113_reg saa7113_ev_p3[] = {
};

static const struct saa7113_reg saa7113_ev_p4[] = {
};

static const struct saa7113_reg saa7113_ev_p5[] = {
};

static const struct saa7113_reg saa7113_ev_p6[] = {
};

#ifdef SAA7113_COMPLETE
/* Order of this array should be following the querymenu data */
static const unsigned char *saa7113_regs_ev_bias[] = {
	(unsigned char *)saa7113_ev_m6, (unsigned char *)saa7113_ev_m5,
	(unsigned char *)saa7113_ev_m4, (unsigned char *)saa7113_ev_m3,
	(unsigned char *)saa7113_ev_m2, (unsigned char *)saa7113_ev_m1,
	(unsigned char *)saa7113_ev_default, (unsigned char *)saa7113_ev_p1,
	(unsigned char *)saa7113_ev_p2, (unsigned char *)saa7113_ev_p3,
	(unsigned char *)saa7113_ev_p4, (unsigned char *)saa7113_ev_p5,
	(unsigned char *)saa7113_ev_p6,
};

/*
 * Auto White Balance configure
 */
static const struct saa7113_reg saa7113_awb_off[] = {
};

static const struct saa7113_reg saa7113_awb_on[] = {
// from xxs
	{0xfc, 0x00},
	{0x30, 0x00}
};

static const unsigned char *saa7113_regs_awb_enable[] = {
	(unsigned char *)saa7113_awb_off,
	(unsigned char *)saa7113_awb_on,
};

/*
 * Manual White Balance (presets)
 */
static const struct saa7113_reg saa7113_wb_tungsten[] = {

};

static const struct saa7113_reg saa7113_wb_fluorescent[] = {

};

static const struct saa7113_reg saa7113_wb_sunny[] = {

};

static const struct saa7113_reg saa7113_wb_cloudy[] = {

};

/* Order of this array should be following the querymenu data */
static const unsigned char *saa7113_regs_wb_preset[] = {
	(unsigned char *)saa7113_wb_tungsten,
	(unsigned char *)saa7113_wb_fluorescent,
	(unsigned char *)saa7113_wb_sunny,
	(unsigned char *)saa7113_wb_cloudy,
};

/*
 * Color Effect (COLORFX)
 */
static const struct saa7113_reg saa7113_color_sepia[] = {
};

static const struct saa7113_reg saa7113_color_aqua[] = {
};

static const struct saa7113_reg saa7113_color_monochrome[] = {
};

static const struct saa7113_reg saa7113_color_negative[] = {
};

static const struct saa7113_reg saa7113_color_sketch[] = {
};

/* Order of this array should be following the querymenu data */
static const unsigned char *saa7113_regs_color_effect[] = {
	(unsigned char *)saa7113_color_sepia,
	(unsigned char *)saa7113_color_aqua,
	(unsigned char *)saa7113_color_monochrome,
	(unsigned char *)saa7113_color_negative,
	(unsigned char *)saa7113_color_sketch,
};

/*
 * Contrast bias
 */
static const struct saa7113_reg saa7113_contrast_m2[] = {
};

static const struct saa7113_reg saa7113_contrast_m1[] = {
};

static const struct saa7113_reg saa7113_contrast_default[] = {
};

static const struct saa7113_reg saa7113_contrast_p1[] = {
};

static const struct saa7113_reg saa7113_contrast_p2[] = {
};

static const unsigned char *saa7113_regs_contrast_bias[] = {
	(unsigned char *)saa7113_contrast_m2,
	(unsigned char *)saa7113_contrast_m1,
	(unsigned char *)saa7113_contrast_default,
	(unsigned char *)saa7113_contrast_p1,
	(unsigned char *)saa7113_contrast_p2,
};

/*
 * Saturation bias
 */
static const struct saa7113_reg saa7113_saturation_m2[] = {
};

static const struct saa7113_reg saa7113_saturation_m1[] = {
};

static const struct saa7113_reg saa7113_saturation_default[] = {
};

static const struct saa7113_reg saa7113_saturation_p1[] = {
};

static const struct saa7113_reg saa7113_saturation_p2[] = {
};

static const unsigned char *saa7113_regs_saturation_bias[] = {
	(unsigned char *)saa7113_saturation_m2,
	(unsigned char *)saa7113_saturation_m1,
	(unsigned char *)saa7113_saturation_default,
	(unsigned char *)saa7113_saturation_p1,
	(unsigned char *)saa7113_saturation_p2,
};

/*
 * Sharpness bias
 */
static const struct saa7113_reg saa7113_sharpness_m2[] = {
};

static const struct saa7113_reg saa7113_sharpness_m1[] = {
};

static const struct saa7113_reg saa7113_sharpness_default[] = {
};

static const struct saa7113_reg saa7113_sharpness_p1[] = {
};

static const struct saa7113_reg saa7113_sharpness_p2[] = {
};

static const unsigned char *saa7113_regs_sharpness_bias[] = {
	(unsigned char *)saa7113_sharpness_m2,
	(unsigned char *)saa7113_sharpness_m1,
	(unsigned char *)saa7113_sharpness_default,
	(unsigned char *)saa7113_sharpness_p1,
	(unsigned char *)saa7113_sharpness_p2,
};
#endif /* SAA7113_COMPLETE */

#endif
