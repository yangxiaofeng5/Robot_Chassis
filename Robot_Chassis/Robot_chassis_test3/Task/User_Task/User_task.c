#include "User_task.h"
#include "oled.h"
#include "dji_remote.h"
#include "robomaster_vcan.h"


void User_Task(void const * argument)
{
  /* USER CODE BEGIN User_Task */
	
	printf ("In User_Task!\r\n");
  /* Infinite loop */
  for(;;)
  {
		printf ("ch1=%d ",remote_control.ch1);
		printf ("ch2=%d ",remote_control.ch2);
		printf ("ch3=%d ",remote_control.ch3);
		printf ("ch4=%d\r\n",remote_control.ch4);
//		oled_showchar(1, 1, 'a');
		
		wave_form_data[0] = (short)remote_control.ch1;
    wave_form_data[1] = (short)remote_control.ch2;
		wave_form_data[2] = (short)remote_control.ch3;
		wave_form_data[3] = (short)remote_control.ch4;
		shanwai_send_wave_form();
    osDelay(10);
  }
  /* USER CODE END User_Task */
}


