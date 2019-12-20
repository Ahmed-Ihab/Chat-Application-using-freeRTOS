
#include "Application.h"

//----------- Section of Private Constant Macros ---------------//


#define AVAILABLE	  1
#define NOT_AVAILABLE 0
#define BUSY		  0

#define NEW_REQUEST	  1	
#define REQUEST_DONE  0
#define NO_REQUEST	  0

#define NOT_SENDING	  0
#define SENDING_DATA  1


#define NOTHING_TO_SEND  0
#define SENDING_COMPLETE 1

#define NOT_READY	0
#define READY		1

#define  NOT_COMPLETED    0
#define  COMPLETED		  1

#define NOT_RECEIVING	0 
#define RECEIVING	    1
#define IDLE			2
#define HOLDING_RX_DATA 3


#define NO_PRINTING		0
#define PRINT_ONCE		1


/************************* ----------- Section of Static Private Global Variables ------------ *******************/


//------------------------------ Section of creating variables from structure  ---------------------//

S_Create_Buffer_t TX_Buffer;
S_Create_Buffer_t RX_Buffer;
//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of creating variables from enum ---------------------------//
LED_Display_Mode_t LED_Display_Mode = LED_NO_MODE;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Pointers ---------------------------------//


//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Counters ---------------------------------//

static uint16 g_TASK_LED_Counter=0;

static uint16 g_freeze_RX_Data_time_Counter = 0;
static uint8 g_Number_of_Printing_Counter=PRINT_ONCE;
//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Flags ------------------------------------//

static uint8 g_Send_Flag=NOT_SENDING;
static uint8 g_LCD_Request_Flag=NO_REQUEST;
static uint8 g_LCD_Available_Flag=AVAILABLE;
static uint8 g_LCD_Clear_First_Row_Flag = 0;
static uint8 g_LCD_Clear_Second_Row_Flag = 0;	

static uint8 g_UART_Send_Complete_Flag=NOTHING_TO_SEND;

static uint8 g_Task_init_Flag = NOT_COMPLETED;

static uint8 g_Receive_Flag = IDLE;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global variables --------------------------------//
static uint8 g_Clear_Str [] = "               " ; //The String which will be printed to clear the line
static uint8 g_initial_Str [] = ">>" ; //The String which will be printed to clear the line
static uint8 * g_LCD_String_ptr;
static uint8 g_LCD_Print_Data;
static uint8 g_LCD_column;
static uint8 g_LCD_row;


static sint8 g_Current_TX_Data_To_be_Displayed=-1;
static sint8 g_Current_Data_To_be_Sent=-1;

static sint8 g_Current_RX_Data_To_be_Displayed=-1;


static uint8 g_Receive_State = IDLE;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Handler Task --------------------------------//


TaskHandle_t LCD_Handle;        //The Handle we use to control the LCD Task (Suspend/Resume)
TaskHandle_t Switch_Handle;	    //The Handle we use to control the Keypad Task
TaskHandle_t Keypad_Handle;	    //The Handle we use to control the Keypad Task
TaskHandle_t Display_Handle;	//The Handle we use to control the Display Task
TaskHandle_t UART_Send_Handle;	//The Handle we use to control the UART_Send Task
TaskHandle_t LED_Handle;		//The Handle we use to control the LED Task

//--------------------------------------------------------------------------------------------------//



/****************************************************************************************************************/


//----------- Section of Private Prototypes of Functions   -------------//




//----------- Section of implementation of Function (Function Definition)  -------------//


