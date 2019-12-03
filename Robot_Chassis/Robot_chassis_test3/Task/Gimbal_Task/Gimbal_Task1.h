#ifndef __Gimbal_Task1_H
#define __Gimbal_Task1_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
/********freertos需要调用的库文件********/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
/********外设驱动库文件********/
#include "dji_remote.h"
//#include "oled.h"
//#include "oledfont.h"	
#include "pid.h"
	
//任务初始化 空闲一段时间
#define GIMBAL_TASK_INIT_TIME 201 
	 
//云台控制周期
#define GIMBAL_CONTROL_TIME 1

/***********************************************START***********************************************/
/*功能代码名称：参数转换系数*/
	 
//云台遥控控制系数转换 				2*pi/660弧度制
#define RC_TRAVEL_FACTOR 0.00751997f
	 
//云台电机编码器系数转换      	2*PI/8192弧度制 	 
#define Motor_Ecd_to_Rad 0.00076699f 

//软件取反系数转换
#define NEGATION_COEFFICIENT -1
/***********************************************END***************************************************/

/***********************************************START***********************************************/
/*功能代码名称：硬件*/

//云台电机反装设置 1为反装 0为正装
#define PITCH_TURN 1
#define YAW_TURN 1

//电机码盘值最大以及中值
#define Half_ecd_range 4096
#define ecd_range 8191

//遥控器输入死区，因为遥控器存在差异，摇杆在中间，其值不一定为零
#define RC_Dead_Band 10
/***********************************************END***************************************************/

/***********************************************START***********************************************/
/*功能代码名称：云台电机的最初机械角度设置*/
	 
#define YAW_ENCODE_OFFEST 			4766.0f			//YAW轴机械中值 
#define PITCH_ENCODE_OFFEST 		1120.0f			//pitch轴机械中值
#define YAW_MAX_ENCODE_ANGLE 		3828.0f			
#define YAW_MIN_ENCODE_ANGLE 		5720.0f
#define PITCH_MAX_ENCODE_ANGLE 	41.0f
#define PITCH_MIN_ENCODE_ANGLE 	2150.0f
/***********************************************END***************************************************/
	 
/***********************************************START***********************************************/
/*功能代码名称：电机PID宏定义*/
	 
//pitch 速度环 PID参数以及 PID最大输出，积分输出
#define PITCH_SPEED_PID_KP 595.0f//2000.0f
#define PITCH_SPEED_PID_KI 20.0f//20.0f
#define PITCH_SPEED_PID_KD 0.0f
#define PITCH_SPEED_PID_MAX_OUT 30000.f//30000.0f
#define PITCH_SPEED_PID_MAX_IOUT 5000.0f

//yaw 速度环 PID参数以及 PID最大输出，积分输出
#define YAW_SPEED_PID_KP 1000.0f
#define YAW_SPEED_PID_KI 0.0f//20.0f
#define YAW_SPEED_PID_KD 0.0f
#define YAW_SPEED_PID_MAX_OUT 30000.0f
#define YAW_SPEED_PID_MAX_IOUT 5000.0f

//pitch 角度环 角度由陀螺仪解算 PID参数以及 PID最大输出，积分输出
#define PITCH_GYRO_ABSOLUTE_PID_KP 15.0f
#define PITCH_GYRO_ABSOLUTE_PID_KI 0.0f
#define PITCH_GYRO_ABSOLUTE_PID_KD 0.0f

#define PITCH_GYRO_ABSOLUTE_PID_MAX_OUT 10.0f
#define PITCH_GYRO_ABSOLUTE_PID_MAX_IOUT 0.0f

//yaw 角度环 角度由陀螺仪解算 PID参数以及 PID最大输出，积分输出
#define YAW_GYRO_ABSOLUTE_PID_KP 10.0f
#define YAW_GYRO_ABSOLUTE_PID_KI 0.0f
#define YAW_GYRO_ABSOLUTE_PID_KD 0.3f
#define YAW_GYRO_ABSOLUTE_PID_MAX_OUT 10.0f
#define YAW_GYRO_ABSOLUTE_PID_MAX_IOUT 0.0f

//pitch 角度环 角度由编码器 PID参数以及 PID最大输出，积分输出
#define PITCH_ENCODE_RELATIVE_PID_KP 20.0f
#define PITCH_ENCODE_RELATIVE_PID_KI 0.00f
#define PITCH_ENCODE_RELATIVE_PID_KD 0.0f

#define PITCH_ENCODE_RELATIVE_PID_MAX_OUT 10.0f
#define PITCH_ENCODE_RELATIVE_PID_MAX_IOUT 0.0f

//yaw 角度环 角度由编码器 PID参数以及 PID最大输出，积分输出
#define YAW_ENCODE_RELATIVE_PID_KP 8.0f
#define YAW_ENCODE_RELATIVE_PID_KI 0.0f
#define YAW_ENCODE_RELATIVE_PID_KD 0.0f
#define YAW_ENCODE_RELATIVE_PID_MAX_OUT 10.0f
#define YAW_ENCODE_RELATIVE_PID_MAX_IOUT 0.0f
/***********************************************END***************************************************/

typedef enum
{
  GIMBAL_ZERO_FORCE = 0, //云台无力
  //GIMBAL_ABSOLUTE_ANGLE, //云台陀螺仪绝对角度控制     空置中
  //GIMBAL_RELATIVE_ANGLE, //云台电机编码值相对角度控制
	GIMBAL_MIXED_AB_RE_ANGLE,
  GIMBAL_MOTIONLESS,     //云台在遥控器无输入一段时间后保持不动，避免陀螺仪漂移
} gimbal_behaviour_e;

typedef struct
{
    const moto_info_t *gimbal_motor_measure;
		pid_struct_t gimbal_motor_absolute_angle_pid;
    pid_struct_t gimbal_motor_relative_angle_pid;
		pid_struct_t gimbal_motor_gyro_pid;
    uint16_t offset_ecd;//相对角中值
    fp32 max_relative_angle; //rad//这两个值直接测出来，换电机的时候也要测注意都是 角度值
    fp32 min_relative_angle; //rad
	  fp32 relative_angle;     //rad
    fp32 relative_angle_set; //rad
    fp32 absolute_angle;     //rad
    fp32 absolute_angle_set; //rad
    fp32 motor_gyro;         //rad/s角速度
    fp32 motor_gyro_set;			//rad/s
		fp32 rc_effective_stroke; //遥控器有效行程 经过死区限制
		fp32 given_current;				//PID计算后给电机的值
} Gimbal_Motor_t;

typedef struct
{
	int16_t pitch_channel;
	int16_t yaw_channel;
	
}Gimbal_Value_Limit_t;

typedef struct
{
    const RC_Type *gimbal_rc_ctrl;
    const fp32 *gimbal_INT_angle_point;
    const fp32 *gimbal_INT_gyro_point;
    Gimbal_Motor_t gimbal_yaw_motor;
    Gimbal_Motor_t gimbal_pitch_motor;
		gimbal_behaviour_e Gimbal_Motor_mode;
		Gimbal_Value_Limit_t gimbal_value_limit;//数据需要处理时存放的结构体
} Gimbal_Control_t;
	 
extern osThreadId vGimbal_TaskHandle;
void Gimbal_Task(void const * argument);
extern const Gimbal_Motor_t *get_yaw_motor_point(void);
extern const Gimbal_Motor_t *get_pitch_motor_point(void);
#ifdef __cplusplus
}
#endif
#endif /*__Gimbal_Task1_H */
