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
	
	MPU6500_INITIAL();//陀螺仪初始化
	
	//底盘电机pid参数初始化
	for (uint8_t i = 0; i < 7; i++)
  {
    pid_init(&motor_pid[i], 1.5, 0.1, 0, 5000, 16384); 
  }
	oled_clear(Pen_Clear);
	oled_refresh_gram();
	
  /* Infinite loop */
  for(;;)
  {
		vTaskSuspend(vInitial_TaskHandle);//执行完基本设置初始化后就挂起
    osDelay(1);
  }
  /* USER CODE END Initial_Task */
}

