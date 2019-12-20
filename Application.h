

#ifndef APP_H_
#define APP_H_

#include "FreeRTOS.h"
#include "task.h"
#include "DIO.h"
#include "Switch.h"
#include "LED.h"
#include "LCD.h"
#include "Keypad.h"
#include "UART.h"

#define BUFFER_SIZE   16

//----------- Section of Public Constant Macros --------------------------------//

typedef struct
{
	uint8 Data[BUFFER_SIZE];
	sint8 Index;
} S_Create_Buffer_t;

extern S_Create_Buffer_t  Buffer;

//------------ Section of Structure  -------------------------------------------//


//------------ Section of enum  -------------------------------------------//
typedef enum {LED_NO_MODE,LED_OFF_MODE, LED_ON_MODE}LED_Display_Mode_t;


//----------- Section of GLobal Variables ---------------------------------------//

extern TaskHandle_t LCD_Handle;        //The Handle we use to control the LCD Task (Suspend/Resume)
extern TaskHandle_t Switch_Handle;	    //The Handle we use to control the Keypad Task
extern TaskHandle_t Keypad_Handle;	    //The Handle we use to control the Keypad Task
extern TaskHandle_t Display_Handle;	//The Handle we use to control the Display Task
extern TaskHandle_t UART_Send_Handle;	//The Handle we use to control the UART_Send Task
extern TaskHandle_t LED_Handle;		//The Handle we use to control the LED Task


//----------- Section of Public Prototypes of Functions and Tasks ---------------//

void TASK_Init (void * POINTERVOID );
void Task_LCD (void * POINTERVOID);
void Task_Keypad (void * POINTERVOID);
void Task_Display (void * POINTERVOID);
void Task_SWITCH (void * POINTERVOID);
void Task_UART_Transceiver (void * POINTERVOID);
void Task_LED (void * POINTERVOID);

void Reset_TX_System (void);
void Reset_RX_System (void);
void UART_Recieve_Data (void);

#endif /* APP_H_ */