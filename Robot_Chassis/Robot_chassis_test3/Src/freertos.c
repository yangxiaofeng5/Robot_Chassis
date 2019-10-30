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
#include "User_task.h"
#include "Chassis_Task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
osThreadId vUser_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartTask(void const * argument);
void Initial_Task(void const * argument);
void Chassis_Task(void const * argument);
void User_Task(void const * argument);

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
//  osThreadDef(vChassis_Task, Chassis_Task, osPriorityAboveNormal, 0, 512);
//  vChassis_TaskHandle = osThreadCreate(osThread(vChassis_Task), NULL);

//  /* definition and creation of vUser_Task */
//  osThreadDef(vUser_Task, User_Task, osPriorityAboveNormal, 0, 256);
//  vUser_TaskHandle = osThreadCreate(osThread(vUser_Task), NULL);

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
	osThreadDef(vInitial_Task, Initial_Task, osPriorityHigh, 0, 128);
  vInitial_TaskHandle = osThreadCreate(osThread(vInitial_Task), NULL);
	
	/************底盘任务****************/
	osThreadDef(vChassis_Task, Chassis_Task, osPriorityAboveNormal, 0, 128);
  vChassis_TaskHandle = osThreadCreate(osThread(vChassis_Task), NULL);

	/************自定义任务**************/
  /* definition and creation of vUser_Task */
//  osThreadDef(vUser_Task, User_Task, osPriorityRealtime, 0, 128);
//  vUser_TaskHandle = osThreadCreate(osThread(vUser_Task), NULL);

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

/* USER CODE BEGIN Header_Chassis_Task */
///**
//* @brief Function implementing the vChassis_Task thread.
//* @param argument: Not used
//* @retval None
//*/
/* USER CODE END Header_Chassis_Task */
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

/* USER CODE BEGIN Header_User_Task */
///**
//* @brief Function implementing the vUser_Task thread.
//* @param argument: Not used
//* @retval None
//*/
/* USER CODE END Header_User_Task */
//void User_Task(void const * argument)
//{
//  /* USER CODE BEGIN User_Task */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END User_Task */
//}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
