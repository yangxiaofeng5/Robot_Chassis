#include "Test_Task.h"
#include "bsp_imu.h"

#include "bsp_can.h"
#include "pid.h"

#define SYSTEM_INFORMATION_CALLBACK 0 //任务运行时间信息统计显示，通过0.1来使用
#define MOTOR_TEST 0									//电机测试代码
//任务运行返回参数所需的变量
uint8_t pcWriteBuffer[200];
uint8_t ulHighFrequencyTimerTicks;

extern moto_info_t motor_info[MOTOR_MAX_NUM];
pid_struct_t motor_pid[7];
uint16_t target_speed = 6000;

void Test_Task(void const * argument)
{
	//	printf("In Test_Task!\n");
	for (uint8_t i = 0; i < 7; i++)
  {
    pid_init(&motor_pid[i], 2000, 0, 0, 5000, 30000); 
  }
	
	MPU6500_INITIAL();
	

  for(;;)
  {
		#if SYSTEM_INFORMATION_CALLBACK
		printf("=================================================\r\n");
		printf("\r\ntask_name    \tstate\t   prior\t  stack\t   Id\r\n");
		vTaskList((char *)&pcWriteBuffer);
		printf("%s\r\n", pcWriteBuffer);

//		printf("\r\ntask_name     time_count(10us) usage_pec\r\n");
		vTaskGetRunTimeStats((char *)&pcWriteBuffer);
		printf("%s\r\n", pcWriteBuffer);
    osDelay(100);
		#endif
		
		//电机检验代码
		#if MOTOR_TEST
		MPU6500_GET_DATA();
//		for (uint8_t i = 0; i < 7; i++)
//			{
//				motor_info[i].set_voltage  = pid_calc(&motor_pid[i], target_speed , motor_info[i].rotor_angle);
//			}
	
		motor_info[4].set_voltage = pid_calc(&motor_pid[4], target_speed , motor_info[4].rotor_angle);//imu.pit
//		motor_info[4].set_voltage=PID_Calc(&motor_pid[4], target_speed , motor_info[4].rotor_angle, imu.pit);

		printf("In Chassis_Task's loop\r\n");

		set_motor_voltage(0, 													//设置电机速度
												motor_info[0].rotor_angle, 
												motor_info[1].rotor_angle, 
												motor_info[2].rotor_angle, 
												motor_info[4].set_voltage);


		wave_form_data[0] = motor_info[4].rotor_speed;
    wave_form_data[1] = motor_info[4].set_voltage;
		wave_form_data[2] = motor_info[4].rotor_angle;
		wave_form_data[3] = imu.pit*1000;
//		
//		wave_form_data[4] = motor_info[4].rotor_angle;
//    wave_form_data[5] = motor_info[5].rotor_speed;
//		wave_form_data[6] = motor_info[6].rotor_speed;
		
		shanwai_send_wave_form();
    osDelay(10);
		#endif
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
