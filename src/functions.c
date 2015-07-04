#include "functions.h"
#include <xc.h>
#include <stdio.h>
#include <string.h>

#define _XTAL_FREQ 32000000    //for '_delay' function, in fact it is the CPU freq rather than crystal frequency


bool Spi_Byte_Send(uint8_t);


/******************************************************************************
** Function name:		Gpio_Config
**
** Descriptions:		Configures the GPIO pins
** Returned value:		returns TRUE if successful
**
******************************************************************************/
bool Gpio_Config(void){

   //PortA and PortC configurations
    TRISA |= 0b00000010;
    TRISA &= 0b11001011;
    TRISC |= 0b00110011;
    TRISC &= 0b11100011;
    
  return true;

}


void Delay_ms(uint16_t time_to_delay)
{
    uint16_t i=0;
   
   Dac0_Start_Hold();         //Dac output with nominal value, while Delaying
   for(i=0; i<time_to_delay; i++){
   __delay_ms(1);             //This inline function does not accept dynamical argument
   }
}




/******************************************************************************
** Function name:		Read_Adf7012_Muxout
**
** Descriptions:		Read the Muxout pin of ADF7012
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Read_Adf7012_Muxout(void){

  if(PORTCbits.RC5)
      return true;
  else
      return false;
  
}


/******************************************************************************
** Function name:		Write_Adf7012_Reg
**
** Descriptions:		Writes the required value to the ADF7012
** Parameters:			reg_value ,a pointer to the first address of the register array
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Write_Adf7012_Reg(uint8_t* reg_value, uint8_t size_of_reg){

    uint8_t i = 0;
  Delay_ms(1);
  ADF7012_LOAD_REGISTER_ENABLE;
  Delay_ms(1);

  for (i = 0; i < size_of_reg; i++){
  Spi_Byte_Send(*(reg_value+i));
  }

  Delay_ms(1);
  ADF7012_LOAD_REGISTER_DISABLE;

  Delay_ms(1);
  return true;
}

/******************************************************************************
** Function name:		Send_Vcxo_Signal
**
** Descriptions:		Outputs the input argument to the DAC pin
** Parameters:			
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Send_Vcxo_Signal(uint8_t value){

  DACCON1 = value;              //Output the value to the DAC pin
  return true;
}
/******************************************************************************
** Function name:		Init_Adf7012
**
** Descriptions:		Starts the ADF7012 with hardcoded configuration
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
/*
_Bool Init_Adf7012(void){

uint8_t register0[4] = {0x04, 0x11, 0xE0, 0x00};
uint8_t register1[3] = {0x5B, 0x40, 0x01}      ;
uint8_t register2[4] = {0x00, 0x00, 0x81, 0xEE};
uint8_t register3[4] = {0x00, 0x45, 0x20, 0xFF};

Delay_ms(500);

/*send register0

ADF7012_LOAD_REGISTER_ENABLE;

Delay_ms(1);
force_register0:
  SSPSend(PORTNUM, register0, sizeof(register0));
  if(timeout_flag != 0){
    timeout_flag = 0;
    goto force_register0;
  }
  Delay_ms(1);
  ADF7012_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

/*send register1
  ADF7012_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register1:
  SSPSend(PORTNUM, register1, sizeof(register1));
  if(timeout_flag != 0){
    timeout_flag = 0;
    goto force_register1;
  }
  Delay_ms(1);
  ADF7012_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

/*send register2
  ADF7012_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register2:
  SSPSend(PORTNUM, register2, sizeof(register2));
  if(timeout_flag != 0){
     timeout_flag = 0;
     goto force_register2;
   }
  Delay_ms(1);
  ADF7012_LOAD_REGISTER_DISABLE;
  Delay_ms(10);


  /*send register3 
  ADF7012_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register3:
  SSPSend(PORTNUM, register3, sizeof(register3));
  if(timeout_flag != 0){
     timeout_flag = 0;
     goto force_register3;
   }
  Delay_ms(1);
  ADF7012_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

return TRUE;
}

 * */


/******************************************************************************
** Function name:		Reverse_Array
**
** Parameters:			Array which wanted to reversed, length of the array
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Reverse_Array(uint8_t* input,uint8_t length){
  uint8_t i = 0;
  uint8_t buffer_array[4] = {0};             //Dynamic memory allocation not allowed for 8 bit MCUs, simply create the largest array as hardcoded
  memcpy(buffer_array, input, length);

  for(i = 0; i<length; i++){
	  *(input+i) = *(buffer_array+(length-1)-i);
  }

  

return true;
}


/******************************************************************************
** Function name:		Spi_Byte_Send
**
** Parameters:			Data to send
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Spi_Byte_Send(uint8_t data){

    PORTAbits.RA2 = 0;
    uint8_t i;
    uint8_t data_to_send;
    data_to_send = data;
    for(i = 0; i <8; i++){

        if(data_to_send & 0x80)
            PORTCbits.RC2 = 1;
        else
            PORTCbits.RC2 = 0;

          data_to_send <<= 1;

        PORTAbits.RA2 = 1;
        Delay_ms(1);
        PORTAbits.RA2 = 0;
        Delay_ms(1);

    }
    return true;
}

void Timer0_Start(void){
  TMR0 = 0x00;
  TMR0IF = 0;
  TMR0IE = 1;            //Timer0 interrupt enable
}
void Timer0_Stop(void){
  TMR0IE = 0;            //Timer0 interrupt disable
  TMR0 = 0x00;
}

void Timer1_Start(void){
    TMR1H = 0x00;
    TMR1L = 0x00;       //Reset Timer1 registers

    TMR1IE = 0;         //Do not take Timer1 interrupt

    
    CCPR1H = 0x03;      //Compare Registers for 833us
    CCPR1L = 0x41;

    CCP1IF = 0;
    CCP1IE = 1;         //compare1 modulu interrupt enable
}

void Timer1_Stop(void){
    TMR1H = 0x00;
    TMR1L = 0x00;      //Reset Timer1 registers

    CCP1IE = 0;        // Compare1 module interrupt disable
}

void Dac0_Start_Hold(void){
    DACEN = 1;          //DAC output enable
    DACCON1 = 0x10;     //Vdd/2 on DACOUT
}

void Dac0_Stop(void){
    DACCON1 = 0x00;     //Gnd on DACOUT
    DACEN = 0;          //DAC output disable
}


void Adc1_Start(void){
    ADIF = 0;           //Clear interrupt flag
    ADON = 1;
    ADIE = 1;           //ADC interrupt enable
}

void Adc1_Stop(void){
    ADON = 0;          //ADC disable
    ADIE = 0;          //ADC interrupt disable
}