
#include "Application.h"


int main(void) 
{

	xTaskCreate(TASK_Init,"TASK_Init",100,NULL,4,NULL);

	xTaskCreate(Task_SWITCH,"Task_SWITCH",100,NULL,1,&Switch_Handle);
	
	xTaskCreate(Task_LCD,"Task_LCD",100,NULL,2,&LCD_Handle);
	
	xTaskCreate(Task_Display,"Task_Display",100,NULL,1,NULL);
	
	xTaskCreate(Task_Keypad,"Task_Keypad",100,NULL,2,&Keypad_Handle);
	
	xTaskCreate(Task_UART_Transceiver,"Task_UART_Transceiver",100,NULL,3,&UART_Send_Handle);
	
	xTaskCreate(Task_LED,"Task_LED",100,NULL,1,&LED_Handle);

	/* Start Scheduler */
	vTaskStartScheduler();

	while(1)
	{

	}
}
