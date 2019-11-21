#include "Initial_Task.h"
#include "Chassis_Task.h"

#include "oled.h"
#include "bsp_imu.h"

#include "robomaster_vcan.h"

void Initial_Task(void const * argument)
{
  /* USER CODE BEGIN Initial_Task */
	printf("In Initial_Task\r\n");
  oled_init();
	oled_clear(Pen_Clear);
	oled_LOGO();
	osDelay (10);
	
	MPU6500_INITIAL();//�����ǳ�ʼ��
	
	oled_clear(Pen_Clear);
	oled_refresh_gram();
	
  /* Infinite loop */
  for(;;)
  {
		vTaskSuspend(vInitial_TaskHandle);//ִ����������ó�ʼ����͹���
    osDelay(1);
  }
  /* USER CODE END Initial_Task */
}

