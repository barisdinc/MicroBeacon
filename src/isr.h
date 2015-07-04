#ifndef ISR_H
#define	ISR_H

#ifdef	__cplusplus
extern "C" {
#endif

extern void ADC_ISR(void);
extern void TIMER1_ISR(void);
extern void TIMER0_ISR(void);
extern void I2C_ISR(void);


#ifdef	__cplusplus
}
#endif

#endif	/* ISR_H */