void TASK_Init (void * POINTERVOID )
{
	DDRB = 0XFF;
	PORTB |=(1<<PB1);
	DIO_init();
	Switch_Init();
	Keypad_Init();
	UART_init();
	
	UART_Set_Callback_RX(UART_Recieve_Data); 

	Function_Process_t LCD_Process = NOT_FINISHED;
	TX_Buffer.Index=-1;
	RX_Buffer.Index=-1;
	g_Task_init_Flag = NOT_COMPLETED ;
	/*
	//Suspend all the Tasks until the initialization is completed
	vTaskSuspend( LCD_Handle );
	vTaskSuspend( Switch_Handle);
	vTaskSuspend( Keypad_Handle );
	vTaskSuspend( Display_Handle );
	vTaskSuspend( UART_Send_Handle );
	vTaskSuspend( LED_Handle );
	*/
	while(1)
	{
		LCD_Process = LCD_4Bits_Initialization_OS() ;
	
		if( LCD_Process == FINISHED )
		{
			g_Task_init_Flag = COMPLETED ;
			/*
			vTaskResume( LCD_Handle );
			vTaskResume( Switch_Handle);
			vTaskResume( Keypad_Handle );
			vTaskResume( Display_Handle );
			vTaskResume( UART_Send_Handle );
			vTaskResume( LED_Handle );
			*/
			vTaskSuspend(NULL);
			
		}
		vTaskDelay(1);
	}

}

void Task_LCD (void * POINTERVOID)
{
	uint8 Initial_Printing_of_System_Flag = NOT_READY;
	
  while(1)
  {
	  if(g_Task_init_Flag == COMPLETED)
	  {
			if(Initial_Printing_of_System_Flag != READY )
			{
				LCD_4Bits_Print_String(1,1,g_initial_Str);
				LCD_4Bits_Print_String(2,1,g_initial_Str);

				Initial_Printing_of_System_Flag = READY;
			}
			
			if (Initial_Printing_of_System_Flag == READY)
			{
				if(g_LCD_Request_Flag == NEW_REQUEST) 	//Is There any Request to be served
				{
					if((g_Send_Flag == SENDING_DATA)) 
					{
						g_LCD_Available_Flag=BUSY;			//Not Available NOW (Busy NOW)
						//Wait for the LCD to Print
						LCD_4Bits_Print_String(g_LCD_row,g_LCD_column,g_LCD_String_ptr);
						g_LCD_Available_Flag=AVAILABLE;		// Available
						g_LCD_Request_Flag=REQUEST_DONE;	//The Request has been Served and now it's free from any Request
						
					}
					
					else if ((g_Send_Flag == NOT_SENDING) || (g_Receive_Flag == RECEIVING))
					{		
						g_LCD_Available_Flag=BUSY;			//Not Available NOW (Busy NOW)
						//Wait for the LCD to Print
						LCD_4Bits_Print_Character(g_LCD_row,g_LCD_column,g_LCD_Print_Data);
						g_LCD_Available_Flag=AVAILABLE;		// Available
						g_LCD_Request_Flag=REQUEST_DONE;	//The Request has been Served and now it's free from any Request
					}
					/*
					if (g_Receive_Flag == RECEIVING)
					{
						g_LCD_Available_Flag=BUSY;			//Not Available NOW (Busy NOW)
						//Wait for the LCD to Print
						LCD_4Bits_Print_Character(g_LCD_row,g_LCD_column,g_LCD_Print_Data);
						//LCD_4Bits_Write_Data(g_LCD_Print_Data);
						g_LCD_Available_Flag=AVAILABLE;		// Available
						g_LCD_Request_Flag=REQUEST_DONE;	//The Request has been Served and now it's free from any Request
					}
						*/
					 if ((g_Receive_Flag == NOT_RECEIVING) && (g_Number_of_Printing_Counter == PRINT_ONCE))
					{
						g_LCD_Available_Flag=BUSY;			//Not Available NOW (Busy NOW)
						//Wait for the LCD to Print
						LCD_4Bits_Print_String(g_LCD_row,g_LCD_column,g_LCD_String_ptr);
						g_LCD_Available_Flag=AVAILABLE;		// Available
						g_LCD_Request_Flag=REQUEST_DONE;	//The Request has been Served and now it's free from any Request
						g_Number_of_Printing_Counter = NO_PRINTING;
					}
				}

				else if (g_LCD_Request_Flag == NO_REQUEST)
				{
					//Do Nothing
				}

			}
				
			}
			
		vTaskDelay(5);		    
	 }
}

