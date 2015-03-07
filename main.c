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
    
    /* TODO <INSERT USER APPLICATION CODE HERE> */
     TRISC=0;
     TRISB=1<<1;
     
     
    while(1)
    {
        LATC=1<<7;
        delay(2);
        LATC=1<<6;
        delay(2);
        LATC=1<<5;
        delay(2);
        LATC=1<<4;
        delay(2);
        
       
    }

}

