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
#include "math.h"
/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* High-priority service */

/******************************************************************************/

#define MOTORDELAYMIN 0         //the smaller the faster
#define MOTORDELAYMAX 2       //the bigger the slower
#define ROTATE90 170            //steps require for doing 90 turn
#define SMOOTHROTATEFACTOR 5    //factor that the outer/inter steps
#define REVERSEFACTOR 15         //factor that helps correct the 180 turn
#define LEFTFORWARDFACTOR 450          //factor that helps forward till 90 degree turn
#define LEFTFORWARDFACTOR2 410         //factor that helps forward till 90 degree turn
#define RIGHTFORWARDFACTOR 430
#define RIGHTFORWARDFACTOR2 390
#define CONTROLLERFACTOR 3

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

// Higher the value the closer
#define FRONTHASWALL 480         // 2,3 senses distance further than this makes left/right turn
#define FRONTWALLMAX2 800         // 2,3 senses distance closer than this then reverse
#define RIGHTHASWALL 75
#define LEFTHASWALL 80

#define IDEALFRONT 475

#define KCONTROLLERMAX 60        // Maximum diff bettween 0,1 sensors value
#define KCONTROLLERMID 20
#define KCONTROLLERSTEP 10
#define KCONTROLLERSTEPMID 3
#define KCONTROLLERSTEPMAX 1

typedef enum {LEFT,RIGHT} Side;
unsigned char LMotorCounter = 0;
unsigned char RMotorCounter = 0;
unsigned char ignoreControll = KCONTROLLERSTEP;
int MotorDelayCounter = 0;
int LTurnCounter = 0;
int RTurnCounter = 0;
int RotateCounter = 0;
int ReverseCounter = 0;
int ForwardCounter = 0;
int ControllerCounter = 0;
Side rotatingSide = LEFT;
unsigned char justTurned= 0;  
int TurnRight = 0;
int TurnLeft = 0;
int errorP = 0;
int errorD = 0;
int oldErrorP = 0;
int totalError = 0;
int TurnedRight = 0;
int TurnedLeft = 0;
int Deadend = 0;



void motorCounterUpdate(Side,unsigned char);
unsigned char merge(unsigned char,unsigned char);
void moveMouse(unsigned char);
void KController();
void rotate(Side);
void smoothTurn();
void Reversing();
void forward();
void FrontController();

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
                if(ForwardCounter >0)
                    forward();
                else if (ReverseCounter > 0) {    //keep reversing until counter goes to 0
			Reversing();
                }
                else if(RotateCounter > 0) {     //keep rotating until counter goes to 0
                    // If front left or front right has a wall closer than 800, reverse 15 steps
                    if ((sensorValue[LEFTFRONTSENSOR] > FRONTWALLMAX2 || sensorValue[RIGHTFRONTSENSOR] > FRONTWALLMAX2)) {
                        ReverseCounter = REVERSEFACTOR;
                        Reversing();
                    }
                    else if(sensorValue[LEFTFRONTSENSOR] < 100 && sensorValue[RIGHTFRONTSENSOR] < 100)  //else rotate which side?
                    {
                        RotateCounter = 20;
                        rotate(rotatingSide);
                    }
                    else{
                    rotate(rotatingSide);
                    TurnRight = 0;
                    TurnLeft = 0;
                    }
                }
                
                else if (TurnLeft == 1){
                       RotateCounter = ROTATE90;
                       rotatingSide = LEFT;
                       justTurned = 1;   //enable just turned.
                       TurnLeft = 0;
				}

                else if (TurnRight == 1 && sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL - 40 && sensorValue[LEFTFRONTSENSOR] > FRONTHASWALL - 40) {
                     RotateCounter = ROTATE90;
                     rotatingSide = RIGHT;
                     justTurned = 1;
                     TurnRight = 0;
                }
                else if ((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL)
                        && sensorValue[RIGHTFRONTSENSOR] < 420 && sensorValue[LEFTFRONTSENSOR] < 420 ) { //use 100 NOT WORKING

                    KController();
                         justTurned = 0;

                }
		else if (sensorValue[LEFTSENSOR] < LEFTHASWALL && algorithm == LEFTWALL) {   // Always left turn if left sensor senses less than 70 in upper left direc
                    TurnLeft = 1;
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = LEFTFORWARDFACTOR;
                    }else {  //if not just rotate, forward step of 420.
                    ForwardCounter = LEFTFORWARDFACTOR2;
                    }
                    forward();   //forward with given forward factor
                }

		else if (sensorValue[RIGHTSENSOR] < RIGHTHASWALL && sensorValue[LEFTSENSOR] > LEFTHASWALL && algorithm == LEFTWALL)   {
                    TurnRight = 1;
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = RIGHTFORWARDFACTOR;
                    } else  {  //if not just rotate, forward step of 420.
                    ForwardCounter = RIGHTFORWARDFACTOR2;
                    }
                    forward();
                }

                else if ((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL )
                        && (sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL && sensorValue[LEFTFRONTSENSOR] > FRONTHASWALL) && RotateCounter <= 0)
				{	// 3 walls
                    RotateCounter = ROTATE90*2;
                    rotatingSide = LEFT;
                    justTurned = 1;
                }

                else {
                    motorCounterUpdate(RIGHT,0);
                    motorCounterUpdate(LEFT,0);
                    justTurned = 0;
                    TurnLeft = 0;
                    TurnRight = 0;
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
    int diff = 0;
    if(ignoreControll > 0){   
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
        ignoreControll--;
        return;
    }
    
    diff = ABS(sensorValue[RIGHTSENSOR] - sensorValue[LEFTSENSOR]);
       // errorD = errorP - oldErrorP;
    if(sensorValue[LEFTSENSOR] > sensorValue[RIGHTSENSOR])
    {
        correctTo = LEFT;
        motorCounterUpdate(correctTo,0);

    }
    else
    {
        correctTo = RIGHT;
        motorCounterUpdate(correctTo,0);
    }
    
    if(diff > KCONTROLLERMAX)    
        ignoreControll = KCONTROLLERSTEPMAX;
    else if(diff <= KCONTROLLERMAX && diff > KCONTROLLERMID)
        ignoreControll = KCONTROLLERSTEPMID;
    else
        ignoreControll = KCONTROLLERSTEP;
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
                // Go forward and self correct if upper left and upper right have walls and no wall in the front
                //if (right sensor and left sensor senses greater than 70) && (either front sensors senses less than 420)
     
//                else if ((sensorValue[LEFTFRONTSENSOR] > 110 && sensorValue[RIGHTFRONTSENSOR] > 45) && TurnedLeft == 1) {
//                        RotateCounter = ROTATE90;
//                        rotatingSide = RIGHT;
//                        justTurned = 1;
//                        TurnedRight = 1;
//                    }
//                else if ((sensorValue[LEFTFRONTSENSOR] > 110 && sensorValue[RIGHTFRONTSENSOR] > 45) && TurnedRight == 1) {
//                            RotateCounter = ROTATE90;
//                        rotatingSide = LEFT;
//                        justTurned = 1;
//                        TurnedLeft = 1;
//                        }
void forward()
{
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
        ForwardCounter--;
}