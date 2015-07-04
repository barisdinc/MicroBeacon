/* 
 * File:   adf7012.h
 * Author: Kadir
 *
 * Created on September 21, 2014, 3:33 PM
 */

#ifndef ADF7012_H
#define	ADF7012_H

#ifdef	__cplusplus
extern "C" {
#endif


// Register Constants ========================================================

// Register 1 ----------------------------------------------------------------
#define ADF_OUTPUT_DIVIDER_BY_1 0
#define ADF_OUTPUT_DIVIDER_BY_2 1
#define ADF_OUTPUT_DIVIDER_BY_4 2
#define ADF_OUTPUT_DIVIDER_BY_8 3



// Register 1 ----------------------------------------------------------------
#define ADF_PRESCALER_4_5 0
#define ADF_PRESCALER_8_9 1
#define ADF_PRESCALER ADF_PRESCALER_8_9

// Register 2 ----------------------------------------------------------------
#define ADF_MODULATION_FSK 0
#define ADF_MODULATION_GFSK 1
#define ADF_MODULATION_ASK 2
#define ADF_MODULATION_OOK 3

// Register 3 ----------------------------------------------------------------
#define ADF_CP_CURRENT_0_3 0
#define ADF_CP_CURRENT_0_9 1
#define ADF_CP_CURRENT_1_5 2
#define ADF_CP_CURRENT_2_1 3
#define ADF_MUXOUT_LOGIC_LOW 0
#define ADF_MUXOUT_LOGIC_HIGH 1
#define ADF_MUXOUT_REG_READY 3
#define ADF_MUXOUT_DIGITAL_LOCK 4
#define ADF_MUXOUT_ANALOGUE_LOCK 5
#define ADF_MUXOUT_R_DIVIDER_2 6
#define ADF_MUXOUT_N_DIVIDER_2 7
#define ADF_MUXOUT_RF_R_DIVIDER 8
#define ADF_MUXOUT_DATA_RATE 9
#define ADF_MUXOUT_BATT_2_35 10
#define ADF_MUXOUT_BATT_2_75 11
#define ADF_MUXOUT_BATT_3 12
#define ADF_MUXOUT_BATT_3_25 13
#define ADF_MUXOUT_TEST_MODE 14
#define ADF_MUXOUT_SD_TEST_MODE 15
#define ADF_LD_PRECISION_3_CYCLES 0
#define ADF_LD_PRECISION_5_CYCLES 1
#define ADF7012_CRYSTAL_FREQ VCXO_FREQ


#ifdef	__cplusplus
}
#endif

#endif	/* ADF7012_H */

