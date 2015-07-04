
#include "config.h"
#include "audio_tone.h"
#include "functions.h"
#include "adf7012.h"
#include <xc.h>

const uint8_t sine_table2[182]= {0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1b,0x1c,0x1c,0x1c,0x1d,0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1d,0x1c,0x1c,0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x19,0x19,0x19,0x18,0x18,0x17,0x17,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x13,0x12,0x12,0x11,0x11,0x10,
0x10,0xf,0xe,0xe,0xd,0xd,0xc,0xc,0xb,0xb,0xa,0xa,0x9,0x9,0x8,0x8,0x7,0x7,0x6,0x6,0x6,0x5,0x5,0x4,0x4,0x4,0x3,0x3,0x3,0x2,0x2,0x2,0x2,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x2,0x3,0x3,0x3,0x4,0x4,0x4,0x5,0x5,0x6,0x6,0x6,0x7,0x7,0x8,0x8,0x9,0x9,0xa,0xa,0xb,0xb,0xc,0xc,0xd,0xd,0xe,0xe,0xf};

//char message[] = "ABCDEFGHIJKLMNOPQRSTUVWYZ0123456789";
char message[] = "A"; //QDEYM2KAYM2KAYM2KAKN";

//last 3 bit is the length and first 5 bits is the character   0123456789-------ABCDEFGHIJKLMNOPQRSTUVWYZ
const unsigned char characters[] = {0xFD,0x7D,0x3D,0x1D,0x0D,0x05,0x85,0xC5,0xE5,0xF5,0,0,0,0,0,0,0,0x0A,0x44,0x54,0x23,0x01,0x14,0x33,0x04,0x02,0x3C,0x2B,0x24,0x1A,0x12,0x3B,0x34,0x6C,0x13,0x03,0x09,0x0B,0x0C,0x1B,0x4C,0x5C,0x64};


//Globals
bool PTT_OFF = false;
uint32_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];

int8_t morse_message[11] = { 0x00,0x13,0x01,0x11,1x01,0x13,0x01,0x11,0x01,0x09,0x09 };
int32_t tx_duration = 0;

bool SPACE = false;

extern bool Change_to_New_Baud;

// Source-specific
static const int TABLE_SIZE                = 182;
static const uint32_t PLAYBACK_RATE        = 31250;    // Timer0 with 32us
static const int BAUD_RATE                 = 1200;
static  uint8_t SAMPLES_PER_BAUD ;

static uint32_t PHASE_DELTA_1200;
static uint32_t PHASE_DELTA_2200;
static uint32_t PHASE_DELTA_1000;


static uint8_t current_byte;
static uint8_t current_sample_in_baud;    // 1 bit = SAMPLES_PER_BAUD samples

bool MODEM_TRANSMITTING = false;
bool Inside_Character  = false;   //Did we start sending the character

const uint32_t dit_duration = 3000; //duration for dot
const uint8_t dit_dah_weight = 3;  //a dah is xxx times dit

unsigned char char_data;
unsigned char char_length;

static uint8_t  phase_delta;               // 1200/2200 for standard AX.25
static uint8_t  phase;                     // Phase pointer for sine table
static uint32_t packet_pos;                // Next bit to be sent out

void Configure_Audio(void){
	SAMPLES_PER_BAUD = 26;             //26 samples will be taken for every baud
    PHASE_DELTA_1200 = 7;              //jump 7 samples  on table for 1200Hz sine  for AFSK AX25
    PHASE_DELTA_2200 = 13;             //jump 13 samples on table for 2200Hz sine  for AFSK AX25
    PHASE_DELTA_1000 = 6;             //jump 6 samples on table for 1000Hz sine    for MORSE

}

void Modem_Setup(void)
{
   Configure_Audio();                      //Configure Audio variables for AFSK1200
   Radio_Setup();                          //Reset configurations for ADF7012
}

void Modem_Flush_Frame(void)
{
  phase_delta = PHASE_DELTA_1200;
  phase = 0;
  packet_pos = 0;
  current_sample_in_baud = 0;
  MODEM_TRANSMITTING = true;
  ADF7012_CLEAR_DATA_PIN;
  Delay_ms(1);
  
/*
    try_to_push_button:
    if(!Ptt_On()){     //means power is bad
        Modem_Setup(); //try to reconfigure adf7012
        Delay_ms(200);
        goto try_to_push_button;
    }
*/
  Ptt_On(); /* TODO remove this line when working on real hardware */

  Delay_ms(100);
  Timer0_Start();
}

