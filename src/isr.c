
#include <xc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "config.h"


uint8_t adc__high = 0;                       //Adc return values
uint8_t adc__low  = 0;

#define BATTERY_MEAS_EVERY_MILISECOND 100000 //batarya olcme periyodu [ms]

bool Change_to_New_Baud = false;             //Flag to check for changing to new baud

uint32_t Systick_Counter = 0;                //Counter with period of 833 us

uint8_t BEACON_MODE ;

extern void Sinus_Generator(void);           //function which generates audio signal
extern void Sinus_Generator2(void);           //function which generates audio signal


//I2c global variables
#define I2C_DATA_BUFFER_LENGTH                5
uint8_t i2c_address                         = 0;
uint8_t i2c_data[I2C_DATA_BUFFER_LENGTH]    = {0};
uint8_t i2c_dummy                           = 0;
uint8_t i2c_data_index                      = 0;
//I2c global variables





void ADC_ISR(void){
  adc__high = ADRESH;
  adc__low  = ADRESL;

  ADIF = 0;
}


void TIMER1_ISR(void){
  Change_to_New_Baud = true;         //Change to new baud in Sinus_Generator()

  //reset Timer1 registers
  TMR1H = 0x00;
  TMR1L = 0x00;

  PIR1 &= ~0x04; //Clear Timer1 interrupt flag

  Systick_Counter += 1;
  if(Systick_Counter > BATTERY_MEAS_EVERY_MILISECOND){
    Systick_Counter = 0;
    ADCON0 |= 0b00000010;         //If period overruns for battery reading start the ADC conversion
  }
}


void TIMER0_ISR(void){
  if (BEACON_MODE == MODE_MORSE_FM) Sinus_Generator2();
  if (BEACON_MODE == MODE_AFSK1200) Sinus_Generator();                //Call Sinus_Generator() with Playback_Rate
  INTCON &= ~0x04;                  //Clear Timer0 interrupt flag
}

void I2C_ISR(void){
  if(!SSP1STATbits.D_nA){                           //last byte received/transmitted was an address
    SSP1IF = 0;                                     //clear interrupt flag
    i2c_address = SSP1BUF;                          //read address and clear the buffer
    if(SSP1STATbits.R_nW){                          //address with read option
      if(!SSP1STATbits.BF)                          //load the buffer if it is not full
        SSP1BUF = i2c_data[i2c_data_index++];       //First byte of the data to send
      else{                                         //clear the buffer and send data
        i2c_dummy = SSP1BUF;
        SSP1BUF = 0xCC;
      }
      CKP = 1;                                      //release the clock line
    }
    else{                                           //address with write option
      CKP = 1;                                      //release the clock line
    }
  }


  else{                                             //last byte received/transmitted was a data
    SSP1IF = 0;                                     //clear interrupt flag
    //if(SSP1STATbits.R_nW){                        //address with read option
    if(i2c_address & 0x01){                         //address with read option
      if(SSP1CON2bits.ACKSTAT == 1){                //transfer is complete
        CKP = 1;                                    //release the clock line
      }
      else{                                         //transfer is not complete
        if(!SSP1STATbits.BF){                       //load the buffer if it is not full
          SSP1BUF = i2c_data[i2c_data_index++];     //rest of the data to send
        }
        else{                                       //clear the buffer and send data
          i2c_dummy = SSP1BUF;
          SSP1BUF = 0xCC;                           //0xCC data is interpreted as buffer full error on master side
        }
        CKP = 1;                                    //release the clock line
      }
    }
    else{                                           //address with write option
      i2c_data[i2c_data_index++] = SSP1BUF;         //read data and clear buffer
      CKP = 1;                                      //release the clock line
    }
  }
  i2c_dummy = SSP1BUF;                              //clear buffer full flag
  WCOL      = 0;                                    //clear write collision flag
  SSPOV     = 0;                                    //clear overflow flag

  if(i2c_data_index >= I2C_DATA_BUFFER_LENGTH)
    i2c_data_index = 0;

}