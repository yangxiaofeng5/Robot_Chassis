#ifndef __Gimbal_Task_H
#define __Gimbal_Task_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
/********freertos��Ҫ���õĿ��ļ�********/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
/********�����������ļ�********/
//#include "dji_remote.h"
//#include "oled.h"
//#include "oledfont.h"	
	 
	 
extern osThreadId vGimbal_TaskHandle;
	 
void Gimbal_Task(void const * argument);
	 
#ifdef __cplusplus
}
#endif
#endif /*__Gimbal_Task_H */