void Sinus_Generator(void) {

    uint8_t Audio_Signal;
    static uint8_t tone_index = 0;
if (MODEM_TRANSMITTING == true) {

   if (packet_pos == modem_packet_size) {
      MODEM_TRANSMITTING = false;             //Flag to check all packet content is transmitted
      Timer0_Stop();
      
      Send_Vcxo_Signal(0x10);                 //DAC output to Vdd/2
      goto end_generator;                     //return from the function
    }

    // If we have changed to new baud already
    if (current_sample_in_baud == 0) {   
      if ((packet_pos & 7) == 0)              // Load up next byte
        current_byte = modem_packet[packet_pos >> 3];
      else
        current_byte = current_byte >> 1 ;    // Load up next bit
      if ((current_byte & 0x01) == 0) {
        // Toggle tone (1200 or 2200)
         if(tone_index){
            phase_delta = PHASE_DELTA_1200;
            tone_index = 0;
            //ADF7012_SET_CLK_PIN;
          }
          else{
            phase_delta = PHASE_DELTA_2200;
            //ADF7012_CLEAR_CLK_PIN;
            tone_index = 1;
          }
      }
    }

    phase += phase_delta;
    if(phase >= TABLE_SIZE)                    //No modulus instruction for CPU, takes more cycles to operate when compiling modulus operation
        phase = phase - TABLE_SIZE;

   
    Audio_Signal = *(sine_table2 + phase) ;     //Take the appropriate Audio sample from the table
    Send_Vcxo_Signal(Audio_Signal);            //Output the Audio sample to DAC output

    current_sample_in_baud++;
    
    if(Change_to_New_Baud == true) {           //Change to new baud if the the required time for a single baud is spent
      current_sample_in_baud = 0;
      packet_pos++;
      Change_to_New_Baud = false;
    }
     
  }

end_generator:

  return;

}


void generate(unsigned char a)
{
 unsigned char n = a & 7; // beep count, 7 is 00000111, last 3 bits are length
 for( n = n + 2; n > 2; n--)
  {
  unsigned char mask = ( 1 << n );
  if( a & mask )
   {
    tx_duration = 3;
   } else
   {
    //printf("dit ");
   }
  }
//  printf("-");
 //pause(2); // 3 dots between characters, 1 was already after the beep
}

void sendMorse(unsigned char a)
{
 if (a>96) {a-=32; } //change lowercase to uppercase 
 if (((a >= '0' ) && (a <= '9' )) || ((a >= 'A' ) && (a <= 'Z' )))
 {
 unsigned char k = characters[a - '0']; //The list is 0,1,2,3,4,5... so first element equals to '0'
 generate(k);
 }
}


//Send each byte from queue
void sendMessage( const char *msg )
{
// char *m = msg;
 //for( ; *m ; m++ ) {
 //sendMorse( *m );
 //printf("\n\r");
 //}
//as an alternative using indices
//for(int i=0;msg[i]!=\0';i++)
//{ msg[i]..... }
}


void Sinus_Generator2(void) {

    uint8_t Audio_Signal;
    static uint8_t tone_index = 0;

    phase_delta = PHASE_DELTA_1000;
    
    if (!Sending_Character) return;
    
    if (tx_duration > 0)
    {
        phase += phase_delta;
        if(phase >= TABLE_SIZE)                    //No modulus instruction for CPU, takes more cycles to operate when compiling modulus operation
            phase = phase - TABLE_SIZE;


        if (!SPACE) 
        {
         Audio_Signal = *(sine_table2 + phase);     //Take the appropriate Audio sample from the table
         Send_Vcxo_Signal(Audio_Signal);            //Output the Audio sample to DAC output
        }
        tx_duration = tx_duration - 1;
    } else //tx_duration is not >= 0 
    {
        //if (Sending_Character)
            if(!Inside_Character)
            {
                {
                    unsigned char char_to_send = message[packet_pos];
                    if (char_to_send>96) {char_to_send-=32; } //change to uppercase a->A b->B
                    if (((char_to_send >= '0' ) && (char_to_send <= '9' )) || ((char_to_send >= 'A' ) && (char_to_send <= 'Z' )))
                    {
                        char_data   = characters[char_to_send - '0']; //The list is 0,1,2,3,4,5... so first element equals to '0'
                        char_length = char_data & 7; // beep count, 7 is 00000111, last 3 bits are length
                        Inside_Character = true;
                    }

                }
            } else //we're inside a character
            {
                if (SPACE) 
                {                
                    unsigned char mask = ( 1 << (char_length + 2) );
                    if( char_data & mask )
                        {
                            tx_duration = dit_duration * dit_dah_weight; //dah
                        } else
                        {
                            tx_duration = dit_duration; //dit
                        }
                    SPACE = false;
                    if (char_length <= 0)
                        {
                            Inside_Character = false;
                            packet_pos++;
                            if (packet_pos >= (sizeof(message)-1)) 
                            {
                                packet_pos=0;
                                Sending_Character = false;
                                Timer0_Stop();
                            }
                            tx_duration = dit_duration * dit_dah_weight; SPACE = true;
                        }
                    char_length -= 1;
                } else { tx_duration = dit_duration; SPACE = true; }  //send space adter each dit/dah 
                    
            }
    }
    
    

  return;

}
