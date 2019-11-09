/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "Initial_Task.h"

#include "Chassis_Task.h"

#include "dji_remote.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t pcWriteBuffer[200];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId vStart_TaskHandle;
osThreadId vInitial_TaskHandle;
osThreadId vChassis_TaskHandle;
osThreadId vGimbal_TaskHandle;
osThreadId vProtect_TaskHandle;
osThreadId vTask_10msHandle;
osThreadId vOut_ControlHandle;
osThreadId vRevolver_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartTask(void const * argument);
void Initial_Task(void const * argument);
void Chassis_Task(void const * argument);
void Gimbal_Task(void const * argument);
void Protect_Task(void const * argument);
void Task_10ms(void const * argument);
void Out_Control(void const * argument);
void Revolver_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of vStart_Task */
  osThreadDef(vStart_Task, StartTask, osPriorityNormal, 0, 128);
  vStart_TaskHandle = osThreadCreate(osThread(vStart_Task), NULL);

//  /* definition and creation of vInitial_Task */
//  osThreadDef(vInitial_Task, Initial_Task, osPriorityNormal, 0, 128);
//  vInitial_TaskHandle = osThreadCreate(osThread(vInitial_Task), NULL);

//  /* definition and creation of vChassis_Task */
//  osThreadDef(vChassis_Task, Chassis_Task, osPriorityNormal, 0, 128);
//  vChassis_TaskHandle = osThreadCreate(osThread(vChassis_Task), NULL);

//  /* definition and creation of vGimbal_Task */
//  osThreadDef(vGimbal_Task, Gimbal_Task, osPriorityAboveNormal, 0, 128);
//  vGimbal_TaskHandle = osThreadCreate(osThread(vGimbal_Task), NULL);

//  /* definition and creation of vProtect_Task */
//  osThreadDef(vProtect_Task, Protect_Task, osPriorityLow, 0, 128);
//  vProtect_TaskHandle = osThreadCreate(osThread(vProtect_Task), NULL);

//  /* definition and creation of vTask_10ms */
//  osThreadDef(vTask_10ms, Task_10ms, osPriorityHigh, 0, 128);
//  vTask_10msHandle = osThreadCreate(osThread(vTask_10ms), NULL);

//  /* definition and creation of vOut_Control */
//  osThreadDef(vOut_Control, Out_Control, osPriorityRealtime, 0, 128);
//  vOut_ControlHandle = osThreadCreate(osThread(vOut_Control), NULL);

//  /* definition and creation of vRevolver_Task */
//  osThreadDef(vRevolver_Task, Revolver_Task, osPriorityHigh, 0, 128);
//  vRevolver_TaskHandle = osThreadCreate(osThread(vRevolver_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	printf("everything is ready!");
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartTask */
/**
  * @brief  Function implementing the vStart_Task thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartTask */
void StartTask(void const * argument)
{
    
    
    
    
    
    
    
    
    

  /* USER CODE BEGIN StartTask */
		portENTER_CRITICAL();
	/************初始化任务**************/
  osThreadDef(vInitial_Task, Initial_Task, osPriorityNormal, 0, 128);
  vInitial_TaskHandle = osThreadCreate(osThread(vInitial_Task), NULL);

	/************底盘任务****************/
  osThreadDef(vChassis_Task, Chassis_Task, osPriorityNormal, 0, 128);
  vChassis_TaskHandle = osThreadCreate(osThread(vChassis_Task), NULL);
	
	/************云台任务****************/
  osThreadDef(vGimbal_Task, Gimbal_Task, osPriorityAboveNormal, 0, 128);
  vGimbal_TaskHandle = osThreadCreate(osThread(vGimbal_Task), NULL);
	
	/************拨盘任务****************/
	osThreadDef(vRevolver_Task, Revolver_Task, osPriorityHigh, 0, 128);
  vRevolver_TaskHandle = osThreadCreate(osThread(vRevolver_Task), NULL);

	/************10ms任务**************/
  osThreadDef(vTask_10ms, Task_10ms, osPriorityHigh, 0, 128);
  vTask_10msHandle = osThreadCreate(osThread(vTask_10ms), NULL);
	
	/************任务失控任务**************/
	osThreadDef(vOut_Control, Out_Control, osPriorityHigh, 0, 128);
  vOut_ControlHandle = osThreadCreate(osThread(vOut_Control), NULL);

	/************失控任务控制**************/
  osThreadDef(vProtect_Task, Protect_Task, osPriorityLow, 0, 128);
  vProtect_TaskHandle = osThreadCreate(osThread(vProtect_Task), NULL);

	vTaskDelete(vStart_TaskHandle); //删除开始任务
  portEXIT_CRITICAL();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask */
}

