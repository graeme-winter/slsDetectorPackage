#pragma once

#include "commonServerFunctions.h" // blackfin.h, ansi.h
#ifdef GOTTHARDD
#include <unistd.h>
#endif

/* AD9257 ADC DEFINES */
#define AD9257_ADC_NUMBITS			(24)

// default value is 0xF
#define AD9257_DEV_IND_2_REG		(0x04)
#define AD9257_CHAN_H_OFST			(0)
#define AD9257_CHAN_H_MSK			(0x00000001 << AD9257_CHAN_H_OFST)
#define AD9257_CHAN_G_OFST			(1)
#define AD9257_CHAN_G_MSK			(0x00000001 << AD9257_CHAN_G_OFST)
#define AD9257_CHAN_F_OFST			(2)
#define AD9257_CHAN_F_MSK			(0x00000001 << AD9257_CHAN_F_OFST)
#define AD9257_CHAN_E_OFST			(3)
#define AD9257_CHAN_E_MSK			(0x00000001 << AD9257_CHAN_E_OFST)

// default value is 0x3F
#define AD9257_DEV_IND_1_REG		(0x05)
#define AD9257_CHAN_D_OFST			(0)
#define AD9257_CHAN_D_MSK			(0x00000001 << AD9257_CHAN_D_OFST)
#define AD9257_CHAN_C_OFST			(1)
#define AD9257_CHAN_C_MSK			(0x00000001 << AD9257_CHAN_C_OFST)
#define AD9257_CHAN_B_OFST			(2)
#define AD9257_CHAN_B_MSK			(0x00000001 << AD9257_CHAN_B_OFST)
#define AD9257_CHAN_A_OFST			(3)
#define AD9257_CHAN_A_MSK			(0x00000001 << AD9257_CHAN_A_OFST)
#define AD9257_CLK_CH_DCO_OFST		(4)
#define AD9257_CLK_CH_DCO_MSK		(0x00000001 << AD9257_CLK_CH_DCO_OFST)
#define AD9257_CLK_CH_IFCO_OFST		(5)
#define AD9257_CLK_CH_IFCO_MSK		(0x00000001 << AD9257_CLK_CH_IFCO_OFST)

// default value is 0x00
#define AD9257_POWER_MODE_REG		(0x08)
#define AD9257_POWER_INTERNAL_OFST	(0)
#define AD9257_POWER_INTERNAL_MSK	(0x00000003 << AD9257_POWER_INTERNAL_OFST)
#define AD9257_INT_CHIP_RUN_VAL     ((0x0 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_INT_FULL_PWR_DWN_VAL ((0x1 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_INT_STANDBY_VAL      ((0x2 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_INT_RESET_VAL		((0x3 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_POWER_EXTERNAL_OFST	(5)
#define AD9257_POWER_EXTERNAL_MSK	(0x00000001 << AD9257_POWER_EXTERNAL_OFST)
#define AD9257_EXT_FULL_POWER_VAL	((0x0 << AD9257_POWER_EXTERNAL_OFST) & AD9257_POWER_EXTERNAL_MSK)
#define AD9257_EXT_STANDBY_VAL		((0x1 << AD9257_POWER_EXTERNAL_OFST) & AD9257_POWER_EXTERNAL_MSK)

