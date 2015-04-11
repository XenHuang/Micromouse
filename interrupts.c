/******************************************************************************/
/*Files to Include                                                            */

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

#include "system.h"
#include "user.h"
/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* High-priority service */

/******************************************************************************/

#define MOTORDELAYMIN 0         //the smaller the faster
#define MOTORDELAYMAX 10       //the bigger the slower
#define ROTATE90 215            //steps require for doing 90 turn
#define SMOOTHROTATEFACTOR 4    //factor that the outer/inter steps

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

#define FRONTWALLMIN 0          // the higher the value the closer
#define SIDEWALLMIN 100           // the higher the value the closer

#define KCONTROLLERMIN 10
#define KCONTROLLERMINSTEP 5
#define KCONTROLLERMID 60
#define KCONTROLLERMIDSTEP 15
#define KCONTROLLERMAX 120
#define KCONTROLLERMAXSTEP 30

typedef enum {LEFT,RIGHT} Side;
unsigned char LMotorCounter = 0;
unsigned char RMotorCounter = 0;
int MotorDelayCounter = 0;
int LMotorDelayCounter = 0;
int RMotorDelayCounter = 0;

void motorCounterUpdate(Side,unsigned char);
unsigned char merge(unsigned char,unsigned char);
void moveMouse(unsigned char);
void KController();

int ABS(int);


#if defined(__XC) || defined(HI_TECH_C)
void interrupt high_isr(void)
#elif defined (__18CXX)
#pragma code high_isr=0x08
#pragma interrupt high_isr
void high_isr(void)
#else
#error "Invalid compiler selection for implemented ISR routines"
#endif

{

      /* This code stub shows general interrupt handling.  Note that these
      conditional statements are not handled within 3 seperate if blocks.
      Do not use a seperate if block for each interrupt flag to avoid run
      time errors. */

#if 1
    
      /* TODO Add High Priority interrupt routine code here. */
	if(INTCONbits.TMR0IF == 1)
	{
            if(MotorDelayCounter > MOTORDELAYMAX)
            {
                //Three Wall
                //Two Side Wall
                if(sensorValue[LEFTSENSOR] > SIDEWALLMIN && sensorValue[RIGHTSENSOR] > SIDEWALLMIN)
                {
                    //KController();
                }
                //One Side Wall

                motorCounterUpdate(RIGHT,0);
                motorCounterUpdate(LEFT,0);
                moveMouse(merge(LMotorCounter,RMotorCounter));
                
                MotorDelayCounter = 0;
            }

            MotorDelayCounter++;
            INTCONbits.TMR0IF = 0;
	} 

#endif

}

/* Low-priority interrupt routine */
#if defined(__XC) || defined(HI_TECH_C)
void low_priority interrupt low_isr(void)
#elif defined (__18CXX)
#pragma code low_isr=0x18
#pragma interruptlow low_isr
void low_isr(void)
#else
#error "Invalid compiler selection for implemented ISR routines"
#endif
{

      /* This code stub shows general interrupt handling.  Note that these
      conditional statements are not handled within 3 seperate if blocks.
      Do not use a seperate if block for each interrupt flag to avoid run
      time errors. */

#if 0

      /* TODO Add Low Priority interrupt routine code here. */

      /* Determine which flag generated the interrupt */
      if(<Interrupt Flag 1>)
      {
          <Interrupt Flag 1=0>; /* Clear Interrupt Flag 1 */
      }
      else if (<Interrupt Flag 2>)
      {
          <Interrupt Flag 2=0>; /* Clear Interrupt Flag 2 */
      }
      else
      {
          /* Unhandled interrupts */
      }

#endif

}

void motorCounterUpdate(Side side, unsigned char reverse)
{
    if(reverse == TRUE)
    {
        if(side == LEFT)
            side = RIGHT;
        else    //side == RIGHT
            side = LEFT;
    }
    
    if(side == LEFT)
    {
        switch(LMotorCounter)
        {
            case 0b00000001: LMotorCounter = 0b00000010; break;
            case 0b00000010: LMotorCounter = 0b00000100; break;
            case 0b00000100: LMotorCounter = 0b00001000; break;
            case 0b00001000: LMotorCounter = 0b00000001; break;
            default: LMotorCounter = 0b00000001;
        }
    } else {    //side == RIGHT
        switch(RMotorCounter)
        {
            case 0b00000001: RMotorCounter = 0b00001000; break;
            case 0b00000010: RMotorCounter = 0b00000001; break;
            case 0b00000100: RMotorCounter = 0b00000010; break;
            case 0b00001000: RMotorCounter = 0b00000100; break;
            default: RMotorCounter = 0b00001000;
        }
    }
}

unsigned char merge(unsigned char left,unsigned char right)
{
    unsigned char merged;
    merged = (left << 4) + right;
    return merged;
}

void moveMouse(unsigned char action)
{
    LATC = action;
}


int ABS(int x)
{
    if(x<0) x = -x;
    return x;
}


void KController()
{
    Side correctTo;
    int diff = ABS(sensorValue[LEFTSENSOR] - sensorValue[RIGHTSENSOR]);

    int steps = 0;
    if(diff <= KCONTROLLERMIN)
        steps = KCONTROLLERMINSTEP;
    else if(diff > KCONTROLLERMIN && diff <= KCONTROLLERMID)
        steps = KCONTROLLERMIDSTEP;
    else if(diff > KCONTROLLERMID && diff >= KCONTROLLERMAX)
        steps = KCONTROLLERMAXSTEP;


    if(sensorValue[LEFTSENSOR] > sensorValue[RIGHTSENSOR])
    {
        correctTo = RIGHT;
        for(;steps > 0; steps--)
        {
            motorCounterUpdate(correctTo,0);
            moveMouse(merge(LMotorCounter,RMotorCounter));
        }
    }
    else
    {
        correctTo = LEFT;
        for(;steps > 0; steps--)
        {
            motorCounterUpdate(correctTo,0);
            moveMouse(merge(LMotorCounter,RMotorCounter));
        }
    }
}