void Task_Keypad (void * POINTERVOID)
{
  uint8 Keypad_Input;
	
  while(1)
  {
	if(g_Task_init_Flag == COMPLETED)
	{
		if(g_Send_Flag == NOT_SENDING)
		{
			keypad_Update();
			if(Keypad_Get_Data(&Keypad_Input) == 1)
			{
				TX_Buffer.Index++;
				TX_Buffer.Data[TX_Buffer.Index]=Keypad_Input;
			}
			
			else if(Keypad_Get_Data(&Keypad_Input) == 0)
			{
				// Do Nothing
			}
		}
	}
	
		vTaskDelay(25);
  }
}

void Task_Display (void * POINTERVOID)
{
	while(1)
	{
		if(g_Task_init_Flag == COMPLETED)
		{
			if(g_Send_Flag == SENDING_DATA)
			{
				//Write Once the Clear String to clear the row
				if(g_LCD_Clear_First_Row_Flag==0)
				{
					if(g_LCD_Available_Flag == AVAILABLE)		// Is LCD is Available Now
					{
						g_LCD_String_ptr = g_Clear_Str;
						g_LCD_row=1;
						g_LCD_column=3;
						g_LCD_Request_Flag=NEW_REQUEST;			// Add Request to the LCD to Print the Data
						g_LCD_Clear_First_Row_Flag=1;
					}
				}				
			}
			
			else if (g_Send_Flag == NOT_SENDING)
			{
				if ( g_Current_TX_Data_To_be_Displayed < TX_Buffer.Index )
				{
					if(g_LCD_Available_Flag == AVAILABLE)		// Is LCD is Available Now
					{
						g_Current_TX_Data_To_be_Displayed++;
						g_LCD_Print_Data = TX_Buffer.Data[g_Current_TX_Data_To_be_Displayed];
						g_LCD_row=1;
						g_LCD_column=(g_Current_TX_Data_To_be_Displayed+3);
						g_LCD_Request_Flag=NEW_REQUEST;			// Add Request to the LCD to Print the Data	
					}
				}
				
			}
			
			if(g_Receive_Flag == RECEIVING)
			{
				if ( g_Current_RX_Data_To_be_Displayed < RX_Buffer.Index )
				{
					if(g_LCD_Available_Flag == AVAILABLE)		// Is LCD is Available Now
					{
						g_Current_RX_Data_To_be_Displayed++;
						g_LCD_Print_Data = RX_Buffer.Data[g_Current_RX_Data_To_be_Displayed];
						g_LCD_row=2;
						g_LCD_column=(g_Current_RX_Data_To_be_Displayed+3);
						g_LCD_Request_Flag=NEW_REQUEST;			// Add Request to the LCD to Print the Data	
					}
				}
			}
			
			else if(g_Receive_Flag == NOT_RECEIVING)
			{
				if(g_LCD_Clear_Second_Row_Flag==0)
				{
					if(g_LCD_Available_Flag == AVAILABLE)		// Is LCD is Available Now
					{
						g_LCD_String_ptr = g_Clear_Str;
						g_LCD_row=2;
						g_LCD_column=3;
						g_LCD_Request_Flag=NEW_REQUEST;			// Add Request to the LCD to Print the Data
						g_LCD_Clear_Second_Row_Flag=1;			
					}
				}
			}
		}
		
		vTaskDelay(50);
	}
}


void Task_SWITCH (void * POINTERVOID)
{

	while(1)
	{
		if(g_Task_init_Flag == COMPLETED)
		{
			SWITCH_Update();
			if(SWITCH_GetState(SWITCH_1)==SWITCH_PRE_PRESSED)
			{
				//Start to Send the Data that has been written in the buffer through UART
				//Block the Keypad Driver
				//Enable the LED
				g_Send_Flag=SENDING_DATA;
			}
		}
		
		vTaskDelay(25);
	}
	
}