// default value is 0x0
#define AD9257_TEST_MODE_REG        (0x0D)
#define AD9257_OUT_TEST_OFST        (0)
#define AD9257_OUT_TEST_MSK         (0x0000000F << AD9257_OUT_TEST_OFST)
#define AD9257_TST_OFF_VAL          ((0x0 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_MDSCL_SHRT_VAL   ((0x1 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_PSTV_FS_VAL      ((0x2 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_NGTV_FS_VAL      ((0x3 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_ALTRNTNG_CHKRBRD_VAL  ((0x4 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_PN_23_SQNC_VAL   ((0x5 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_PN_9_SQNC__VAL   ((0x6 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_0_WRD_TGGL_VAL ((0x7 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_USR_INPT_VAL     ((0x8 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_0_BT_TGGL_VAL  ((0x9 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_x_SYNC_VAL     ((0xa << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_BIT_HGH_VAL    ((0xb << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_MXD_BT_FRQ_VAL   ((0xc << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_RST_SHRT_GN_OFST (4)
#define AD9257_TST_RST_SHRT_GN_MSK  (0x00000001 << AD9257_TST_RST_SHRT_GN_OFST)
#define AD9257_TST_RST_LNG_GN_OFST  (5)
#define AD9257_TST_RST_LNG_GN_MSK   (0x00000001 << AD9257_TST_RST_LNG_GN_OFST)
#define AD9257_USER_IN_MODE_OFST    (6)
#define AD9257_USER_IN_MODE_MSK     (0x00000003 << AD9257_USER_IN_MODE_OFST)
#define AD9257_USR_IN_SNGL_VAL      ((0x0 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)
#define AD9257_USR_IN_ALTRNT_VAL    ((0x1 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)
#define AD9257_USR_IN_SNGL_ONC_VAL  ((0x2 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)
#define AD9257_USR_IN_ALTRNT_ONC_VAL  ((0x3 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)

// default value is 0x01
#define AD9257_OUT_MODE_REG			(0x14)
#define AD9257_OUT_FORMAT_OFST		(0)
#define AD9257_OUT_FORMAT_MSK		(0x00000001 << AD9257_OUT_FORMAT_OFST)
#define AD9257_OUT_BINARY_OFST_VAL	((0x0 << AD9257_OUT_FORMAT_OFST) & AD9257_OUT_FORMAT_MSK)
#define AD9257_OUT_TWOS_COMPL_VAL	((0x1 << AD9257_OUT_FORMAT_OFST) & AD9257_OUT_FORMAT_MSK)
#define AD9257_OUT_OTPT_INVRT_OFST  (2)
#define AD9257_OUT_OTPT_INVRT_MSK   (0x00000001 << AD9257_OUT_OTPT_INVRT_OFST)
#define AD9257_OUT_LVDS_OPT_OFST	(6)
#define AD9257_OUT_LVDS_OPT_MSK		(0x00000001 << AD9257_OUT_LVDS_OPT_OFST)
#define AD9257_OUT_LVDS_ANSI_VAL	((0x0 << AD9257_OUT_LVDS_OPT_OFST) & AD9257_OUT_LVDS_OPT_MSK)
#define AD9257_OUT_LVDS_IEEE_VAL	((0x1 << AD9257_OUT_LVDS_OPT_OFST) & AD9257_OUT_LVDS_OPT_MSK)

// default value is 0x3
#define AD9257_OUT_PHASE_REG		(0x16)
#define AD9257_OUT_CLK_OFST			(0)
#define AD9257_OUT_CLK_MSK			(0x0000000F << AD9257_OUT_CLK_OFST)
#define AD9257_OUT_CLK_0_VAL        ((0x0 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_60_VAL       ((0x1 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_120_VAL      ((0x2 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_180_VAL      ((0x3 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_240_VAL      ((0x4 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_300_VAL      ((0x5 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_360_VAL      ((0x6 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_420_VAL      ((0x7 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_480_VAL      ((0x8 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_540_VAL      ((0x9 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_600_VAL      ((0xa << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_660_VAL      ((0xb << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_IN_CLK_OFST			(4)
#define AD9257_IN_CLK_MSK			(0x00000007 << AD9257_IN_CLK_OFST)
#define AD9257_IN_CLK_0_VAL         ((0x0 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_1_VAL         ((0x1 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_2_VAL         ((0x2 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_3_VAL         ((0x3 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_4_VAL         ((0x4 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_5_VAL         ((0x5 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_6_VAL         ((0x6 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_7_VAL         ((0x7 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)

