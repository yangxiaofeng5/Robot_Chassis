#include "Test_Task.h"

#include "bsp_imu.h"

#include "bsp_can.h"
#include "pid.h"

#define SYSTEM_INFORMATION_CALLBACK 0 //任务运行时间信息统计显示，通过0.1来使用
#define MOTOR_TEST 1									//电机测试代码
#define MPU6500_TSES 0								//陀螺仪测试代码

#define Pitch_RC_SENABE -0.000006f    //遥控器系数
#define Motor_Ecd_to_Rad 0.000766990394f //      2*  PI  /8192

//任务运行返回参数所需的变量
uint8_t pcWriteBuffer[200];
uint8_t ulHighFrequencyTimerTicks;

moto_info_t motor_angle_info[4];
moto_info_t motor_speed_info[4];

pid_struct_t motor_angle_pid[4];
pid_struct_t motor_speed_pid[4];
float  target_speed1 = 0;
float  target_speed2 = 0;



void Test_Task(void const * argument)
{
	//	printf("In Test_Task!\n");
	for (uint8_t i = 0; i < 4; i++)
  {
    pid_init(&motor_angle_pid[i], 15.0f, 0, 0, 0.0f, 10.0f); 
		pid_init(&motor_speed_pid[i], 1220.0f, 30.0f, 0, 5000.0f, 30000.0f); 
  }
	osDelay(300);
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
		target_speed1 = remote_control.ch4*0.0088228f;
		target_speed2 = remote_control.ch3*0.0088228f;
		if(remote_control .switch_left == 2)
		{
						set_motor_voltage(0, 													//设置电机速度
													0, 
													0, 
													0, 
													0);
		wave_form_data[4] = target_speed1*1000.0f;
		wave_form_data[3] = target_speed2*1000.0f;
		wave_form_data[2] = -imu.wx*1000;
		wave_form_data[1] = -imu.wz*1000;
//    wave_form_data[5] = motor_info[5].rotor_speed;
//		wave_form_data[6] = motor_info[6].rotor_speed;
			shanwai_send_wave_form();
		}
		else
		{
		
				motor_angle_info[0].set_voltage = (int16_t)pid_calc(&motor_speed_pid[0], 
																						target_speed1,
																						imu.wx);
//				motor_speed_info[0].current_set = (int16_t)pid_calc(&motor_angle_pid[0], 
//																						motor_angle_info[0].set_voltage, 
//																						motor_angle_info[0].rotor_angle);
//			
				motor_angle_info[1].set_voltage = (int16_t)pid_calc(&motor_speed_pid[1], 
																						target_speed2,
																						-imu.wz);
			printf("In Chassis_Task's loop\r\n");

			set_motor_voltage(0, 													//设置电机速度
													-motor_angle_info[0].set_voltage, 
													motor_angle_info[1].set_voltage, 
													0, 
													0);


			wave_form_data[0] = motor_angle_info[0].rotor_angle*100;
			wave_form_data[1] = motor_angle_info[0].torque_current;
			wave_form_data[2] = -motor_angle_info[0].rotor_speed;
			wave_form_data[3] = -imu.wx*1000;
			shanwai_send_wave_form();
		}
    osDelay(10);
		#endif
		
		#if MPU6500_TSES
		MPU6500_GET_DATA();
		
		wave_form_data[0] = imu.pit*1000;
    wave_form_data[1] = imu.yaw*1000;		//左右
		wave_form_data[2] = imu.rol*1000;		//前后
		wave_form_data[3] = imu.temp;
//		
//		wave_form_data[4] = imu.pit;
//    wave_form_data[5] = imu.pit;
//		wave_form_data[6] = imu.pit;
		
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
