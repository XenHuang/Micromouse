/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>        /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>       /* HiTech General Include File */
#elif defined(__18CXX)
    #include <p18cxxx.h>   /* C18 General Include File */
#endif

#if defined(__XC) || defined(HI_TECH_C)

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#endif

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */

#include <stdio.h>
#include <stdlib.h>
#include <pic18f4620.h>
#include <time.h>
#include <xc.h>
#define _XTAL_FREQ 32000000
/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

/* i.e. uint8_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/
int delay_time = 0;
void delay(unsigned int ms){
    int x = 0;
    for (x = 0; x < ms; x++)
        __delay_ms(1);
}

    int sensor;
	int motor;
     
void main(void)
{
    //enable 8MHz internal clock
    OSCCON = 0b01111100;
    //enable internal PLL clock = 32MHz
    OSCTUNE = 0b01001111;
   
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();
   
	T0CON = 0b11000000;
		//7.TMR0ON: 1 = Enables Timer()
		//6.T08BIT: 1 = 8-bit timer, 0 = 16-bit
		//5.T0CS: 0 = Internal instruction cycle clock(CLKO, not T0CKI)
		//4.T0SE: 0 = increment on low-to-high transition onT0CKI pin
		//3.PSA: 0 = Using Timer() prescaler
		//2-0. T0PS2:T0PS0: 000 = 1:2 ------ 111 = 1:256 prescale value

	INTCON2 = 0b11110100;
		//7.RBPU: 1 = All PORTB pull-ups are disabled
		//6.INTEDG0: 1 = interrupt on rising edge for interrupt 0
		//5.INTEDG1: same as above but for 1
		//4.INTEDG2: same as above but for 2
		//3.Unimplemented
		//2.TMR0IP:	1 = TMR0 Overflow interrupt = High (on Logic Diagram)
		//1.Unimplemented
		//0.RBIP: 1 = RB port change interrupt = High (on Logic Diagram)

	INTCON = 0b00100000;
		//7.GIE/GIEH: 1 = Enable interrupts
		//6.PEIE/GIEL 1 = Enable peripheral interrupts
		//5.TMR0IE 1 = Enable TMR0 Overflow interrupt
		//4.INT0IE 1 = Enable INT0 interrupt
		//3.RBIE 1 = Enable RB port change interrupt
		//2.TMR0IF 1 = TMR0 Overflow interrupt flag 
		//1.INT0IF 1 = INT0 interrupt flag
		//0.RBIF 1 = RB port change interrupt flag

	INTCONbits.GIE = 1;

	TRISC = 0;
 
    while(1)
    {
		        
    }

}

void intterrupt motro()
{
	if(INTCONbits.TMR0IF == 1)
	{
		if(motor < 4)
		motor++;
		else
		motor = 0;

		switch(motor)
		{
			case 0: LATC = 0b10001000;
			case 1: LATC = 0b01000100;
			case 2: LATC = 0b00100010;
			case 3: LATC = 0b00010001;
		}	
		INTCONbits.TMR0IF = 0;
	} 
}
