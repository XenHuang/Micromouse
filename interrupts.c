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
<<<<<<< HEAD
#define FORWARDFACTOR 320          //factor that helps forward till 90 degree turn
#define FORWARDFACTORAFTERTURN 50         //factor that helps forward till 90 degree turn
#define FORWARDFACTOR2 440
=======
#define FORWARDFACTOR 350         //factor that helps forward till 90 degree turn
#define FORWARDFACTORAFTERTURN 350         //factor that helps forward till 90 degree turn
>>>>>>> 6e84ce29399cc215944b2c4145dd86812139ce4b

#define CONTROLLERFACTOR 3

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

// Higher the value the closer
#define FRONTHASWALL 300         // 2,3 senses distance further than this makes left/right turn
#define FRONTWALLMAX2 800         // 2,3 senses distance closer than this then reverse
#define RIGHTHASWALL 200
#define LEFTHASWALL 200      //the higher, higher chance to turn, more errors tho
#define SIDENEEDCORRECTION 400
#define FRONTNEEDCORRECTION 200

#define IDEALFRONT 475

#define KCONTROLLERMAX 30        // Maximum diff bettween 0,1 sensors value
#define KCONTROLLERMID 10
#define KCONTROLLERSTEP 10
#define KCONTROLLERSTEPMID 3
#define KCONTROLLERSTEPMAX 1

typedef enum {LEFT,RIGHT} Side;
unsigned char LMotorCounter = 0;
unsigned char RMotorCounter = 0;
unsigned char ignoreControll = KCONTROLLERSTEP;
unsigned char deadEnd = 0;
typedef enum {empty,front,twoside,threeside,leftside,rightside,frontleft,frontright} State;
unsigned char ChangeRequired = 0;
State cState = twoside;
State pState = twoside;
int controlToLeft = 0;
int controlToRight = 0;
int MotorDelayCounter = 0;
int LTurnCounter = 0;
int RTurnCounter = 0;
int RotateCounter = 0;
int ReverseCounter = 0;
int ForwardCounter = 0;
Side rotatingSide = LEFT;
unsigned char justTurned= 0;  
int TurnRight = 0;
int TurnLeft = 0;
int oldErrorP = 0;
int TurnedRight = 0;
int TurnedLeft = 0;
int Deadend = 0;
int TurnLeftAgain = 0;
int Forwarded = 0;



void motorCounterUpdate(Side,unsigned char);
unsigned char merge(unsigned char,unsigned char);
void moveMouse(unsigned char);
void KController();
void rotate(Side);
void smoothTurn();
void Reversing();
void forward();
void singleForward();
void initialRotation(Side side,int isDeadEnd);
void FrontController();
void stateUpdate();

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
                stateUpdate();

                if(cState != pState)
                    ChangeRequired = 1;
                else
                    ChangeRequired = 0;

                if(ChangeRequired == 1 && RotateCounter == 0)
                {
                    if(sensorValue[LEFTFRONTSENSOR] < FRONTHASWALL-50 && sensorValue[RIGHTFRONTSENSOR] < FRONTHASWALL-50)
                    {
                        if(justTurned == 1)
                            ForwardCounter = FORWARDFACTORAFTERTURN;
                        else
                            ForwardCounter = FORWARDFACTOR;
                    } else
                        ForwardCounter = 1;
                    forward();
                    pState = cState;
                    ChangeRequired = 0;
                }
                else if(RotateCounter > 0) {
                    if(sensorValue[LEFTFRONTSENSOR] < 100 && sensorValue[RIGHTFRONTSENSOR] < 100 && deadEnd == 1)
                         RotateCounter = 5;
                    rotate(rotatingSide);
                    TurnRight = 0;
                    TurnLeft = 0;
                }
                else if(controlToLeft > 0 || controlToRight > 0 || ForwardCounter > 0)
                {
                    KController();
                    
                    if(controlToLeft > 0 )
                    {
                        motorCounterUpdate(RIGHT,0);
                        controlToLeft--;
                    } else if(controlToRight > 0){
                        motorCounterUpdate(LEFT,0);
                        controlToRight--;
                    } else {
                        forward();
                    }
<<<<<<< HEAD
                }
                
                else if (ReverseCounter > 0) {    //keep reversing until counter goes to 0
                    Reversing();
                }
                
                else if(RotateCounter > 0) {     //keep rotating until counter goes to 0
//                    // If front left or front right has a wall closer than 800, reverse 15 steps
//                    if ((sensorValue[LEFTFRONTSENSOR] > FRONTWALLMAX2 || sensorValue[RIGHTFRONTSENSOR] > FRONTWALLMAX2)) {
//                     //   ReverseCounter = REVERSEFACTOR;
////                        Reversing();
//                    } else {  //else rotate which side?
                    if(sensorValue[LEFTFRONTSENSOR] < 100 && sensorValue[RIGHTFRONTSENSOR] < 100 && deadEnd == 1)
                            RotateCounter = 3;
                    rotate(rotatingSide);
                    if(RotateCounter == 0)
                    ForwardCounter = FORWARDFACTORAFTERTURN;
                    TurnRight = 0;
                    TurnLeft = 0;
                      
                }
                else if (TurnLeft == 1 && sensorValue[LEFTSENSOR] < LEFTHASWALL) {
                       RotateCounter = ROTATE90;
                       rotatingSide = LEFT;
                       justTurned = 1;   //enable just turned.
                       TurnLeft = 0;
                       deadEnd = 0;
				}
                
                else if (TurnLeftAgain == 1 && sensorValue[LEFTSENSOR] < LEFTHASWALL && (TurnRight == 0 || TurnRight != 0) ) {
                       RotateCounter = ROTATE90;
                       rotatingSide = LEFT;
                       justTurned = 1;   //enable just turned.
                       TurnLeft = 0;
                       deadEnd = 0;
                       TurnLeftAgain = 0;
				}
                 else if (sensorValue[LEFTSENSOR] < LEFTHASWALL && TurnRight == 1)  {
                       RotateCounter = ROTATE90;
                       rotatingSide = LEFT;
                       justTurned = 1;   //enable just turned.
                       TurnLeft = 0;
                       deadEnd = 0;
                       TurnLeftAgain = 0;
				}
                else if (TurnRight == 1 && sensorValue[LEFTSENSOR] < LEFTHASWALL && TurnLeftAgain != 1){
                    TurnLeftAgain = 1;
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = FORWARDFACTORAFTERTURN + 40;
                    }else {  //if not just rotate, forward step of 420.
                    ForwardCounter = FORWARDFACTORAFTERTURN;
                    }
                    forward();   //forward with given forward factor
                }


                else if (TurnRight == 1
                        && (sensorValue[RIGHTFRONTSENSOR] > 250 && sensorValue[LEFTFRONTSENSOR] > 250) && sensorValue[LEFTSENSOR] > LEFTHASWALL) {
                     RotateCounter = ROTATE90;
                     rotatingSide = RIGHT;
                     justTurned = 1;
                     TurnRight = 0;
                     deadEnd = 0;
                }
