#include "Initial_Task.h"
#include "Chassis_Task.h"
#include "oled.h"

void Initial_Task(void const * argument)
{
  /* USER CODE BEGIN Initial_Task */
	//unsigned portBASE_TYPE uxPriority;//��ȡ�������ȼ�ʹ�õ�
	//	uxPriority = uxTaskPriorityGet (NULL);
	
	printf("In Initial_Task\r\n");
  oled_init();
	oled_clear(Pen_Clear);
	
for (uint8_t i = 0; i < 7; i++)
  {
    pid_init(&motor_pid[i], 1.5, 0.1, 0, 5000, 16384); //init pid parameter, kp=40, ki=3, kd=0, output limit = 30000
  }
	oled_LOGO();
	osDelay (1000);
	oled_clear(Pen_Clear);
	oled_refresh_gram();
  /* Infinite loop */
  for(;;)
  {
		printf ("In Initial_Task's loop\r\n");
//		HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
//		vTaskPrioritySet(vInitial_TaskHandle,(uxPriority-3));//��ִֻ��һ�鲻�����ã����ù���vTaskSuspend
		vTaskSuspend(vInitial_TaskHandle);//ִ����������ó�ʼ����͹���
    osDelay(1);
  }
  /* USER CODE END Initial_Task */
}

