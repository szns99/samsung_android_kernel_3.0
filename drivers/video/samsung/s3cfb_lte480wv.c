/* linux/drivers/video/samsung/s3cfb_lte480wv.c
 *
 * Samsung LTE480 4.8" WVGA Display Panel Support
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "s3cfb.h"
#if 0
#include <linux/gpio.h>
#include <linux/delay.h>

void SPI_Initial()
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

void SPI_Uninitial()
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

void Start()
{
	SetCS(0);
	udelay(10);
	SetSCL(0);
	udelay(10);
	SetSDA(0);
	udelay(10);
}

void Stop()
{
	SetCS(1);
	udelay(10);
	SetSDA(1);
	udelay(10);
	SetSCL(1);
	udelay(50);
}

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
		SetSCL(0);
		udelay(10);
		SetSCL(1);
		udelay(10);
	}
	Stop();
}

void SPI_WriteData(u8 data)
{
	u16 bit;
	data = (data&0xff)|0x4000;
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
	SPI_WriteData(0x0F);//0E

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


	///=============================

	SPI_WriteComm(0x3A00);  //  RGB 18bits D[17:0]
	SPI_WriteData(0x77);			

	SPI_WriteComm(0x1100);

	mdelay(120);
	
	SPI_WriteComm(0x2900);
	SPI_Uninitial();
	mdelay(50);
}
#endif

static struct s3cfb_lcd lte480wv = {
	.width = 480,
	.height = 800,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 48,
		.v_fp = 13,
		.v_fpe = 1,
		.v_bp = 29,
		.v_bpe = 1,
		.v_sw = 3,
	},

	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* name should be fixed as 's3cfb_set_lcd_info' */
void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
	lte480wv.init_ldi = NULL;
	ctrl->lcd = &lte480wv;
}

