#ifndef AUDIO_TONE_H
#define	AUDIO_TONE_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#define MODEM_MAX_PACKET 64

extern uint8_t modem_packet[MODEM_MAX_PACKET];   
extern uint32_t modem_packet_size;    
extern bool Sending_Character; 

void Modem_Setup(void);
void Modem_Start(void);
void Modem_Flush_Frame(void);


#ifdef	__cplusplus
}
#endif

#endif	/* AUDIO_TONE_H */

