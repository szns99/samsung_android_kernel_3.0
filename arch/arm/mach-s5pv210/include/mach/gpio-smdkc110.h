#ifndef __GPIO_SMDKC110_H_
#define __GPIO_SMDKC110_H_


#define GPIO_PS_VOUT			S5PV210_GPH0(2)
#define GPIO_PS_VOUT_AF			0xFF

#define GPIO_BUCK_1_EN_A		S5PV210_GPH0(3)
#define GPIO_BUCK_1_EN_B		S5PV210_GPH0(4)

#define GPIO_BUCK_2_EN			S5PV210_GPH0(5)
#define GPIO_DET_35			S5PV210_GPH0(6)
#define GPIO_DET_35_AF			0xFF

#define GPIO_nPOWER			S5PV210_GPH2(6)

#define GPIO_EAR_SEND_END		S5PV210_GPH3(6)
#define GPIO_EAR_SEND_END_AF		0xFF

#define GPIO_HWREV_MODE0		S5PV210_GPJ0(2)
#define GPIO_HWREV_MODE1		S5PV210_GPJ0(3)
#define GPIO_HWREV_MODE2		S5PV210_GPJ0(4)
#define GPIO_HWREV_MODE3		S5PV210_GPJ0(7)

#define GPIO_PS_ON			S5PV210_GPJ1(4)

#define GPIO_MICBIAS_EN			S5PV210_GPJ4(2)

#define GPIO_UART_SEL			S5PV210_MP05(7)

/*camera reset pin*/
#define CAMERA_RESET_PIN		S5PV210_GPJ2(7)
/*camera power down pin*/
#define CAMERA_PDN_PIN			S5PV210_GPJ2(6)
/*camera power pin*/
#define CAMERA_POWER_PIN		S5PV210_GPJ3(3)

/*camera ready pin*/
//#define CAMERA_INT_PIN			S5PV210_GPH3(2)

/*sdhci1 power pin*/
//#define WL_CD_PIN				S5PV210_GPG0(6)
#define WL_RESET    		S5PV210_GPJ4(2)
#define WL_POWER				S5PV210_GPG1(0)
#define WL_WAKE					S5PV210_GPG1(3)
#define WL_HOST_WAKE		S5PV210_GPH1(0)

#endif
/* end of __GPIO_SMDKC110_H_ */

