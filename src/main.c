/* 
 * File:   main.c
 * Author: Kadir
 *
 * Created on September 15, 2014, 6:55 PM
 */

#include "configuration.h"
#include "functions.h"
#include <math.h>
#include "ax25.h"
#include "audio_tone.h"
#include "isr.h"
#include "common_variables.h"
#include "config.h"


//#define Debug_Modem_Packet                 //uncomment this line to debug the data encoded with AX25

#define I2C_ADDRESS                           0x60

extern bool PTT_OFF;                         //PTT_OFF flag, not to pull Ptt_Off function within an interrupt
extern bool MODEM_TRANSMITTING;              //flag to check whether modem_packet is fully transmitted

extern uint8_t BEACON_MODE ;
bool Sending_Character = false;


#ifdef Debug_Modem_Packet
extern uint8_t modem_packet[MODEM_MAX_PACKET];
uint8_t k = 0;
#endif


void interrupt global_interrupt(){          //single interrupt vector to handle all of ISR's
    
    INTCON &= ~0x80;                        //Global interrupt disable in ISR

    //ADC interrupt
    if(ADIF){
      ADC_ISR();
      return;
    }
    //ADC interrupt
    

    //Timer1 interrupt
    if(PIR1 & 0x04){
      TIMER1_ISR();
      return;
    }
    //Timer1 interrupt

    
    //Timer0 interrupt
    if(INTCON & 0x04){
      TIMER0_ISR();
      return;
    }
    //Timer0 interrupt

    
    //I2C interrupt
    if(SSP1IF){
      I2C_ISR();
      return;
    }
    //I2C interrupt

    INTCON |= 0x80;                       //Global interrupt enabled again
}

void System_Start(void){

    //Watchdog timer configuration for 128 seconds
      WDTPS4 = 1;
      WDTPS3 = 0;
      WDTPS2 = 0;
      WDTPS1 = 0;
      WDTPS0 = 1;
    //Watchdog timer configuration for 128 seconds

    //Internal RC osc with 4xPLL operating at 32MHz
      OSCCON  = 0x00;
      OSCCON |= 0b11110000;
      OSCTUNE = 0x00;
    //Internal RC osc with 4xPLL operating at 32MHz
    
    //Configurations for Timer0
      TMR0CS = 0;                 //Internal clock source (Fosc/4)
      PSA    = 1;                 //Do not use Prescaler
    //Configurations for Timer0

    //Configurations for Timer1
      TMR1ON = 1;                 //Timer1 always count
      TMR1GE = 0;

      TMR1CS1 = 0;                //Fosc/4
      TMR1CS0 = 0;

      T1CKPS1 = 1;                //1/8 prescaler
      T1CKPS0 = 1;

      CCP1M3 = 1;                 //Software interrupt on compare event
      CCP1M2 = 0;
      CCP1M1 = 1;
      CCP1M1 = 0;
    //Configurations for Timer1
      
    //Configurations for Dac0
      DACOE   = 1;
      DACPSS1 = 0;
      DACPSS0 = 0;
      DACNSS  = 0;
    //Configurations for Dac0

   
    //Configurations for Adc1
      ANSA1   = 1;              //RA1 analog input
      ADCON0 &= 0b10000011;
      ADCON0 |= 0b00000100;     //AN1 channel select
      ADNREF  = 0;              //Vref- = GND
      ADPREF1 = 0;
      ADPREF0 = 0;              //Vref+ = Vdd
      ADCS2   = 1;
      ADCS1   = 1;
      ADCS0   = 0;              //Fosc/64 for conversion clock
      ADFM    = 1;              //Output on right hand side
    //Configurations for Adc1


    //Reset Interrupt Flags
      TMR0IF = 0;
      TMR1IF = 0;
      CCP1IF = 0;
      ADIF   = 0;
    //Reset Interrupt Flags

    //Configurations for I2C
      SSPEN   = 1;               //Pin configurations for the I2C peripheral
      SSPM3   = 0;               //Slave operation with 7 bit adress, S/P interrupt disabled
      SSPM2   = 1;
      SSPM1   = 1;
      SSPM0   = 0;
      GCEN    = 0;               //General Call disabled
      ACKDT   = 0;               //ACK on every receive    /*TODO this may not be implemented in Proteus*/
      SEN     = 1;               //Clock stretch enabled
      SSP1ADD = I2C_ADDRESS;     //7 bit adress
      SSPMSK |= 0b11111110;      //Address check for all bits
      PCIE    = 0;               //P interrupt disable
      SCIE    = 0;               //S interrupt disable
      BOEN    = 0;               //SSP1BUF is updated ignoring SSPOV
      AHEN    = 0;               //Adress hold disable
      DHEN    = 0;               //Data hold disable
      SBCDE   = 0;               //Collision Detect interrupt disable
      SSP1IF  = 0;               //Clear interrupt flag
      SSP1IE  = 1;               //I2C interrupt enable
    //Configurations for I2C

    //Global Interrupt ve Peripheral Interrupt Enable
      INTCON |= 0xC0;
}


