#include "Test_Task.h"

#include "bsp_can.h"
#include "pid.h"

#define SYSTEM_INFORMATION_CALLBACK 0 //��������ʱ����Ϣͳ����ʾ��ͨ��0.1��ʹ��
#define MOTOR_TEST 0									//������Դ���
//�������з��ز�������ı���
uint8_t pcWriteBuffer[200];
uint8_t ulHighFrequencyTimerTicks;

extern moto_info_t motor_info[MOTOR_MAX_NUM];
pid_struct_t motor_pid[7];
uint16_t target_speed = 2000;

void Test_Task(void const * argument)
{
	//	printf("In Test_Task!\n");
	for (uint8_t i = 0; i < 7; i++)
  {
    pid_init(&motor_pid[i], 2, 0.5, 0, 300, 30000); 
  }

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
		
		//����������
		#if MOTOR_TEST
		for (uint8_t i = 0; i < 7; i++)
			{
				motor_info[i].set_voltage  = pid_calc(&motor_pid[i], target_speed , motor_info[i].rotor_angle);
			}
	
		printf("In Chassis_Task's loop\r\n");

		set_motor_voltage(0, 													//���õ���ٶ�
												motor_info[0].rotor_angle, 
												motor_info[1].rotor_angle, 
												motor_info[2].rotor_angle, 
												motor_info[4].rotor_angle);


		wave_form_data[0] = motor_info[0].rotor_speed;
    wave_form_data[1] = motor_info[1].rotor_speed;
		wave_form_data[2] = motor_info[2].rotor_angle;
		wave_form_data[3] = motor_info[3].rotor_speed;
		
		wave_form_data[4] = motor_info[4].rotor_angle;
    wave_form_data[5] = motor_info[5].rotor_speed;
		wave_form_data[6] = motor_info[6].rotor_speed;
		
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
