#include "config.h"
#include "adf7012.h"
#include <stdint.h>
#include "functions.h"
#include <string.h>
#include <xc.h>

//Function Prototypes
void Adf_Reset_Register_Zero(void);
void Adf_Reset_Register_One(void);
void Adf_Reset_Register_Two(void);
void Adf_Reset_Register_Three(void);
void Adf_Reset(void);
void Adf_Write_Register_Zero(void);
void Adf_Write_Register_One(void);
void Adf_Write_Register_Two(void);
void Adf_Write_Register_Three(void);
void Adf_Write_Register(uint8_t*);
bool Adf_Locked(void);
void Ptt_Off(void);
void Radio_Setup(void);

extern bool PTT_OFF;
//Function Prototypes

// Configuration storage structs =============================================
typedef struct {
    struct {
        uint32_t  frequency_error_correction;
        uint8_t r_divider;
        uint8_t crystal_doubler;
        uint8_t crystal_oscillator_disable;
        uint8_t clock_out_divider;
        uint8_t vco_adjust;
        uint8_t output_divider;
    } r0;

    struct {
        uint32_t  fractional_n;
        uint8_t integer_n;
        uint8_t prescaler;
    } r1;

    struct {
        uint8_t mod_control;
        uint8_t gook;
        uint8_t power_amplifier_level;
        uint32_t  modulation_deviation;
        uint8_t gfsk_modulation_control;
        uint8_t index_counter;
    } r2;

    struct {
        uint8_t pll_enable;
        uint8_t pa_enable;
        uint8_t clkout_enable;
        uint8_t data_invert;
        uint8_t charge_pump_current;
        uint8_t bleed_up;
        uint8_t bleed_down;
        uint8_t vco_disable;
        uint8_t muxout;
        uint8_t ld_precision;
        uint8_t vco_bias;
        uint8_t pa_bias;
        uint8_t pll_test_mode;
        uint8_t sd_test_mode;
    } r3;
} Adf_Config;

Adf_Config adf_config;

// Configuration functions ===================================================

