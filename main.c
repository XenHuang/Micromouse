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
    
     TRISB=2;
     TRISD=0;
     ADCON1 =0b00000001;
     /*bit7-6:unimplemented=0
       bit5:Voltage Reference 1=Vref(An2),0=Vss
       bit4:V.R 1=Vref(AN3),0=Vdd
      bit3-0:A/D port Configuration controls,0000=All enabled*/
     ADCON2=0b00001010;
     /*bit7:1=Right Justified,0=left
       bit6:unimplemented=0
       bit5-3:A/D Acquisition time,001=2TAD
       bit2-0:A/D Conversion clock,010=Fosc/32*/
     ADCON0=0b00101011;
     /*bit7-6:unimplemented=0
       bit5-2:Analog Channel Select bits 1100(AN12)
       bit1:A/D Conversion Status bit,1=A/D conversion in progress,0=A/D idle
       bit0:A/D On bit,1=A/D converter module is enabled,0=disabled*/
     
    while(1)
    {
        
        ADCON0bits.GO_DONE = 1;
        while(ADCON0bits.GO_DONE != 0); //Loop here until A/D conversion completes 
        sensor = (ADRESH << 2) + (ADRESL >> 6);
        
        
        if(ADRESL > 100)
            LATD=1;
        else
            LATD=0;
       
    }

}

