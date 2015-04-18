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
#define ROTATE90 167            //steps require for doing 90 turn
#define SMOOTHROTATEFACTOR 5    //factor that the outer/inter steps
#define REVERSEFACTOR 15         //factor that helps correct the 180 turn
#define FORWARDFACTOR 420         //factor that helps forward till 90 degree turn
#define FORWARDFACTOR2 380         //factor that helps forward till 90 degree turn

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

// Higher the value the closer
#define FRONTWALLMIN 420          // 2,3 senses distance closer than this rotates
#define FRONTWALLMAX 140          // 2,3 senses distance further than this makes left/right turn
#define WALLINONEHALFCELL 100         // Is there a frontwall after one and half cell.
#define SIDEWALLMIN 70           // 0,1 senses distance further than this makes left/right turn
#define SIDEWALLMIN2 100
#define FRONTWALLMAX2 800         // 2,3 senses distance closer than this then reverse

#define KCONTROLLERMAX 150        // Maximum diff bettween 0,1 sensors value
#define KCONTROLLERMID 20
#define KCONTROLLERSTEP 8
#define KCONTROLLERSTEPMID 4
#define KCONTROLLERSTEPMAX 1

typedef enum {LEFT,RIGHT} Side;
unsigned char LMotorCounter = 0;
unsigned char RMotorCounter = 0;
unsigned char controllerSteps = KCONTROLLERSTEP;
int MotorDelayCounter = 0;
int LTurnCounter = 0;
int RTurnCounter = 0;
int RotateCounter = 0;
int ReverseCounter = 0;
int ForwardCounter = 0;
Side rotatingSide;
unsigned char justTurned = 0;  


void motorCounterUpdate(Side,unsigned char);
unsigned char merge(unsigned char,unsigned char);
void moveMouse(unsigned char);
void KController();
void rotate(Side);
void smoothTurn();
void Reversing();
void forward();

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
                if (ForwardCounter > 0)      //keep forwarding until counter goes to 0
                    forward();
                else if (ReverseCounter > 0)   //keep reversing until counter goes to 0
					Reversing();
                else if(RotateCounter > 0)      //keep rotating until counter goes to 0
                {
                    // If front left or front right has a wall closer than 800, reverse 15 steps
                    if ((sensorValue[LEFTFRONTSENSOR] > FRONTWALLMAX2 || sensorValue[RIGHTFRONTSENSOR] > FRONTWALLMAX2)) {
                        ReverseCounter = REVERSEFACTOR;
                        Reversing();
                    } else {  //else rotate which side?
                    rotate(rotatingSide);
                    }
                }
//				else if (LTurnCounter > 0 || RTurnCounter > 0) {
//					//smoothTurn();
//                }
				else if (sensorValue[LEFTSENSOR] < SIDEWALLMIN && algorithm == LEFTWALL)    // Always left turn if left sensor senses less than 70 in upper left direction.
				{
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = FORWARDFACTOR;
                    }else {  //if not just rotate, forward step of 420.
                   ForwardCounter = FORWARDFACTOR2;
                    }
                    forward();   //forward with given forward factor
                    RotateCounter = ROTATE90;
                    rotatingSide = LEFT;
                    justTurned = 1;   //enable just turned.
					//RTurnCounter = SMOOTHROTATEFACTOR;
					//smoothTurn();
				}
                //Right turn if upper left has wall && front has wall in 1.5 cells && upper right has no wall.
                //if (right sensor senses less than 70) && (both front sensor senses greater than 100) && (left sensor senses higher than 70) 
				else if (sensorValue[RIGHTSENSOR] < SIDEWALLMIN && (sensorValue[LEFTFRONTSENSOR] > WALLINONEHALFCELL && sensorValue[RIGHTFRONTSENSOR] > WALLINONEHALFCELL)
                        && sensorValue[LEFTSENSOR] > SIDEWALLMIN && algorithm == LEFTWALL)   
				{
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = FORWARDFACTOR;
                    } else  {  //if not just rotate, forward step of 420.
                    ForwardCounter = FORWARDFACTOR2;
                    }
                    forward();   //forward with given forward factor
                    RotateCounter = ROTATE90;          
                    rotatingSide = RIGHT;
                    justTurned = 1;   //enable just turned.
					//RTurnCounter = SMOOTHROTATEFACTOR;
					//smoothTurn();
				} 
                // Go forward and self correct if upper left and upper right have walls and no wall in the front
                //if (right sensor and left sensor senses greater than 70) && (either front sensors senses less than 420)
                else if ((sensorValue[LEFTSENSOR] > SIDEWALLMIN && sensorValue[RIGHTSENSOR] > SIDEWALLMIN2 )
                           && (sensorValue[LEFTFRONTSENSOR] < FRONTWALLMIN || sensorValue[RIGHTFRONTSENSOR] < FRONTWALLMIN))
                {	// 2 walls
                   KController();
                   justTurned = 0;  //disable just turned
               }
                
                else if ((sensorValue[LEFTSENSOR] > SIDEWALLMIN && sensorValue[RIGHTSENSOR] > SIDEWALLMIN )
                        && (sensorValue[LEFTFRONTSENSOR] > FRONTWALLMIN && sensorValue[RIGHTFRONTSENSOR] > FRONTWALLMIN) && RotateCounter <= 0) 
				{	// 3 walls
                    RotateCounter = ROTATE90*2;
                    rotate(rotatingSide);
                } 
                    
//				else if (sensorValue[RIGHTSENSOR] < SIDEWALLMIN && algorithm == RIGHTWALL)
//				{
//                    if(justTurned == FALSE)
//                    ForwardCounter = FORWARDFACTOR;
//                    else
//                    ForwardCounter = FORWARDFACTOR2;
//                    
//                    rotatingSide = RIGHT;
//                    justTurned = TRUE;
//					//LTurnCounter = SMOOTHROTATEFACTOR;
//					//smoothTurn();
//				}                
//				else if (sensorValue[LEFTSENSOR] < SIDEWALLMIN && (sensorValue[LEFTFRONTSENSOR] > FRONTWALLMIN || sensorValue[RIGHTFRONTSENSOR] > FRONTWALLMIN)
//                        && sensorValue[RIGHTSENSOR] > SIDEWALLMIN && algorithm == RIGHTWALL)
//				{
//                    if(justTurned == FALSE)
//                    ForwardCounter = FORWARDFACTOR;
//                    else
//                    ForwardCounter = FORWARDFACTOR2;
//                    
//                    rotatingSide = LEFT;
//                    justTurned = TRUE;
					//LTurnCounter = SMOOTHROTATEFACTOR;
					//smoothTurn();
				//}
               else {
              motorCounterUpdate(RIGHT,0);
              motorCounterUpdate(LEFT,0);
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

    //move straight
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
    
    if(diff > KCONTROLLERMAX)    
        controllerSteps = KCONTROLLERSTEPMAX;
    else if(diff <= KCONTROLLERMAX && diff > KCONTROLLERMID)
        controllerSteps = KCONTROLLERSTEPMID;
    else
        controllerSteps = KCONTROLLERSTEP;
}

void rotate(Side side)
{
    if (side == LEFT){
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,1);
    } else {
        motorCounterUpdate(LEFT,0);
        motorCounterUpdate(RIGHT,1);
    }
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

void Reversing()
{
        motorCounterUpdate(RIGHT,1);
        motorCounterUpdate(LEFT,1);
        ReverseCounter--;
}

void forward()
{
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
        ForwardCounter--;
}
