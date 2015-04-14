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
#define MOTORDELAYMAX 2       //the bigger the slower
#define ROTATE90 200            //steps require for doing 90 turn
#define SMOOTHROTATEFACTOR 5    //factor that the outer/inter steps

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

#define FRONTWALLMIN 500          // the higher the value the closer
#define SIDEWALLMIN 100           // the higher the value the closer

#define KCONTROLLERMIN 10
#define KCONTROLLERSTEP 10

typedef enum {LEFT,RIGHT} Side;
unsigned char LMotorCounter = 0;
unsigned char RMotorCounter = 0;
unsigned char controllerSteps = KCONTROLLERSTEP;
int MotorDelayCounter = 0;
int LTurnCounter = 0;
int RTurnCounter = 0;
int RotateCounter = 0;


void motorCounterUpdate(Side,unsigned char);
unsigned char merge(unsigned char,unsigned char);
void moveMouse(unsigned char);
void KController();
void rotate();
void smoothTurn();

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
                //Continue rotation
                if(RotateCounter > 0)
                    rotate();
				else if (LTurnCounter > 0 || RTurnCounter > 0)
					smoothTurn();
				else if (sensorValue[LEFTSENSOR] < SIDEWALLMIN && algorithm == LEFTWALL)
				{
					RTurnCounter = SMOOTHROTATEFACTOR;
					smoothTurn();
				}
				else if (sensorValue[RIGHTSENSOR] < SIDEWALLMIN && algorithm == RIGHTWALL)
				{
					LTurnCounter = SMOOTHROTATEFACTOR;
					smoothTurn();
				}
                else if ((sensorValue[LEFTSENSOR] > SIDEWALLMIN && sensorValue[RIGHTSENSOR] > SIDEWALLMIN )
                           && (sensorValue[LEFTFRONTSENSOR] < FRONTWALLMIN || sensorValue[RIGHTFRONTSENSOR] < FRONTWALLMIN))
                {	// 2 walls
                   KController();
                } else if ((sensorValue[LEFTSENSOR] > SIDEWALLMIN && sensorValue[RIGHTSENSOR] > SIDEWALLMIN )
                        && (sensorValue[LEFTFRONTSENSOR] > FRONTWALLMIN || sensorValue[RIGHTFRONTSENSOR] > FRONTWALLMIN) && RotateCounter <= 0) 
				{	// 3 walls
                    RotateCounter = ROTATE90*2;
                    rotate();
                }
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
    if(side == LEFT && reverse == 1)
    {
        switch(LMotorCounter)
        {
            case 0b00000001: LMotorCounter = 0b00000010; break;
            case 0b00000010: LMotorCounter = 0b00000100; break;
            case 0b00000100: LMotorCounter = 0b00001000; break;
            case 0b00001000: LMotorCounter = 0b00000001; break;
            default: LMotorCounter = 0b00000001;
        }
    } else if(side == RIGHT && reverse == 1)
    {   
        switch(RMotorCounter)
        {
            case 0b00000001: RMotorCounter = 0b00001000; break;
            case 0b00000010: RMotorCounter = 0b00000001; break;
            case 0b00000100: RMotorCounter = 0b00000010; break;
            case 0b00001000: RMotorCounter = 0b00000100; break;
            default: RMotorCounter = 0b00001000;
        }
    } else if(side == LEFT && reverse == 0)
    {   
        switch(LMotorCounter)
        {
            case 0b00000001: LMotorCounter = 0b00001000; break;
            case 0b00000010: LMotorCounter = 0b00000001; break;
            case 0b00000100: LMotorCounter = 0b00000010; break;
            case 0b00001000: LMotorCounter = 0b00000100; break;
            default: LMotorCounter = 0b00001000;
        }
    } else if(side == RIGHT && reverse == 0)
    {
        switch(RMotorCounter)
        {
            case 0b00000001: RMotorCounter = 0b00000010; break;
            case 0b00000010: RMotorCounter = 0b00000100; break;
            case 0b00000100: RMotorCounter = 0b00001000; break;
            case 0b00001000: RMotorCounter = 0b00000001; break;
            default: RMotorCounter = 0b00000001;
        }
    }
}

unsigned char merge(unsigned char left,unsigned char right)
{
    unsigned char merged;
    merged = left + (right << 4);
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
    if(controllerSteps > 0)
    {   
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
        controllerSteps--;
        return;
    }

    if(sensorValue[LEFTSENSOR] > sensorValue[RIGHTSENSOR])
    {
        correctTo = LEFT;
        motorCounterUpdate(correctTo,0);
        moveMouse(merge(LMotorCounter,RMotorCounter));
    }
    else
    {
        correctTo = RIGHT;
        motorCounterUpdate(correctTo,0);
        moveMouse(merge(LMotorCounter,RMotorCounter));
    }
    controllerSteps = KCONTROLLERSTEP;
}

void rotate()
{
   // if (algorithm == LEFTWALL){
   // motorCounterUpdate(RIGHT,0);
   // motorCounterUpdate(LEFT,1);
   // } else {
    motorCounterUpdate(LEFT,0);
    motorCounterUpdate(RIGHT,1);
  //  }
    RotateCounter--;
}

void smoothTurn()
{
	if(LTurnCounter > 0)
	{
		motorCounterUpdate(LEFT,0);
		LTurnCounter--;
	} else if (RTurnCounter > 0){
		motorCounterUpdate(RIGHT,0);
		RTurnCounter--;
	}
}
