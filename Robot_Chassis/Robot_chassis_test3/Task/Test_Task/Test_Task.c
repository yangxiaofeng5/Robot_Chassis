#include "Test_Task.h"

#include "bsp_imu.h"
#include "bsp_can.h"
#include "pid.h"
#include "user_lib.h"

#define SYSTEM_INFORMATION_CALLBACK 0 //任务运行时间信息统计显示，通过0.1来使用
#define MOTOR_TEST 0									//电机测试代码
#define MPU6500_TEST 1								//陀螺仪测试代码
#define DJI_REMOTE_TEST 0									//遥控器测试代码
#define PI 3.1415926f
//#define Pitch_RC_SENABE -0.000006f    //遥控器系数
//#define Motor_Ecd_to_Rad 0.000766990394f //      2*  PI  /8192

//任务运行返回参数所需的变量
uint8_t pcWriteBuffer[200];
uint8_t ulHighFrequencyTimerTicks;

extern moto_info_t motor_info[7];
//moto_info_t motor_angle_info[4];
//moto_info_t motor_speed_info[4];

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
		pid_init(&motor_speed_pid[i], 1220.0f, 00.0f, 0, 5000.0f, 30000.0f); 
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
		target_speed1 = remote_control.ch4*0.0088228f;//150/660
		target_speed2 = remote_control.ch3*0.0088228f;
		if(remote_control.switch_left == 2)
		{
						set_motor_voltage(0, 													//设置电机速度
													0, 
													0, 
													0, 
													0);
		wave_form_data[4] = motor_info[4].rotor_angle;
		wave_form_data[3] = target_speed2*1000.0f;
		wave_form_data[2] = -imu.rol;
		wave_form_data[1] = -imu.pit*1000;

			shanwai_send_wave_form();
		}
		else
		{
//				if( remote_control.ch4 != 0)
				motor_info[4].set_voltage = (int16_t)pid_calc(&motor_speed_pid[4], 
																						target_speed1,
																						imu.wx);
			
//				motor_info[1].set_voltage = (int16_t)pid_calc(&motor_speed_pid[1], 
//																						target_speed2,
//																						-imu.wz);
			printf("In Chassis_Task's loop\r\n");

			set_motor_voltage(0, 													//设置电机速度
													-motor_info[4].set_voltage, 
													/*motor_info[1].set_voltage*/0, 
													0, 
													0);


			wave_form_data[0] = motor_info[4].rotor_angle*100;
			wave_form_data[1] = target_speed1*1000.0f;
			wave_form_data[2] = -motor_info[4].set_voltage;
			wave_form_data[3] = -imu.rol*1000;
			shanwai_send_wave_form();
		}
    osDelay(10);
		#endif
		
		#if MPU6500_TEST
		MPU6500_GET_DATA();
		
		wave_form_data[0] = imu.pit;
    wave_form_data[1] = imu.yaw;		//左右
		wave_form_data[2] = imu.rol;		//前后
		wave_form_data[3] = imu.temp;
		
		wave_form_data[4] = imu.wx;
    wave_form_data[5] = imu.wy;
		wave_form_data[6] = imu.wz;
		
		shanwai_send_wave_form();
    osDelay(10);
		
		#endif
		#if DJI_REMOTE_TEST 
		wave_form_data[0] = remote_control.ch1;
    wave_form_data[1] = remote_control.ch2;		//左右
		wave_form_data[2] = remote_control.ch3;		//前后
		wave_form_data[3] = remote_control.ch4;	
		wave_form_data[4] = remote_control.switch_left;
    wave_form_data[5] = remote_control.switch_right;
//		wave_form_data[6] = imu.pit;
		
		shanwai_send_wave_form();
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
