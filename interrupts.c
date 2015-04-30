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
#define ROTATE90 167            //steps require for doing 90 turn
#define SMOOTHROTATEFACTOR 5    //factor that the outer/inter steps
#define REVERSEFACTOR 15         //factor that helps correct the 180 turn
#define LEFTFORWARDFACTOR 420          //factor that helps forward till 90 degree turn
#define LEFTFORWARDFACTOR2 400         //factor that helps forward till 90 degree turn
#define RIGHTFORWARDFACTOR 450
#define RIGHTFORWARDFACTOR2 410

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

// Higher the value the closer
#define FRONTWALLMIN 420          // 2,3 senses distance closer than this rotates
#define FRONTHASWALL 380          // 2,3 senses distance further than this makes left/right turn
#define WALLINONEHALFCELL 75         // Is there a frontwall after one and half cell.
#define SIDEWALLMIN 200           // 0,1 senses distance further than this makes left/right turn
#define SIDEWALLMIN2 100
#define FRONTWALLMAX2 800         // 2,3 senses distance closer than this then reverse
#define RIGHTHASWALL 91
#define LEFTHASWALL 95

#define KCONTROLLERMAX 80        // Maximum diff bettween 0,1 sensors value
#define KCONTROLLERMID 30
#define KCONTROLLERSTEP 10
#define KCONTROLLERSTEPMID 5
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
unsigned char justTurned= 0;  
int TurnRight = 200;
int TurnLeft = 200;
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
                if (ForwardCounter > 0)   {   //keep forwarding until counter goes to 0
                    forward();
                }
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
            //   else if (sensorValue[LEFTSENSOR] > 90 && sensorValue[RIGHTSENSOR] > 90 && (sensorValue[LEFTFRONTSENSOR] < 250 && sensorValue[RIGHTFRONTSENSOR] < 250) )
                           
              //  {	// 2 walls
               //    KController();
             //      justTurned = 0;  //disable just turned
             //  }
				else if (sensorValue[LEFTSENSOR] < LEFTHASWALL && TurnLeft > LEFTHASWALL && algorithm == LEFTWALL)    // Always left turn if left sensor senses less than 70 in upper left direction.
				{
                    TurnLeft = sensorValue[LEFTSENSOR];
//                   if ((sensorValue[LEFTFRONTSENSOR] < 100 && sensorValue[RIGHTFRONTSENSOR] < 65)) {
//                    KController();
//                    }
//                    else {
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = LEFTFORWARDFACTOR;
                    }else {  //if not just rotate, forward step of 420.
                    ForwardCounter = LEFTFORWARDFACTOR2;
                    }
                    forward();   //forward with given forward factor
                    }
                
                else if (TurnLeft < LEFTHASWALL) {
                    RotateCounter = ROTATE90;
                    rotatingSide = LEFT;
                    justTurned = 1;   //enable just turned.
                    TurnRight= 200;
                    TurnLeft = 300;
                    TurnedLeft = 1;
					//RTurnCounter = SMOOTHROTATEFACTOR;
					//smoothTurn();
                    
				}
                //Right turn if upper left has wall && front has wall in 1.5 cells && upper right has no wall.
                //if (right sensor senses less than 70) && (both front sensor senses greater than 100) && (left sensor senses higher than 70) 
				else if (sensorValue[RIGHTSENSOR] < RIGHTHASWALL && algorithm == LEFTWALL)   
				{
                    TurnRight = sensorValue[RIGHTSENSOR];
//                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
//                    ForwardCounter = RIGHTFORWARDFACTOR;
//                    } else  {  //if not just rotate, forward step of 420.
//                    ForwardCounter = RIGHTFORWARDFACTOR2;
//                    }
//                    forward();   //forward with given forward factor
//                    if ((sensorValue[LEFTFRONTSENSOR] < 100 && sensorValue[RIGHTFRONTSENSOR] < 65)) {
//                    KController();
//                    }
//                    else {
                      if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = RIGHTFORWARDFACTOR;
                    } else  {  //if not just rotate, forward step of 420.
                    ForwardCounter = RIGHTFORWARDFACTOR2;
                    }
                    forward();
                }
					//RTurnCounter = SMOOTHROTATEFACTOR;
					//smoothTurn();
				
                else if (TurnRight < RIGHTHASWALL && sensorValue[RIGHTFRONTSENSOR] > 380) {
                    RotateCounter = ROTATE90;          
                    rotatingSide = RIGHT;
                     justTurned = 1;
                     TurnedRight = 1;
                     TurnLeft = 300;
                     TurnRight = 300;
                }
                else if (TurnRight < RIGHTHASWALL && sensorValue[RIGHTFRONTSENSOR] < 380) {
                    KController();
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
                else if ((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL )
                        && (sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL) && RotateCounter <= 0) 
				{	// 3 walls
                    Deadend = 1;
                    RotateCounter = ROTATE90*2;
                    rotate(rotatingSide);
                    justTurned = 1;
                    TurnRight= 200;
                    TurnLeft = 300;
                    TurnedLeft = 0;
                    TurnedRight = 0;
                } 
                else if ((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL))
                {
                    if (sensorValue[RIGHTFRONTSENSOR] < 80 || sensorValue[LEFTFRONTSENSOR] < 120){
                    KController();
                    }
                    else {
                                motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
                    }
                        
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
                    justTurned = 0;
                    TurnedLeft = 0;
                    TurnedRight = 0;
                    TurnRight = 200;
                    TurnLeft = 300;
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
    if((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL))//has both walls
    {  
        diff = ABS(sensorValue[RIGHTSENSOR] - sensorValue[LEFTSENSOR]);
       // errorD = errorP - oldErrorP;
    if(controllerSteps > 0)
    {   
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
        controllerSteps--;
        return;
    }

    if(sensorValue[LEFTSENSOR] > sensorValue[RIGHTSENSOR])
    {
        //controllerSteps=diff/4;
        correctTo = LEFT;
        motorCounterUpdate(correctTo,0);
        moveMouse(merge(LMotorCounter,RMotorCounter));
    }
    else
    {
       // controllerSteps=diff/4;
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
    else if((sensorValue[LEFTSENSOR] > LEFTHASWALL))//only has left wall
    {
       // diff = ABS(2 * (131 - sensorValue[LEFTSENSOR]));

    }
//    else if((sensorValue[RIGHTSENSOR] > RIGHTHASWALL))//only has right wall
//    {
//        diff = 2 * (sensorValue[RIGHTSENSOR] - 141);
//            if(controllerSteps > 0)
//    {   
//        motorCounterUpdate(RIGHT,0);
//        motorCounterUpdate(LEFT,0);
//        controllerSteps--;
//        return;
//    }
//
//    if(sensorValue[LEFTSENSOR] > sensorValue[RIGHTSENSOR])
//    {
//        //controllerSteps=diff/4;
//        correctTo = LEFT;
//        motorCounterUpdate(correctTo,0);
//        moveMouse(merge(LMotorCounter,RMotorCounter));
//    }
//    else
//    {
//       // controllerSteps=diff/4;
//        correctTo = RIGHT;
//        motorCounterUpdate(correctTo,0);
//        moveMouse(merge(LMotorCounter,RMotorCounter));
//    }
//    
//    if(diff > KCONTROLLERMAX)    
//        controllerSteps = KCONTROLLERSTEPMAX;
//    else if(diff <= KCONTROLLERMAX && diff > KCONTROLLERMID)
//        controllerSteps = KCONTROLLERSTEPMID;
//    else
//        controllerSteps = KCONTROLLERSTEP;
//
//    }
    else if(((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL) && sensorValue[RIGHTFRONTSENSOR] > 80))//no wall, use encoder or gyro
    {
         diff = ABS(sensorValue[RIGHTFRONTSENSOR] - sensorValue[LEFTFRONTSENSOR]);
       // errorD = errorP - oldErrorP;
    if(controllerSteps > 0)
    {   
        motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
        controllerSteps--;
        return;
    }

    if(sensorValue[LEFTFRONTSENSOR] > sensorValue[RIGHTFRONTSENSOR])
    {
        //controllerSteps=diff/4;
        correctTo = LEFT;
        motorCounterUpdate(correctTo,0);
        moveMouse(merge(LMotorCounter,RMotorCounter));
    }
    else
    {
       // controllerSteps=diff/4;
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
    else {
                motorCounterUpdate(RIGHT,0);
        motorCounterUpdate(LEFT,0);
    }
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