void Task_UART_Transceiver (void * POINTERVOID)
{
	uint8 RX_data;
	
	while(1)
	{
		
		if(g_Task_init_Flag == COMPLETED)
		{
			if(g_Send_Flag == SENDING_DATA)
			{
				if(g_Current_Data_To_be_Sent == TX_Buffer.Index)
				{
					g_UART_Send_Complete_Flag = SENDING_COMPLETE;
				}
				
				else if(g_Current_Data_To_be_Sent < TX_Buffer.Index)
				{
					g_Current_Data_To_be_Sent++;
					// Send the Data Bytes from the buffer till reached whether to its maximum size or to the end of buffer data
					UART_send(TX_Buffer.Data[g_Current_Data_To_be_Sent]);
				}
			}
			
			switch (g_Receive_State)
			{
				case IDLE:
				{
					Reset_RX_System();
					g_Receive_State = NOT_RECEIVING;
					break;
				}
				case NOT_RECEIVING:
				{
					g_Receive_Flag = NOT_RECEIVING ;
					//Do nothing 
					break;
				}
					
				case RECEIVING:
				{
					if( RX_Buffer.Index < BUFFER_SIZE)
					{
						g_Receive_Flag = RECEIVING ;
						// Receive the Data Bytes from the buffer till reached whether to its maximum size or to the end of buffer data
						UART_recieve (&RX_data);
						RX_Buffer.Index++;
						RX_Buffer.Data[RX_Buffer.Index]=RX_data;
						g_Receive_State = HOLDING_RX_DATA;
					}	
					
					break;
				}
				
				case HOLDING_RX_DATA:
				{
					//HOLDING_RECEIVED_DATA for a while;			
					if(g_freeze_RX_Data_time_Counter == 2000)
					{
						g_Receive_State = IDLE;
					}
					g_freeze_RX_Data_time_Counter++;					
					break;
				}
				
				default:
				{
					//Shouldn't be here
					break;
				}
			}
		}
		
		vTaskDelay(1);
	}
}


void Task_LED (void * POINTERVOID)
{
	while(1)
	{
		if(g_Task_init_Flag == COMPLETED)
		{
			switch(LED_Display_Mode)
			{
				case LED_NO_MODE:
				{
					if(g_Send_Flag == SENDING_DATA)
					{
						LED_Display_Mode = LED_ON_MODE;
					}
					break;
				}
				
				case LED_ON_MODE:
				{
					if(g_UART_Send_Complete_Flag==SENDING_COMPLETE)
					{
						LED_ON(LED_PORT_,LED_2_PIN);
					}
					
					LED_ON(LED_PORT_,LED_1_PIN);
					//make the LED is On for 200 ms
					if(g_TASK_LED_Counter == 200)
					{
						LED_Display_Mode=LED_OFF_MODE;
					}
					g_TASK_LED_Counter += 50;
					
					break;
				}
				
				case LED_OFF_MODE:
				{
					LED_OFF(LED_PORT_,LED_2_PIN);
					LED_OFF(LED_PORT_,LED_1_PIN);
					//LED_Display_Mode= LED_NO_MODE;
					Reset_TX_System ();
					
					break;
				}
				
				default:
				{
					break;
				}
			}
		}
		
		vTaskDelay(50);				
	}

}





void UART_Recieve_Data (void)
{
	g_Receive_State=RECEIVING;
	g_freeze_RX_Data_time_Counter = 0;
}


void Reset_RX_System(void)
{
	RX_Buffer.Index=-1;
	g_Current_RX_Data_To_be_Displayed=-1;
	g_LCD_Clear_Second_Row_Flag = 0;
	g_Number_of_Printing_Counter = PRINT_ONCE;
	
}

void Reset_TX_System (void)
{
	TX_Buffer.Index=-1;
	g_Current_TX_Data_To_be_Displayed=-1;
	g_LCD_Clear_First_Row_Flag=0;
	g_Send_Flag = NOT_SENDING;
	
	g_Current_Data_To_be_Sent=-1;
	g_UART_Send_Complete_Flag=NOTHING_TO_SEND;
		
	LED_Display_Mode= LED_NO_MODE;
	
	g_TASK_LED_Counter=0;

}