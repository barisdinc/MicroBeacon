/* 
 * File:   ax25.h
 * Author: Kadir
 *
 * Created on September 21, 2014, 3:37 PM
 */

#ifndef AX25_H
#define	AX25_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>


typedef struct  {
	uint8_t callsign[7];
	uint8_t ssid;
} s_address;

extern void Ax25_Send_Header(s_address addresses[], int num_addresses);
void Ax25_Send_Byte(uint8_t byte);
extern void Ax25_Send_String(const char *string);
extern void Ax25_Send_Footer();

#ifdef	__cplusplus
}
#endif

#endif	/* AX25_H */

