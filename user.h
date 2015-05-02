/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

/* TODO Application specific user parameters used in user.c may go here */
#define _XTAL_FREQ 32000000

#define SENSORCOMPUTATION 10    //the number of times averaging sensor value, min = 3
#define NUMSENSORS 4
#define LEFTSENSORL 0b00011011         //ADCON0 value
#define RIGHTSENSORL 0b00101011        //ADCON0 value
#define LEFTFRONTSENSORL 0b00011111    //ADCON0 value
#define RIGHTFRONTSENSORL 0b00110011   //ADCON0 value

const unsigned char sensorLocation[NUMSENSORS] = {LEFTSENSORL,RIGHTSENSORL,LEFTFRONTSENSORL,RIGHTFRONTSENSORL};

typedef enum {LEFTWALL,RIGHTWALL} Algorithm;
Algorithm algorithm;

int delay_time = 0;
int sensorCorrection[NUMSENSORS];
int sensorValue[NUMSENSORS];


/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

/* TODO User level functions prototypes (i.e. InitApp) go here */
void delay(unsigned int);
void InitApp(void);         /* I/O and Peripheral Initialization */
void sensorUpdate(unsigned char);
void sensorComputation(int[][SENSORCOMPUTATION],unsigned char);
int MIN(int,int);
int MAX(int,int);
void AlgorithmSelection();
void setCorrection();
