#ifndef __GPIO_SMDKC110_H_
#define __GPIO_SMDKC110_H_

/*camera reset pin*/
#define CAMERA_RESET_PIN		S5PV210_GPJ3(2)
/*camera power down pin*/
#define CAMERA_PDN_PIN			S5PV210_GPJ3(1)
/*camera power pin*/
#define CAMERA_POWER_PIN		S5PV210_GPJ3(3)
#define GPIO_CAM_FLASH_EN		S5PV210_GPD0(1)
/*camera ready pin*/
//#define CAMERA_INT_PIN			S5PV210_GPH3(2)

/*sdhci1 power pin*/
//#define WL_CD_PIN				S5PV210_GPG0(6)
#define WL_RESET    		S5PV210_GPJ4(2)
#define WL_POWER				S5PV210_GPG1(0)
#define WL_POWER_CTRL			S5PV210_GPJ4(4)
#define WL_WAKE					S5PV210_GPG1(3)
#define WL_HOST_WAKE		S5PV210_GPH1(0)

#define BT_POWER				S5PV210_GPG1(1)
#define BT_RESET				S5PV210_GPG1(2)

#endif
/* end of __GPIO_SMDKC110_H_ */