//                else if (TurnRight == 1 && sensorValue[LEFTSENSOR] > LEFTHASWALL  && (sensorValue[RIGHTFRONTSENSOR] < 250 && sensorValue[LEFTFRONTSENSOR] < 250)) {
//                    justTurned = 0;
//                    TurnRight = 0;
//                    if(justTurned == 0) { 
//                     ForwardCounter = FORWARDFACTOR2;
//                    }else {  //if not just rotate, forward step of 420.
//                    ForwardCounter = FORWARDFACTORAFTERTURN;
//                    }
//                    forward();   //forward with given forward factor
//                }
//                else if (TurnRight == 1 && sensorValue[LEFTSENSOR] < LEFTHASWALL){
//                    TurnLeftAgain = 1;
//                    TurnRight = 0;
//                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
//                    ForwardCounter = FORWARDFACTOR;
//                    }else {  //if not just rotate, forward step of 420.
//                    ForwardCounter = FORWARDFACTORAFTERTURN;
//                    }
//                    forward();   //forward with given forward factor
//                }

                else if (sensorValue[LEFTSENSOR] < LEFTHASWALL && TurnRight != 1) {   // Always left turn if left sensor senses less than 70 in upper left direc
                    TurnLeft = 1;
                    if(justTurned == 0) { //if just rotate, forward lower step of 380.
                    ForwardCounter = FORWARDFACTOR;
                    }else {  //if not just rotate, forward step of 420.
                    ForwardCounter = FORWARDFACTORAFTERTURN;
=======

                } else if(ForwardCounter == 0)
                {
                    if(algorithm == LEFTWALL)
                    {
                        if(cState == empty)             initialRotation(LEFT,0);
                        else if(cState == twoside)      singleForward();
                        else if(cState == threeside)    initialRotation(LEFT,1);
                        else if(cState == leftside)     singleForward();
                        else if(cState == rightside)    initialRotation(LEFT,0);
                        else if(cState == frontleft)    initialRotation(RIGHT,0);
                        else if(cState == frontright)   initialRotation(LEFT,0);
                        else if(cState == front)        initialRotation(LEFT,0);
                    } else {
                        if(cState == empty)             initialRotation(RIGHT,0);
                        else if(cState == twoside)      singleForward();
                        else if(cState == threeside)    initialRotation(RIGHT,1);
                        else if(cState == leftside)     initialRotation(RIGHT,0);
                        else if(cState == rightside)    singleForward();
                        else if(cState == frontleft)    initialRotation(RIGHT,0);
                        else if(cState == frontright)   initialRotation(LEFT,0);
                        else if(cState == front)        initialRotation(RIGHT,0);
>>>>>>> 6e84ce29399cc215944b2c4145dd86812139ce4b
                    }
                }

<<<<<<< HEAD
		else if (sensorValue[RIGHTSENSOR] < RIGHTHASWALL && sensorValue[LEFTSENSOR] > LEFTHASWALL && TurnRight == 0)   {
                    TurnRight = 1;;
                    if(justTurned == 0 && Forwarded != 1) { //if just rotate, forward lower step of 380.
                    ForwardCounter = FORWARDFACTOR;
                    Forwarded = 1;
                    } else if (Forwarded = 1){
                        ForwardCounter = FORWARDFACTOR + 90;
                    }
                    else  {  //if not just rotate, forward step of 420.
                    ForwardCounter = FORWARDFACTORAFTERTURN;
                    }
                    forward();
                }
                
                else if ((sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL )
                        && (sensorValue[RIGHTFRONTSENSOR] > 450 || sensorValue[LEFTFRONTSENSOR] > 450) && RotateCounter <= 0)
		{	// 3 walls
                    RotateCounter = ROTATE90*2;
                    rotatingSide = LEFT;
                    justTurned = 1;
                    deadEnd = 1;
                    TurnLeftAgain=0;
                }
                
                else if ((sensorValue[LEFTSENSOR] > SIDENEEDCORRECTION || sensorValue[RIGHTSENSOR] > SIDENEEDCORRECTION)
                        && (sensorValue[RIGHTFRONTSENSOR] < FRONTNEEDCORRECTION && sensorValue[LEFTFRONTSENSOR] < FRONTNEEDCORRECTION))
                {
                  
                    KController();
                         justTurned = 0;
                    
                }
               else {
                    motorCounterUpdate(RIGHT,0);
                    motorCounterUpdate(LEFT,0);
                    justTurned = 0;
                    TurnLeft = 0;
                    TurnRight = 0;
                    TurnLeftAgain = 0;
               }
=======
>>>>>>> 6e84ce29399cc215944b2c4145dd86812139ce4b
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
//    if(sensorValue[LEFTFRONTSENSOR] > 150 && sensorValue[RIGHTFRONTSENSOR] > 150 &&
<<<<<<< HEAD
//        sensorValue[LEFTFRONTSENSOR] < 300 && sensorValue[RIGHTFRONTSENSOR] < 300 )
//    {
//        if(sensorValue[LEFTFRONTSENSOR] < sensorValue[RIGHTFRONTSENSOR])
//            controlToLeft = 1;
//        else
//            controlToRight = 1;
//          
//} else 
    if(sensorValue[LEFTFRONTSENSOR] < 150 || sensorValue[RIGHTFRONTSENSOR] < 150 )
=======
//        sensorValue[LEFTFRONTSENSOR] < FRONTNEEDCORRECTION && sensorValue[RIGHTFRONTSENSOR] < FRONTNEEDCORRECTION )
//    {
//        if(ABS(sensorValue[LEFTFRONTSENSOR] - sensorValue[RIGHTFRONTSENSOR]) > 50)
//            controlToLeft = 1;
//        else
//            controlToRight = 1;
//
//    } else
        if(sensorValue[LEFTFRONTSENSOR] < 150 || sensorValue[RIGHTFRONTSENSOR] < 150 )
>>>>>>> 6e84ce29399cc215944b2c4145dd86812139ce4b
    {

        if(sensorValue[LEFTSENSOR] > SIDENEEDCORRECTION)
        {
            controlToRight = 30;
        } else if(sensorValue[RIGHTSENSOR] > SIDENEEDCORRECTION)
        {
            controlToLeft = 30;
        }
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
        if(ForwardCounter == 0)
        justTurned = 0;
}


void singleForward()
{
    if(ForwardCounter == 0 || ForwardCounter == 1)
    {
        ForwardCounter = 1;
        forward();
    }
}


void initialRotation(Side side,int isDeadEnd)
{
    if(isDeadEnd == 1)
    {
        RotateCounter = ROTATE90*2;
        deadEnd = 1;
    }
    else
    {
        RotateCounter = ROTATE90;
        deadEnd = 0;
    }

    rotatingSide = side;
    justTurned = 1;
    rotate(rotatingSide);

}


void stateUpdate()
{
    if(sensorValue[LEFTSENSOR] < LEFTHASWALL && sensorValue[RIGHTSENSOR] < RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] < FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] < FRONTHASWALL))
        cState = empty;
    else if(sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] < FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] < FRONTHASWALL))
        cState = twoside;
    else if(sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] > FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL))
        cState = threeside;
    else if(sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] < RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] < FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] < FRONTHASWALL))
        cState = leftside;
    else if(sensorValue[LEFTSENSOR] < LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] < FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] < FRONTHASWALL))
        cState = rightside;
    else if(sensorValue[LEFTSENSOR] > LEFTHASWALL && sensorValue[RIGHTSENSOR] < RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] > FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL))
        cState = frontleft;
    else if(sensorValue[LEFTSENSOR] < LEFTHASWALL && sensorValue[RIGHTSENSOR] > RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] > FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL))
        cState = frontright;
    else if(sensorValue[LEFTSENSOR] < LEFTHASWALL && sensorValue[RIGHTSENSOR] < RIGHTHASWALL &&
            (sensorValue[LEFTFRONTSENSOR] > FRONTHASWALL && sensorValue[RIGHTFRONTSENSOR] > FRONTHASWALL))
        cState = front;
}