// default value is 0x4
#define AD9257_VREF_REG				(0x18)
#define AD9257_VREF_OFST			(0)
#define AD9257_VREF_MSK				(0x00000007 << AD9257_VREF_OFST)
#define AD9257_VREF_1_0_VAL         ((0x0 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_1_14_VAL        ((0x1 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_1_33_VAL        ((0x2 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_1_6_VAL         ((0x3 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_2_0_VAL         ((0x4 << AD9257_VREF_OFST) & AD9257_VREF_MSK)


void setAdc9257(int addr, int val) {

	u_int32_t codata;
	codata = val + (addr << 8);
	FILE_LOG(logINFO, ("\tSetting ADC SPI Register. Wrote 0x%04x at 0x%04x\n", val, addr));
	serializeToSPI(ADC_SPI_REG, codata, ADC_SERIAL_CS_OUT_MSK, AD9257_ADC_NUMBITS,
			ADC_SERIAL_CLK_OUT_MSK, ADC_SERIAL_DATA_OUT_MSK, ADC_SERIAL_DATA_OUT_OFST);
}

void prepareADC9257(){
    FILE_LOG(logINFOBLUE, ("Preparing ADC:\n"));

	//power mode reset
    FILE_LOG(logINFO, ("\tPower mode reset\n"));
    setAdc9257(AD9257_POWER_MODE_REG, AD9257_INT_RESET_VAL);

	//power mode chip run
	FILE_LOG(logINFO, ("\tPower mode chip run\n"));
	setAdc9257(AD9257_POWER_MODE_REG, AD9257_INT_CHIP_RUN_VAL);

	// binary offset
    FILE_LOG(logINFO, ("\tBinary offset\n"));
    setAdc9257(AD9257_OUT_MODE_REG, AD9257_OUT_BINARY_OFST_VAL);

	//output clock phase
	FILE_LOG(logINFO, ("\tOutput clock phase\n")); //FIXME:??
	setAdc9257(AD9257_OUT_PHASE_REG, AD9257_OUT_CLK_60_VAL);

	// lvds-iee reduced , binary offset
	FILE_LOG(logINFO, ("\tLvds-iee reduced, binary offset\n"));
	setAdc9257(AD9257_OUT_MODE_REG, AD9257_OUT_LVDS_IEEE_VAL);

	// all devices on chip to receive next command
	FILE_LOG(logINFO, ("\tAll devices on chip to receive next command\n"));
	setAdc9257(AD9257_DEV_IND_2_REG,
			AD9257_CHAN_H_MSK | AD9257_CHAN_G_MSK | AD9257_CHAN_F_MSK | AD9257_CHAN_E_MSK);
#ifdef GOTTHARDD
    setAdc9257(AD9257_DEV_IND_1_REG,
            AD9257_CHAN_D_MSK | AD9257_CHAN_C_MSK | AD9257_CHAN_B_MSK | AD9257_CHAN_A_MSK );// FIXME: gotthard setting dco and ifco to off??
#else
	setAdc9257(AD9257_DEV_IND_1_REG,
	        AD9257_CHAN_D_MSK | AD9257_CHAN_C_MSK | AD9257_CHAN_B_MSK | AD9257_CHAN_A_MSK |
	        AD9257_CLK_CH_DCO_MSK | AD9257_CLK_CH_IFCO_MSK);
#endif

	// vref 1.33
	FILE_LOG(logINFO, ("\tVref 1.33\n"));// FIXME: needed for Gottthard? earlier not set (default 3.0 v)
	setAdc9257(AD9257_VREF_REG, AD9257_VREF_1_33_VAL);

	// no test mode
	FILE_LOG(logINFO, ("\tNo test mode\n"));
	setAdc9257(AD9257_TEST_MODE_REG, AD9257_TST_OFF_VAL);

#ifdef TESTADC
	FILE_LOG(logINFOBLUE, ("Putting ADC in Test Mode!\n");
	// mixed bit frequency test mode
	FILE_LOG(logINFO, ("\tMixed bit frequency test mode\n"));
	setAdc9257(AD9257_TEST_MODE_REG, AD9257_TST_MXD_BT_FRQ_VAL);
#endif
}