// Config resetting functions --------------------------------------------
void Adf_Reset_Config(void) {

    Adf_Reset_Register_Zero();
    Adf_Reset_Register_One();
    Adf_Reset_Register_Two();
    Adf_Reset_Register_Three();

    Adf_Reset();
}
// Power up default settings are defined here:
void Adf_Reset_Register_Zero(void) {
    adf_config.r0.frequency_error_correction = 0;               // Don't bother for now...
    adf_config.r0.r_divider = ADF7012_CRYSTAL_DIVIDER;          // Whatever works best for 2m, 1.25m and 70 cm ham bands
    adf_config.r0.crystal_doubler = 0;                          // Who would want that? Lower f_pfd means finer channel steps.
    adf_config.r0.crystal_oscillator_disable = 0;               // Disable traditional xtal
    adf_config.r0.clock_out_divider = 0;//1;                        // Don't bother for now...
    adf_config.r0.vco_adjust = 4;                               // Don't bother for now... (Will be automatically adjusted until PLL lock is achieved)
    adf_config.r0.output_divider = 0;
}
void Adf_Reset_Register_One(void) {
    adf_config.r1.integer_n = 80;                              // Pre-set for 144.390 MHz APRS. Will be changed according tx frequency on the fly
    adf_config.r1.fractional_n = 4035;//0xfdf;                             // Pre-set for 144.390 MHz APRS. Will be changed according tx frequency on the fly
    adf_config.r1.prescaler = ADF_PRESCALER_4_5;                // 8/9 requires an integer_n > 91; 4/5 only requires integer_n > 31
}
void Adf_Reset_Register_Two(void) {
    adf_config.r2.mod_control = ADF_MODULATION_FSK;             // For AFSK the modulation is done through the external VCXO we don't want any FM generated by the ADF7012 itself
    adf_config.r2.gook = 0;                                     // Whatever... This might give us a nicer swing in phase maybe...
    adf_config.r2.power_amplifier_level = 63;                   // 16 is about half maximum power. Output ?20dBm at 0x0, and 13 dBm at 0x7E at 868 MHz
    adf_config.r2.modulation_deviation = 5;                    // 16 is about half maximum amplitude @ ASK.
    adf_config.r2.gfsk_modulation_control = 0;                  // Don't bother for now...
    adf_config.r2.index_counter = 0;                            // Don't bother for now...
}
void Adf_Reset_Register_Three(void) {
    adf_config.r3.pll_enable = 1;//0;                               // Switch off PLL (will be switched on after Ureg is checked and confirmed ok)
    adf_config.r3.pa_enable = 1;//0;                                // Switch off PA  (will be switched on when PLL lock is confirmed)
    adf_config.r3.clkout_enable = 0;//1;                            // Clock out enable
    adf_config.r3.data_invert = 1;//1;                              // Results in a TX signal when TXDATA input is low
    adf_config.r3.charge_pump_current = ADF_CP_CURRENT_0_3; //ADF_CP_CURRENT_2_1;     // 2.1 mA. This is the maximum
    adf_config.r3.bleed_up = 0;                                 // Don't worry, be happy...
    adf_config.r3.bleed_down = 0;                               // Dito
    adf_config.r3.vco_disable = 0;                              // VCO is on

    adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK; //ADF_MUXOUT_REG_READY;                // Lights up the green LED if the ADF7012 is properly powered (changes to lock detection in a later stage)

    adf_config.r3.ld_precision = ADF_LD_PRECISION_3_CYCLES;     // What the heck? It is recommended that LDP be set to 1; 0 is more relaxed
    adf_config.r3.vco_bias = 3;                                 // In 0.5 mA steps; Default 6 means 3 mA; Maximum (15) is 8 mA
    adf_config.r3.pa_bias = 0;                                  // In 1 mA steps; Default 4 means 8 mA; Minimum (0) is 5 mA; Maximum (7) is 12 mA (Datasheet says uA which is bullshit)
    adf_config.r3.pll_test_mode = 0;
    adf_config.r3.sd_test_mode = 0;
}
void Adf_Reset(void) {

        ADF7012_CHIP_POWER_DOWN;
        Delay_ms(10);
	ADF7012_CHIP_POWER_UP;
  	Delay_ms(10);

}
// Configuration writing functions ---------------------------------------
void Adf_Write_Config(void) {
    Adf_Write_Register_Zero();
    Adf_Write_Register_One();
    Adf_Write_Register_Two();
    Adf_Write_Register_Three();
}
void Adf_Write_Register_Zero(void) {

    uint32_t reg =
        (0) |
        ((uint32_t)(adf_config.r0.frequency_error_correction & 0x7FF) << 2U) |
        ((uint32_t)(adf_config.r0.r_divider & 0xF ) << 13U) |
        ((uint32_t)(adf_config.r0.crystal_doubler & 0x1 ) << 17U) |
        ((uint32_t)(adf_config.r0.crystal_oscillator_disable & 0x1 ) << 18U) |
        ((uint32_t)(adf_config.r0.clock_out_divider & 0xF ) << 19U) |
        ((uint32_t)(adf_config.r0.vco_adjust & 0x3 ) << 23U) |
        ((uint32_t)(adf_config.r0.output_divider & 0x3 ) << 25U);

  
    uint8_t reg_ptr[4];
    memcpy(reg_ptr, &reg, 4);
    Reverse_Array(reg_ptr,4);
    Write_Adf7012_Reg(reg_ptr, 4);
}
void Adf_Write_Register_One(void) {
    uint32_t reg =
        (1) |
        ((uint32_t)(adf_config.r1.fractional_n & 0xFFF) << 2) |
        ((uint32_t)(adf_config.r1.integer_n & 0xFF ) << 14) |
        ((uint32_t)(adf_config.r1.prescaler & 0x1 ) << 22);
    
    uint8_t reg_ptr[3];
    memcpy(reg_ptr, &reg, 3);
    Reverse_Array(reg_ptr,3);
    Write_Adf7012_Reg(reg_ptr, 3);
}
void Adf_Write_Register_Two(void) {
    uint32_t reg =
        (2) |
        ((uint32_t)(adf_config.r2.mod_control & 0x3 ) << 2) |
        ((uint32_t)(adf_config.r2.gook & 0x1 ) << 4) |
        ((uint32_t)(adf_config.r2.power_amplifier_level & 0x3F ) << 5) |
        ((uint32_t)(adf_config.r2.modulation_deviation & 0x1FF) << 11) |
        ((uint32_t)(adf_config.r2.gfsk_modulation_control & 0x7 ) << 20) |
        ((uint32_t)(adf_config.r2.index_counter & 0x3 ) << 23);

 
    uint8_t reg_ptr[4];
    memcpy(reg_ptr, &reg, 4);
    Reverse_Array(reg_ptr,4);
    Write_Adf7012_Reg(reg_ptr, 4);
}
void Adf_Write_Register_Three(void) {
    uint32_t reg =
        (3) |
        ((uint32_t)(adf_config.r3.pll_enable & 0x1 ) << 2) |
        ((uint32_t)(adf_config.r3.pa_enable & 0x1 ) << 3) |
        ((uint32_t)(adf_config.r3.clkout_enable & 0x1 ) << 4) |
        ((uint32_t)(adf_config.r3.data_invert & 0x1 ) << 5) |
        ((uint32_t)(adf_config.r3.charge_pump_current & 0x3 ) << 6) |
        ((uint32_t)(adf_config.r3.bleed_up & 0x1 ) << 8) |
        ((uint32_t)(adf_config.r3.bleed_down & 0x1 ) << 9) |
        ((uint32_t)(adf_config.r3.vco_disable & 0x1 ) << 10) |
        ((uint32_t)(adf_config.r3.muxout & 0xF ) << 11) |
        ((uint32_t)(adf_config.r3.ld_precision & 0x1 ) << 15) |
        ((uint32_t)(adf_config.r3.vco_bias & 0xF ) << 16) |
        ((uint32_t)(adf_config.r3.pa_bias & 0x7 ) << 20) |
        ((uint32_t)(adf_config.r3.pll_test_mode & 0x1F ) << 23) |
        ((uint32_t)(adf_config.r3.sd_test_mode & 0xF ) << 28);

    uint8_t reg_ptr[4];
    memcpy(reg_ptr, &reg, 4);
    Reverse_Array(reg_ptr,4);
    Write_Adf7012_Reg(reg_ptr, 4);
}
bool Adf_Lock(void) {
    // fiddle around with bias and adjust capacity until the vco locks
    Delay_ms(200);
    int adj = adf_config.r0.vco_adjust; // use default start values from setup
    int bias = adf_config.r3.vco_bias;  // or the updated ones that worked last time

    adf_config.r3.pll_enable = 1;
    //adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
    adf_config.r3.muxout = ADF_MUXOUT_ANALOGUE_LOCK;       /*TODO both analogue_lock and digital_lock did not achieved test it via serial debug*/

    //Adf_Write_Config();                                  //closed not to write all registers
    Adf_Write_Register_Zero();
    Adf_Write_Register_Three();
    
    Delay_ms(5);

    while(!Adf_Locked()) {

        adf_config.r0.vco_adjust = adj;
        adf_config.r3.vco_bias = bias;
        adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
        //adf_config.r3.muxout = ADF_MUXOUT_ANALOGUE_LOCK; /*TODO both analogue_lock and digital_lock did not achieved test it via serial debug*/
        //Adf_Write_Config();                              //closed not to write all registers
        Adf_Write_Register_Zero();
        Adf_Write_Register_Three();

        Delay_ms(5);
        if(++bias == 14) {
            bias = 1;
            if(++adj == 4) {
                // Using best guess defaults:
                adf_config.r0.vco_adjust = 0;
                adf_config.r3.vco_bias = 5;
                return false;
            }
        }
    }
    return true;
}
bool Adf_Locked(void) {
    return Read_Adf7012_Muxout();
}
void Set_Freq(uint32_t freq) {
  adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_1;
  if (freq < 410000000) { adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_2; };
  if (freq < 210000000) { adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_4; };
  if (freq < 130000000) { adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_8; };

  uint32_t f_pfd = ADF7012_CRYSTAL_FREQ / adf_config.r0.r_divider;

  uint32_t n = (uint32_t)(freq / f_pfd);

  float ratio = (float)freq / (float)f_pfd;
  float rest  = ratio - (float)n;


  uint32_t m = (uint32_t)(rest * 4096);

  adf_config.r1.integer_n = n;
  adf_config.r1.fractional_n = m;

}
void Radio_Setup() {
  Adf_Reset_Config();
  Set_Freq(RADIO_FREQUENCY);              // Removed because of workload
  Adf_Write_Config();
  Delay_ms(10);
 }

bool Ptt_On() {
  adf_config.r3.pa_enable = 0;
  adf_config.r2.power_amplifier_level = 0;
  adf_config.r3.muxout = ADF_MUXOUT_REG_READY;
  //Adf_Write_Config();                     //Closed not to write all registers
  Adf_Write_Register_Two();
  Adf_Write_Register_Three();
  Delay_ms(10);

  if (!Read_Adf7012_Muxout())  // Power is not OK
  {
      return false;
  }
  else                         // Power is OK
  {
    adf_config.r3.pa_enable = 1;
    adf_config.r2.power_amplifier_level = 63;     //give max. power to the PA
    Delay_ms(10);
    //Adf_Write_Config();                     //Closed not to write all registers
    Adf_Write_Register_Two();
    Adf_Write_Register_Three();
    Delay_ms(1);
    return true;
  }
}
void Ptt_Off() {
  if (PTT_OFF) return; //do nothing if PTT is already off
  ADF7012_SET_DATA_PIN; 
  adf_config.r3.pa_enable = 0;
  adf_config.r2.power_amplifier_level = 0;
  //Adf_Write_Config();                     //Closed not to write all registers
  Adf_Write_Register_Two();
  Adf_Write_Register_Three();
  Delay_ms(10);
}
