/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#elif defined(__18CXX)
    #include <p18cxxx.h>    /* C18 General Include File */
#endif

#if defined(__XC) || defined(HI_TECH_C)

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#endif

#include "user.h"
#include <pic18f4620.h>
#include "system.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */
void delay(unsigned int ms){
    int x = 0;
    for (x = 0; x < ms; x++)
        __delay_ms(1);
}

void InitApp(void)
{
    /* TODO Initialize User Ports/Peripherals/Project here */
    //all 8 pins in C are for motor output
    TRISC = 0;

    //Sensor Locations
    //Left Front, RE2
    TRISEbits.TRISE2=1;
    //Left, RE1
    TRISEbits.TRISE1=1;
    //Right, RB1
    TRISBbits.RB1=1;
    //Right Front, RB0
    TRISBbits.RB0=1;
    //Algorithm Switches
    //WALLHuger, RB3
    TRISBbits.RB3=1;
    //Flood Fill, RB2
    TRISBbits.RB1=1;

    //Determine Algorithm
    if(PORTBbits.RB3 == TRUE)
        algorithm = LEFTWALL;
    else
        algorithm = RIGHTWALL;

    /* Setup analog functionality and port direction */
    ADCON1 =0b00000001;
     /*bit7-6:unimplemented=0
       bit5:Voltage Reference 1=Vref(An2),0=Vss
       bit4:V.R 1=Vref(AN3),0=Vdd
      bit3-0:A/D port Configuration controls,0001=All enabled*/
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

    /* Initialize peripherals */

    /* Configure the IPEN bit (1=on) in RCON to turn on/off int priorities */
    T0CON = 0b11000100;
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

    /* Enable interrupts */
    //delay(3000);    //wait for settlement
    //INTCONbits.GIE = 1;
}

void sensorComputation(int sensorTemp[][SENSORCOMPUTATION])
{
    int max,min,i,j,sum;
    for(i = 0; i < NUMSENSORS ; i++)
    {
        for(j = 0 ; j < SENSORCOMPUTATION ; j++)
        {
            if(j == 0)
            {
                sum = min = max = sensorTemp[i][j];
                continue;
            }
            min = MIN(sensorTemp[i][j],min);
            max = MAX(sensorTemp[i][j],max);
            sum += sensorTemp[i][j];
        }
        sum = sum - min - max;
        sensorValue[i] = sum/(SENSORCOMPUTATION-2);
    }
}

void sensorUpdate()
{
    int i,j;
    int sensorTemp[NUMSENSORS][SENSORCOMPUTATION];

    //Read from sensor
    for(i = 0; i < SENSORCOMPUTATION; i++)
    {
        for(j = 0 ; j < NUMSENSORS ; j++)
        {
            ADCON0 = sensorLocation[j];

            ADCON0bits.GO_DONE = 1;
            while(ADCON0bits.GO_DONE != 0);
            sensorTemp[j][i] = (ADRESH << 2) + (ADRESL >> 6);
        }
    }

    //Compute and write to variables
    sensorComputation(sensorTemp);

}

int MIN(int a,int b)
{
    if(a>b) return b;
    return a;
}

int MAX(int a, int b)
{
    if(a>b) return a;
    return b;
}