#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif


#define VCXO_FREQ 27000000L            //VCO 40Mhz
#define ADF7012_CRYSTAL_DIVIDER 5     //PFD = 20/15 Mhz
#define RADIO_FREQUENCY   435000000UL  //Beacon center frequency



//BEACON_MODE definitions
#define MODE_AFSK1200 1 
#define MODE_MORSE_FM 2
#define MODE_MORSE_CW 3
    
    
    
    
    
#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

