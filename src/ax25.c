
#include "ax25.h"
#include "audio_tone.h"
#include "functions.h"

//Globals
uint16_t crc;
int16_t ones_in_a_row;


static void Update_Crc(uint8_t input_bit)
{
  crc ^= input_bit;
  if (crc & 1)
    crc = (crc >> 1) ^ 0x8408;  // X-modem CRC poly
  else
    crc = crc >> 1;
}

static void Send_Byte(uint8_t byte)
{
  int i;
  for (i = 0; i < 8; i++) {
    Update_Crc((byte >> i) & 1);
    if ((byte >> i) & 1) {
      // Next bit is a '1'
      if (modem_packet_size >= (MODEM_MAX_PACKET * 8))  // Prevent buffer overrun
        return;
     
      modem_packet[modem_packet_size >> 3] |= (1 << (modem_packet_size & 7));
   
      modem_packet_size++;
      if (++ones_in_a_row < 5) continue;
    }
    // Next bit is a '0' or a zero padding after 5 ones in a row
    if (modem_packet_size >= (MODEM_MAX_PACKET * 8))    // Prevent buffer overrun
      return;
    modem_packet[modem_packet_size >> 3] &= ~(1 << (modem_packet_size & 7));
   
    modem_packet_size++;
    ones_in_a_row = 0;
  }
}

// Exported functions
void Ax25_Send_Byte(uint8_t byte)
{
   Send_Byte(byte);
}

void Ax25_Send_Sync()
{
  uint8_t byte = 0x00;
  int i;
  for (i = 0; i < 8; i++, modem_packet_size++) {
    if (modem_packet_size >= MODEM_MAX_PACKET * 8)  // Prevent buffer overrun
      return;
    if ((byte >> i) & 1)
      modem_packet[modem_packet_size >> 3] |= (1 << (modem_packet_size & 7));
    else
      modem_packet[modem_packet_size >> 3] &= ~(1 << (modem_packet_size & 7));
  }
}

void Ax25_Send_Flag()
{
  uint8_t byte = 0x7e;
  int i;
  for (i = 0; i < 8; i++, modem_packet_size++) {
    if (modem_packet_size >= MODEM_MAX_PACKET * 8)  // Prevent buffer overrun
      return;
    if ((byte >> i) & 1)
      modem_packet[modem_packet_size >> 3] |= (1 << (modem_packet_size & 7));
    else
      modem_packet[modem_packet_size >> 3] &= ~(1 << (modem_packet_size & 7));
  }
}

void Ax25_Send_String(const char *string)
{
  int i;
  for (i = 0; string[i]; i++) {
    Ax25_Send_Byte(string[i]);
  }
}

void Ax25_Send_Header(s_address addresses[], int num_addresses)
{
 
  int i, j;
  modem_packet_size = 0;
  ones_in_a_row = 0;
  crc = 0xffff;

  // Send sync ("10 bytes of zeros")
  for (i = 0; i < 10; i++)
  {
    Ax25_Send_Sync();
  }

  //start the actual frame. Send 3 of them (one empty frame and the real start)
  for (i = 0; i < 3; i++)
  {
    Ax25_Send_Flag();
  }

  for (i = 0; i < num_addresses; i++) {
    // Transmit callsign
  	  for (j = 0; j < 6; j++)
           Send_Byte(addresses[i].callsign[j] << 1);
    
   // Transmit SSID. Termination signaled with last bit = 1
    if (i == num_addresses - 1){
      Send_Byte((('0' + addresses[i].ssid) << 1) | 0x01);
      
    }
    else
      Send_Byte(('0' + addresses[i].ssid) << 1);
  }

  // Control field: 3 = APRS-UI frame
  Send_Byte(0x03);

  // Protocol ID: 0xf0 = no layer 3 data
  Send_Byte(0xf0);

}

void Ax25_Send_Footer()
{
  // Save the crc so that it can be treated it atomically
  uint16_t final_crc = crc;

  // Send the CRC
  Send_Byte(~(final_crc & 0xff));
  final_crc >>= 8;
  Send_Byte(~(final_crc & 0xff));

  // Signal the end of frame
  Ax25_Send_Flag();

}