/* USER CODE BEGIN Header_Initial_Task */
/**
* @brief Function implementing the vInitial_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Initial_Task */
//void Initial_Task(void const * argument)
//{
//  /* USER CODE BEGIN Initial_Task */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END Initial_Task */
//}

///* USER CODE BEGIN Header_Chassis_Task */
///**
//* @brief Function implementing the vChassis_Task thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Chassis_Task */
//void Chassis_Task(void const * argument)
//{
//  /* USER CODE BEGIN Chassis_Task */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END Chassis_Task */
//}

///* USER CODE BEGIN Header_Gimbal_Task */
///**
//* @brief Function implementing the vGimbal_Task thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Gimbal_Task */
//void Gimbal_Task(void const * argument)
//{
//  /* USER CODE BEGIN Gimbal_Task */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END Gimbal_Task */
//}

/* USER CODE BEGIN Header_Protect_Task */
/**
* @brief Function implementing the vProtect_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Protect_Task */
void Protect_Task(void const * argument)
{
  /* USER CODE BEGIN Protect_Task */
	static portTickType System_Current_Time;
  /* Infinite loop */
  for(;;)
  {
		System_Current_Time = xTaskGetTickCount();	//获取当前系统时间	
//		printf("%d %d \n",System_Current_Time,Latest_Remote_Control_Pack_Time);//系统获取时间调试
		if(System_Current_Time - Latest_Remote_Control_Pack_Time >5)//接收机离线50ms
		{
			vTaskSuspend(vChassis_TaskHandle);		//将任务挂起
			vTaskSuspend(vGimbal_TaskHandle);
			vTaskSuspend(vRevolver_TaskHandle);
			vTaskSuspend(vTask_10msHandle);
			
			vTaskResume(vOut_ControlHandle);//解挂失控保护控制任务
		}
		else 
		{
			vTaskResume(vChassis_TaskHandle);		//恢复任务
			vTaskResume(vGimbal_TaskHandle);
			vTaskResume(vRevolver_TaskHandle);
			vTaskResume(vTask_10msHandle);
			
			vTaskSuspend(vOut_ControlHandle);//挂起失控保护控制任务
		}
    osDelayUntil (&System_Current_Time,50);     //绝对延时50ms
  }
  /* USER CODE END Protect_Task */
}

/* USER CODE BEGIN Header_Task_10ms */
/**
* @brief Function implementing the vTask_10ms thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_10ms */
void Task_10ms(void const * argument)
{
  /* USER CODE BEGIN Task_10ms */
  /* Infinite loop */
  for(;;)
  {
//		printf("=================================================\r\n");
//		printf("\r\ntask_name  \tstate\t prior\tstack\t Id\r\n");
//		vTaskList((char *)&pcWriteBuffer);
//		printf("%s\r\n", pcWriteBuffer);

//		printf("\r\ntask_name     time_count(10us) usage_pec\r\n");
//		vTaskGetRunTimeStats((char *)&pcWriteBuffer);
//		printf("%s\r\n", pcWriteBuffer);
    osDelay(5);
  }
  /* USER CODE END Task_10ms */
}

/* USER CODE BEGIN Header_Out_Control */
/**
* @brief Function implementing the vOut_Control thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Out_Control */
void Out_Control(void const * argument)
{
  /* USER CODE BEGIN Out_Control */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Out_Control */
}

/* USER CODE BEGIN Header_Revolver_Task */
/**
* @brief Function implementing the vRevolver_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Revolver_Task */
//void Revolver_Task(void const * argument)
//{
//  /* USER CODE BEGIN Revolver_Task */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END Revolver_Task */
//}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
