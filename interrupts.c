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
unsigned char MOTORDELAYMAX  = 2;       //the bigger the slower
#define ROTATE90 170            //steps require for doing 90 turn
#define SMOOTHROTATEFACTOR 5    //factor that the outer/inter steps
#define REVERSEFACTOR 5         //factor that helps correct the 180 turn
#define FORWARDFACTOR 320         //factor that helps forward till 90 degree turn
#define FORWARDFACTORAFTERTURN 350         //factor that helps forward till 90 degree turn
#define FORWARDUPDATESTATE 220

#define CONTROLLERFACTOR 3

#define LEFTSENSOR 0             //Sensor value position
#define RIGHTSENSOR 1            //Sensor value position
#define LEFTFRONTSENSOR 2        //Sensor value position
#define RIGHTFRONTSENSOR 3       //Sensor value position

// Higher the value the closer
#define LFRONTMIN 72
#define RFRONTMIN 178
#define LFRONT20WALL 913
#define RFRONT20WALL 912
#define LFRONT55WALL 282         
#define RFRONT55WALL 424
#define LFRONT60WALL 238
#define RFRONT60WALL 378
#define LFRONT65WALL 204
#define RFRONT65WALL 338
#define LFRONT70WALL 177
#define RFRONT70WALL 304
#define LFRONT75WALL 159
#define RFRONT75WALL 274
#define LFRONT80WALL 141
#define RFRONT80WALL 260
#define LFRONT85WALL 128
#define RFRONT85WALL 239
#define L50WALL 398
#define R50WALL 387
#define L55WALL 356
#define R55WALL 340
#define L60WALL 321
#define R60WALL 302
#define L65WALL 295
#define R65WALL 273
#define L70WALL 274
#define R70WALL 245
#define L75WALL 256
#define R75WALL 224
#define L80WALL 242
#define R80WALL 207
#define L85WALL 230
#define R85WALL 193
#define L90WALL 217
#define R90WALL 181
#define L95WALL 209
#define R95WALL 172
#define L100WALL 199
#define R100WALL 162

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
State doState = twoside;
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
int Deadend = 0;



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

                //for debug
                if(RotateCounter == ROTATE90*2-1 || RotateCounter == ROTATE90-1)
                {
                    int a = 0;
                }

                if (ReverseCounter > 0) {
                    Reversing();
                }
                else if((ChangeRequired == 1 && RotateCounter == 0) && ForwardCounter != FORWARDUPDATESTATE)
                {
                    if(sensorValue[LEFTFRONTSENSOR] < LFRONT75WALL && sensorValue[RIGHTFRONTSENSOR] < RFRONT75WALL && cState != threeside)
                    {
                        if(justTurned == 1)
                            ForwardCounter = FORWARDFACTORAFTERTURN;
                        else
                            ForwardCounter = FORWARDFACTOR;
                    } else
                    {
                        ForwardCounter = 1;
                        doState = cState;
                    }
                    forward();
                    pState = cState;
                    ChangeRequired = 0;
                }
                else if(RotateCounter > 0) {
                    if ((sensorValue[LEFTFRONTSENSOR] > LFRONT20WALL || sensorValue[RIGHTFRONTSENSOR] > RFRONT20WALL))
                    {
                        ReverseCounter = REVERSEFACTOR;
                        Reversing();
                    } else {
                        rotate(rotatingSide);
                        if(RotateCounter == 0)
                        {
                            justTurned = 1;
                            if(deadEnd == 1)
                            {
                                ReverseCounter = REVERSEFACTOR*3;
                            }
                        }
                    }
                }
                else if(controlToLeft > 0 || controlToRight > 0 || ForwardCounter > 0)
                {
                    KController();
                    if(ForwardCounter == FORWARDUPDATESTATE)
                        doState = cState;

                    if(controlToLeft > 0 )
                    {
                        motorCounterUpdate(RIGHT,0);
                        controlToLeft--;
                        ForwardCounter--;
                    } else if(controlToRight > 0){
                        motorCounterUpdate(LEFT,0);
                        ForwardCounter--;
                        controlToRight--;
                    } else {
                        forward();
                    }

                }
                else if(ForwardCounter == 0)
                {
                    KController();
                    if(algorithm == LEFTWALL)
                    {
                        //if turned then must forward
                        if(justTurned == 1 && doState != twoside && doState != leftside)
                                ForwardCounter = FORWARDFACTOR;
                        else{
                            if(doState == empty)             initialRotation(LEFT,0);
                            else if(doState == twoside)      singleForward();
                            else if(doState == threeside)    initialRotation(LEFT,1);
                            else if(doState == leftside)     singleForward();
                            else if(doState == rightside)    initialRotation(LEFT,0);
                            else if(doState == frontleft)    initialRotation(RIGHT,0);
                            else if(doState == frontright)   initialRotation(LEFT,0);
                            else if(doState == front)        initialRotation(LEFT,0);
                        }
                    } else {
                        if(justTurned == 1 && doState != twoside && doState != rightside)
                            ForwardCounter = FORWARDFACTOR;
                        else{
                            if(doState == empty)             initialRotation(RIGHT,0);
                            else if(doState == twoside)      singleForward();
                            else if(doState == threeside)    initialRotation(RIGHT,1);
                            else if(doState == leftside)     initialRotation(RIGHT,0);
                            else if(doState == rightside)    singleForward();
                            else if(doState == frontleft)    initialRotation(RIGHT,0);
                            else if(doState == frontright)   initialRotation(LEFT,0);
                            else if(doState == front)        initialRotation(RIGHT,0);
                        }
                    }
                } else {
                    KController();
                    forward();
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
        if(sensorValue[LEFTFRONTSENSOR] < LFRONT65WALL || sensorValue[RIGHTFRONTSENSOR] < RFRONT65WALL )
    {

        if(sensorValue[LEFTSENSOR] > L60WALL)
        {
            controlToRight = 5;
        } else if(sensorValue[RIGHTSENSOR] > R60WALL)
        {
            controlToLeft = 5;
        }
//        else if(cState == leftside && sensorValue[LEFTSENSOR] > L70WALL && sensorValue[LEFTSENSOR] < L65WALL )
//        {
//                controlToLeft = 2;
//        }
//        else if(cState == rightside && sensorValue[RIGHTSENSOR] > R70WALL && sensorValue[RIGHTSENSOR] < R65WALL )
//        {
//                controlToRight = 2;
//        }


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
    MOTORDELAYMAX = 2;
    if(ForwardCounter == 0 || ForwardCounter == 1)
    {
        ForwardCounter = 1;
        forward();
    }
}


void initialRotation(Side side,int isDeadEnd)
{
    MOTORDELAYMAX=2;
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
    rotate(rotatingSide);

}


void stateUpdate()
{
    if(sensorValue[LEFTSENSOR] < L100WALL && sensorValue[RIGHTSENSOR] < R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] < LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] < RFRONT70WALL))
        cState = empty;
    else if(sensorValue[LEFTSENSOR] > L100WALL && sensorValue[RIGHTSENSOR] > R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] < LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] < RFRONT70WALL))
        cState = twoside;
    else if(sensorValue[LEFTSENSOR] > L100WALL && sensorValue[RIGHTSENSOR] > R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] > LFRONT55WALL && sensorValue[RIGHTFRONTSENSOR] > RFRONT55WALL))
        cState = threeside;
    else if(sensorValue[LEFTSENSOR] > L100WALL && sensorValue[RIGHTSENSOR] < R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] < LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] < RFRONT70WALL))
        cState = leftside;
    else if(sensorValue[LEFTSENSOR] < L100WALL && sensorValue[RIGHTSENSOR] > R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] < LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] < RFRONT70WALL))
        cState = rightside;
    else if(sensorValue[LEFTSENSOR] > L100WALL && sensorValue[RIGHTSENSOR] < R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] > LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] > RFRONT70WALL))
        cState = frontleft;
    else if(sensorValue[LEFTSENSOR] < L100WALL && sensorValue[RIGHTSENSOR] > R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] > LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] > RFRONT70WALL))
        cState = frontright;
    else if(sensorValue[LEFTSENSOR] < L100WALL && sensorValue[RIGHTSENSOR] < R100WALL &&
            (sensorValue[LEFTFRONTSENSOR] > LFRONT70WALL && sensorValue[RIGHTFRONTSENSOR] > RFRONT70WALL))
        cState = front;
}