#include "Test_Task.h"

uint8_t pcWriteBuffer[200];
uint8_t ulHighFrequencyTimerTicks;


void Test_Task(void const * argument)
{
	//	printf("In Test_Task!\n");

  for(;;)
  {
		printf("=================================================\r\n");
		printf("\r\ntask_name    \tstate\t   prior\t  stack\t   Id\r\n");
		vTaskList((char *)&pcWriteBuffer);
		printf("%s\r\n", pcWriteBuffer);

//		printf("\r\ntask_name     time_count(10us) usage_pec\r\n");
		vTaskGetRunTimeStats((char *)&pcWriteBuffer);
		printf("%s\r\n", pcWriteBuffer);
    osDelay(100);
  }

}
void configureTimerForRunTimeStats(void)
{
	ulHighFrequencyTimerTicks = 0ul;
}

unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks;
}