int main(void) {
    Gpio_Config();                   //Gpio configuration
    System_Start();
    uint8_t loopcount = 255;
    while (loopcount && !(OSCSTAT & (0x01))){loopcount--;}    //Wait for HFIOFS Osc. stable bit
    /* TODO check the timeout somehow, CPU clock is not stable, implement a dummy counter or WDT will handle this */

    Timer1_Start();                  //Timer1 with 833 us period
    Dac0_Start_Hold();               //Start Dac output and make the output Vdd/2
    Adc1_Start();                    //Just configure Adc1 peripheral, conversion will be started within Timer1 ISR

    ADF7012_CHIP_POWER_DOWN;         //CE pin low , to reset the configuration on ADF7012
    Delay_ms(10);
    ADF7012_LOAD_REGISTER_DISABLE;   //LE pin high, to disable loading registers
    Delay_ms(10);
    ADF7012_CHIP_POWER_UP;           //CE pin high, to enable ADF7012
    Delay_ms(200);
    
    s_address beacon_address[2] = {{"TAMSAT", 5},{"DUNYAM", 6}};  //Source and destination adresses with callsigns
    Ax25_Send_Header(beacon_address,2);                           //Header with 2 adresses
    Ax25_Send_String("TAMSAT Beacon. Merhaba Dunya...");                                    //Send string
    Ax25_Send_Footer();                                           //Send Footer

    Modem_Setup();                                                //Set modem reset configurations to ADF7012
    Adf_Lock();                                                   //Try to achieve a good PLL lock, otherwise use default vco configuration
    Delay_ms(100);
    Ptt_Off();                                      //Turn of PTT
    
    

    //Startng conditions
    //Start with ore code over FM
    //BEACON_MODE = MODE_AFSK1200;
    BEACON_MODE = MODE_MORSE_FM;
    Sending_Character = true;
    Ptt_On();
    Timer0_Start();
    while(1){
        
        if ((BEACON_MODE == MODE_AFSK1200) &&  Sending_Character &&  PTT_OFF) Ptt_On(); 
        if ((BEACON_MODE == MODE_MORSE_FM) && !Sending_Character && !PTT_OFF) Ptt_Off();
        if (!Sending_Character && ( BEACON_MODE == MODE_MORSE_FM))
        {
            BEACON_MODE = MODE_AFSK1200;
            Modem_Flush_Frame();
        }
            
        //if ((BEACON_MODE == MODE_AFSK1200) && !MODEM_TRANSMITTING && !PTT_OFF) Ptt_Off();

     //     Delay_ms(5000);

    /*	  Modem_Flush_Frame();                                   //Transmit modem_packet[]
          while(MODEM_TRANSMITTING);
          Delay_ms(2000);

          Ptt_On();
          MODEM_TRANSMITTING = true;
          Timer0_Start();
          Delay_ms(10000);
          Timer0_Stop();
          Ptt_Off();

    */    
      

#ifdef Debug_Modem_Packet
        
          for (k=0; k< MODEM_MAX_PACKET; k++){
          Spi_Byte_Send(modem_packet[k]);
          }
          Delay_ms(3000);
#endif

          CLRWDT();                                            //Clear Watchdog timer
         }
    return (EXIT_SUCCESS);
}



    

// -----------------------------------------------------------------------


