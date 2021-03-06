#include "Gimbal_Task.h"
#include "Gimbal_behaviour.h"
#include "bsp_imu.h"

#include "arm_math.h"
#include "user_lib.h"

//电机编码值规整 0—8191
#define ECD_Format(ecd)         \
    {                           \
        if ((ecd) > ecd_range)  \
            (ecd) -= ecd_range; \
        else if ((ecd) < 0)     \
            (ecd) += ecd_range; \
    }

#define gimbal_total_pid_clear(gimbal_clear)                                                   \
    {                                                                                          \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_absolute_angle_pid);   \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_relative_angle_pid);   \
        pid_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_gyro_pid);                    \
                                                                                               \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_absolute_angle_pid); \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_relative_angle_pid); \
        pid_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_gyro_pid);                  \
    }		
		
//云台控制所有相关数据
static Gimbal_Control_t gimbal_control;
//发送的电机变量
static int16_t Yaw_Set_Current = 0, Pitch_Set_Current = 0;
		
//云台初始化
static void GIMBAL_Init(Gimbal_Control_t *gimbal_init);
//云台数据更新
static void GIMBAL_Feedback_Update(Gimbal_Control_t *gimbal_feedback_update);
//云台状态设置
static void GIMBAL_Set_Mode(Gimbal_Control_t *gimbal_set_mode);
//云台状态切换保存数据，例如从陀螺仪状态切换到编码器状态保存目标值
static void GIMBAL_Mode_Change_Control_Transit(Gimbal_Control_t *gimbal_mode_change);
//设置云台控制量
static void GIMBAL_Set_Contorl(Gimbal_Control_t *gimbal_set_control);
//云台控制pid计算
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop);
		
//云台相对位置设置及更新值
static void GIMBAL_Cali_set(Gimbal_Control_t *gimbal_cali_set);

static void gimbal_motor_absolute_angle_control(Gimbal_Motor_t *gimbal_motor);
static void gimbal_motor_relative_angle_control(Gimbal_Motor_t *gimbal_motor);
static void gimbal_motor_raw_angle_control(Gimbal_Motor_t *gimbal_motor);

//计算云台电机相对中值的相对角度
static fp32 motor_ecd_to_angle_change(uint16_t rotor_angle, uint16_t offset_ecd);

//在陀螺仪角度控制下，对控制的目标值进限制以防超最大相对角度
static void GIMBAL_absolute_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add);
static void GIMBAL_relative_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add);
static void GIMBAL_PID_Init(Gimbal_PID_t *pid, fp32 maxout, fp32 max_iout, fp32 kp, fp32 ki, fp32 kd);
static fp32 GIMBAL_PID_Calc(Gimbal_PID_t *pid, fp32 get, fp32 set, fp32 error_delta);

static void Gimbal_PID_clear(Gimbal_PID_t *gimbal_pid_clear);

void Gimbal_Task(void const * argument)
{
//	printf("In Gimbal_Task!\n");
	  //等待陀螺仪任务更新陀螺仪数据
    osDelay(GIMBAL_TASK_INIT_TIME);
	  //云台初始化
    GIMBAL_Init(&gimbal_control);
	//这里拨弹放到自己的任务里去了
	//此处还需要加掉线检测
	for(;;)
	{
		MPU6500_GET_DATA();																	 //更新陀螺仪的数据
		
		gimbal_control.gimbal_pitch_motor.relative_angle = motor_ecd_to_angle_change(gimbal_control.gimbal_pitch_motor.gimbal_motor_measure->rotor_angle,
                                                                                 gimbal_control.gimbal_pitch_motor.offset_ecd);    
		gimbal_control.gimbal_yaw_motor.relative_angle = motor_ecd_to_angle_change(gimbal_control.gimbal_yaw_motor.gimbal_motor_measure->rotor_angle,
                                                                               gimbal_control.gimbal_yaw_motor.offset_ecd);
		if(remote_control.ch4 == 0)
		{
			if(gimbal_control.gimbal_pitch_motor.relative_angle_set == 0)
				gimbal_control.gimbal_pitch_motor.relative_angle_set = gimbal_control.gimbal_pitch_motor.relative_angle;
				gimbal_control.gimbal_pitch_motor.given_current = pid_calc(&gimbal_control.gimbal_yaw_motor.gimbal_motor_gyro_pid,
																																gimbal_control.gimbal_pitch_motor.relative_angle_set,
																																gimbal_control.gimbal_pitch_motor.relative_angle);
		}
		else
		{
			gimbal_control.gimbal_pitch_motor.relative_angle_set = 0;
			gimbal_control.gimbal_pitch_motor.relative_angle_set = remote_control.ch4*0.0088228f;
		
		}	
		gimbal_control.gimbal_pitch_motor.given_current = pid_calc(&gimbal_control.gimbal_pitch_motor.gimbal_motor_gyro_pid,
																																gimbal_control.gimbal_pitch_motor.relative_angle_set,
																																imu.wx);
//		GIMBAL_Set_Mode(&gimbal_control);                    //设置云台控制模式
//		GIMBAL_Mode_Change_Control_Transit(&gimbal_control); //控制模式切换 控制数据过渡
//    GIMBAL_Feedback_Update(&gimbal_control);             //云台数据反馈
//    GIMBAL_Set_Contorl(&gimbal_control);                 //设置云台控制量
//    GIMBAL_Control_loop(&gimbal_control);                //云台控制PID计算	
//		
//#if YAW_TURN
//        Yaw_Set_Current = gimbal_control.gimbal_yaw_motor.given_current;
//#else
//        Yaw_Set_Current = -gimbal_control.gimbal_yaw_motor.given_current;
//#endif

//#if PITCH_TURN
//        Pitch_Set_Current = gimbal_control.gimbal_pitch_motor.given_current;
//#else
//        Pitch_Set_Current = -gimbal_control.gimbal_pitch_motor.given_current;
//#endif
		if(remote_control .switch_left == 2)
		set_motor_voltage(0, 													//设置电机速度
												0, 
												0, 
												0, 
												0);	
		else
		set_motor_voltage(0, 													//设置电机速度
												-gimbal_control.gimbal_pitch_motor.given_current, 
												0, 
												0, 
												0);	
		
		wave_form_data[0] = gimbal_control.gimbal_pitch_motor.given_current;
		wave_form_data[1] = gimbal_control.gimbal_pitch_motor.gimbal_motor_measure->rotor_angle;
		
		wave_form_data[2] = gimbal_control.gimbal_pitch_motor.relative_angle_set*100;
		wave_form_data[3] = gimbal_control.gimbal_pitch_motor.relative_angle*100;

		wave_form_data[4] = imu.pit;
		wave_form_data[5] = imu.yaw;
		wave_form_data[6] = imu.rol;
		
		wave_form_data[7] = imu.temp;
		shanwai_send_wave_form();
		osDelay (10);
	}
}


const Gimbal_Motor_t *get_yaw_motor_point(void)
{
    return &gimbal_control.gimbal_yaw_motor;
}

const Gimbal_Motor_t *get_pitch_motor_point(void)
{
    return &gimbal_control.gimbal_pitch_motor;
}

static void GIMBAL_Init(Gimbal_Control_t *gimbal_init)
{
    //电机数据指针获取
	//这里云台电机的编号需要根据电机的编号来更改
    gimbal_init->gimbal_yaw_motor.gimbal_motor_measure = get_Yaw_Gimbal_Motor_Measure_Point();
    gimbal_init->gimbal_pitch_motor.gimbal_motor_measure = get_Pitch_Gimbal_Motor_Measure_Point();
	
	//陀螺仪数据指针获取
    gimbal_init->gimbal_INT_angle_point = get_INS_angle_point();
    gimbal_init->gimbal_INT_gyro_point = get_MPU6500_Gyro_Data_Point();
	
    //遥控器数据指针获取
    gimbal_init->gimbal_rc_ctrl = get_remote_control_point();
    //初始化电机模式
    gimbal_init->gimbal_yaw_motor.gimbal_motor_mode = gimbal_init->gimbal_yaw_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_init->gimbal_pitch_motor.gimbal_motor_mode = gimbal_init->gimbal_pitch_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;

    //初始化yaw电机pid
    GIMBAL_PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_absolute_angle_pid, 
										YAW_GYRO_ABSOLUTE_PID_MAX_OUT, 
										YAW_GYRO_ABSOLUTE_PID_MAX_IOUT, 
										YAW_GYRO_ABSOLUTE_PID_KP, 
										YAW_GYRO_ABSOLUTE_PID_KI, 
										YAW_GYRO_ABSOLUTE_PID_KD);
    GIMBAL_PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_relative_angle_pid, 
										YAW_ENCODE_RELATIVE_PID_MAX_OUT, 
										YAW_ENCODE_RELATIVE_PID_MAX_IOUT, 
										YAW_ENCODE_RELATIVE_PID_KP, 
										YAW_ENCODE_RELATIVE_PID_KI, 
										YAW_ENCODE_RELATIVE_PID_KD);
		pid_init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_gyro_pid, 
										YAW_SPEED_PID_KP,
										YAW_SPEED_PID_KI,
										YAW_SPEED_PID_KD,
										YAW_SPEED_PID_MAX_OUT,
										YAW_SPEED_PID_MAX_IOUT);
		
    //初始化pitch电机pid
    GIMBAL_PID_Init(&gimbal_init->gimbal_pitch_motor.gimbal_motor_absolute_angle_pid, 
										PITCH_GYRO_ABSOLUTE_PID_MAX_OUT, 
										PITCH_GYRO_ABSOLUTE_PID_MAX_IOUT, 
										PITCH_GYRO_ABSOLUTE_PID_KP, 
										PITCH_GYRO_ABSOLUTE_PID_KI, 
										PITCH_GYRO_ABSOLUTE_PID_KD);
    GIMBAL_PID_Init(&gimbal_init->gimbal_pitch_motor.gimbal_motor_relative_angle_pid, 
										PITCH_ENCODE_RELATIVE_PID_MAX_OUT, 
										PITCH_ENCODE_RELATIVE_PID_MAX_IOUT, 
										PITCH_ENCODE_RELATIVE_PID_KP, 
										PITCH_ENCODE_RELATIVE_PID_KI, 
										PITCH_ENCODE_RELATIVE_PID_KD);
    		pid_init(&gimbal_init->gimbal_pitch_motor.gimbal_motor_gyro_pid, 
										PITCH_SPEED_PID_KP,
										PITCH_SPEED_PID_KI,
										PITCH_SPEED_PID_KD,
										PITCH_SPEED_PID_MAX_OUT,
										PITCH_SPEED_PID_MAX_IOUT);
				
		//清除所有PID
    gimbal_total_pid_clear(gimbal_init);
		
		//设置云台电机的初始值
		GIMBAL_Cali_set(gimbal_init);
		
    GIMBAL_Feedback_Update(gimbal_init);

    gimbal_init->gimbal_yaw_motor.absolute_angle_set = gimbal_init->gimbal_yaw_motor.absolute_angle;
    gimbal_init->gimbal_yaw_motor.relative_angle_set = gimbal_init->gimbal_yaw_motor.relative_angle;
    gimbal_init->gimbal_yaw_motor.motor_gyro_set = gimbal_init->gimbal_yaw_motor.motor_gyro;


    gimbal_init->gimbal_pitch_motor.absolute_angle_set = gimbal_init->gimbal_pitch_motor.absolute_angle;
    gimbal_init->gimbal_pitch_motor.relative_angle_set = gimbal_init->gimbal_pitch_motor.relative_angle;
    gimbal_init->gimbal_pitch_motor.motor_gyro_set = gimbal_init->gimbal_pitch_motor.motor_gyro;
}

static void GIMBAL_Set_Mode(Gimbal_Control_t *gimbal_set_mode)
{
    if (gimbal_set_mode == NULL)
    {
        return;
    }
    gimbal_behaviour_mode_set(gimbal_set_mode);
}


static void GIMBAL_Feedback_Update(Gimbal_Control_t *gimbal_feedback_update)
{
    if (gimbal_feedback_update == NULL)
    {
        return;
    }
    //云台数据更新//这里的off_set的值是我需要实际测量的，后期用算法解决
		//pitch轴
    gimbal_feedback_update->gimbal_pitch_motor.absolute_angle = *(gimbal_feedback_update->gimbal_INT_angle_point + INS_ROLL_ADDRESS_OFFSET);
    gimbal_feedback_update->gimbal_pitch_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_pitch_motor.gimbal_motor_measure->rotor_angle,
                                                                                          gimbal_feedback_update->gimbal_pitch_motor.offset_ecd);
    gimbal_feedback_update->gimbal_pitch_motor.motor_gyro = *(gimbal_feedback_update->gimbal_INT_gyro_point + 2);//原为Y
		
    gimbal_feedback_update->gimbal_yaw_motor.absolute_angle = *(gimbal_feedback_update->gimbal_INT_angle_point + INS_YAW_ADDRESS_OFFSET);
    gimbal_feedback_update->gimbal_yaw_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_yaw_motor.gimbal_motor_measure->rotor_angle,
                                                                                        gimbal_feedback_update->gimbal_yaw_motor.offset_ecd);
    gimbal_feedback_update->gimbal_yaw_motor.motor_gyro = arm_cos_f32(gimbal_feedback_update->gimbal_pitch_motor.relative_angle) * 
																																	(*(gimbal_feedback_update->gimbal_INT_gyro_point + 1))
                                                        - arm_sin_f32(gimbal_feedback_update->gimbal_pitch_motor.relative_angle) * 
																																	(*(gimbal_feedback_update->gimbal_INT_gyro_point + 0));//原为X
}


//计算相对角度
static fp32 motor_ecd_to_angle_change(uint16_t rotor_angle, uint16_t offset_ecd)
{
    int32_t relative_ecd = rotor_angle - offset_ecd;
    if (relative_ecd > Half_ecd_range)
    {
        relative_ecd -= ecd_range;
    }
    else if (relative_ecd < -Half_ecd_range)
    {
        relative_ecd += ecd_range;
    }

    return relative_ecd * Motor_Ecd_to_Rad;
}
//云台状态切换保存，用于状态切换过渡
static void GIMBAL_Mode_Change_Control_Transit(Gimbal_Control_t *gimbal_mode_change)
{
    if (gimbal_mode_change == NULL)
    {
        return;
    }
    //yaw电机状态机切换保存数据三种
    if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && 
				gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_yaw_motor.raw_cmd_current = 
				gimbal_mode_change->gimbal_yaw_motor.current_set = 
				gimbal_mode_change->gimbal_yaw_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_GYRO && 
						 gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        gimbal_mode_change->gimbal_yaw_motor.absolute_angle_set = gimbal_mode_change->gimbal_yaw_motor.absolute_angle;
    }
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCONDE && 
						 gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        gimbal_mode_change->gimbal_yaw_motor.relative_angle_set = gimbal_mode_change->gimbal_yaw_motor.relative_angle;
    }
    gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode;

    //pitch电机状态机切换保存数据
    if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && 
				gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_pitch_motor.raw_cmd_current = 
				gimbal_mode_change->gimbal_pitch_motor.current_set = 
				gimbal_mode_change->gimbal_pitch_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_GYRO && 
						 gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        gimbal_mode_change->gimbal_pitch_motor.absolute_angle_set = gimbal_mode_change->gimbal_pitch_motor.absolute_angle;
    }
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCONDE && 
						 gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        gimbal_mode_change->gimbal_pitch_motor.relative_angle_set = gimbal_mode_change->gimbal_pitch_motor.relative_angle;
    }

    gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode;
}

//云台控制量设置
static void GIMBAL_Set_Contorl(Gimbal_Control_t *gimbal_set_control)
{
    if (gimbal_set_control == NULL)
    {
        return;
    }

    fp32 add_yaw_angle = 0.0f;
    fp32 add_pitch_angle = 0.0f;

    gimbal_behaviour_control_set(&add_yaw_angle, &add_pitch_angle, gimbal_set_control);
    
		wave_form_data[6] = add_yaw_angle*100;
		wave_form_data[7] = add_pitch_angle*100;
		
//		shanwai_send_wave_form();
		//yaw电机模式控制
    if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw模式下，直接发送控制值
        gimbal_set_control->gimbal_yaw_motor.raw_cmd_current = add_yaw_angle;
    }
    else if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro模式下，陀螺仪角度控制
        GIMBAL_absolute_angle_limit(&gimbal_set_control->gimbal_yaw_motor, add_yaw_angle);
    }
    else if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde模式下，电机编码角度控制
        GIMBAL_relative_angle_limit(&gimbal_set_control->gimbal_yaw_motor, add_yaw_angle);
    }

    //pitch电机模式控制
    if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw模式下，直接发送控制值
        gimbal_set_control->gimbal_pitch_motor.raw_cmd_current = add_pitch_angle;
    }
    else if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro模式下，陀螺仪角度控制
        GIMBAL_absolute_angle_limit(&gimbal_set_control->gimbal_pitch_motor, add_pitch_angle);
    }
    else if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde模式下，电机编码角度控制
        GIMBAL_relative_angle_limit(&gimbal_set_control->gimbal_pitch_motor, add_pitch_angle);
    }
		
}

//陀螺仪 控制量限制
static void GIMBAL_absolute_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add)
{
    static fp32 bias_angle;
    static fp32 angle_set;
    if (gimbal_motor == NULL)
    {
        return;
    }
    //当前控制误差角度
    bias_angle = rad_format(gimbal_motor->absolute_angle_set - gimbal_motor->absolute_angle);
    //云台相对角度+ 误差角度 + 新增角度 如果大于 最大机械角度
    if (gimbal_motor->relative_angle + bias_angle + add > gimbal_motor->max_relative_angle)
    {
        //如果是往最大机械角度控制方向
        if (add > 0.0f)
        {
            //计算出一个最大的添加角度，
            add = gimbal_motor->max_relative_angle - gimbal_motor->relative_angle - bias_angle;
        }
    }
    else if (gimbal_motor->relative_angle + bias_angle + add < gimbal_motor->min_relative_angle)
    {
        if (add < 0.0f)
        {
            add = gimbal_motor->min_relative_angle - gimbal_motor->relative_angle - bias_angle;
        }
    }
    angle_set = gimbal_motor->absolute_angle_set;
    gimbal_motor->absolute_angle_set = rad_format(angle_set + add);
}

static void GIMBAL_relative_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    gimbal_motor->relative_angle_set = gimbal_motor->relative_angle_set + add;
		    //是否超过最大 最小值
    if (gimbal_motor->relative_angle_set > gimbal_motor->max_relative_angle)
    {
        gimbal_motor->relative_angle_set = gimbal_motor->max_relative_angle;
//				wave_form_data[7] = gimbal_motor->relative_angle_set*100000;
    }
    else if (gimbal_motor->relative_angle_set < gimbal_motor->min_relative_angle)
    {
        gimbal_motor->relative_angle_set = gimbal_motor->min_relative_angle;
//				wave_form_data[7] = gimbal_motor->relative_angle_set*100000;
    }
}
//经过测试将云台的最大最小值记录下来
static void GIMBAL_Cali_set(Gimbal_Control_t *gimbal_cali_set)
{
	 if (gimbal_cali_set == NULL)
   {
		return;
   }
       
	gimbal_control.gimbal_yaw_motor.offset_ecd 						= YAW_ENCODE_OFFEST;
	gimbal_control.gimbal_yaw_motor.max_relative_angle 		= rad_format(YAW_MAX_ENCODE_ANGLE);
	gimbal_control.gimbal_yaw_motor.min_relative_angle 		= rad_format(YAW_MIN_ENCODE_ANGLE);
	gimbal_control.gimbal_pitch_motor.offset_ecd 					= PITCH_ENCODE_OFFEST;
	gimbal_control.gimbal_pitch_motor.max_relative_angle 	= rad_format(PITCH_MAX_ENCODE_ANGLE);
	gimbal_control.gimbal_pitch_motor.min_relative_angle 	= rad_format(PITCH_MIN_ENCODE_ANGLE);
	 
	 
}



//云台控制状态使用不同控制pid
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop)
{
    if (gimbal_control_loop == NULL)
    {
        return;
    }
    //yaw不同模式对于不同的控制函数
    if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw控制
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro角度控制
        gimbal_motor_absolute_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde角度控制
        gimbal_motor_relative_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }

    //pitch不同模式对于不同的控制函数
    if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw控制
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro角度控制
        gimbal_motor_absolute_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde角度控制
        gimbal_motor_relative_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
}

static void gimbal_motor_absolute_angle_control(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    //角度环，速度环串级pid调试
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_absolute_angle_pid, 
																										gimbal_motor->absolute_angle, 
																										gimbal_motor->absolute_angle_set, 
																										gimbal_motor->motor_gyro);
    gimbal_motor->current_set = pid_calc(&gimbal_motor->gimbal_motor_gyro_pid, 
																					gimbal_motor->motor_gyro_set,
																					gimbal_motor->motor_gyro);
    //控制值赋值
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

static void gimbal_motor_relative_angle_control(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }

    //角度环，速度环串级pid调试
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, 
																										gimbal_motor->relative_angle, 
																										gimbal_motor->relative_angle_set, 
																										gimbal_motor->motor_gyro);
    gimbal_motor->current_set = pid_calc(&gimbal_motor->gimbal_motor_gyro_pid, 
																					gimbal_motor->motor_gyro_set, 
																					gimbal_motor->motor_gyro);
    //控制值赋值
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}
static void gimbal_motor_raw_angle_control(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    gimbal_motor->current_set = gimbal_motor->raw_cmd_current;
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

//这里的PID用的是在此.h文件里的云台pid结构体
static void GIMBAL_PID_Init(Gimbal_PID_t *pid, fp32 maxout, fp32 max_iout, fp32 kp, fp32 ki, fp32 kd)
{
    if (pid == NULL)
    {
        return;
    }
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->err = 0.0f;
    pid->get = 0.0f;

    pid->max_iout = max_iout;
    pid->max_out = maxout;
}

static fp32 GIMBAL_PID_Calc(Gimbal_PID_t *pid, fp32 get, fp32 set, fp32 error_delta)
{
    fp32 err;
    if (pid == NULL)
    {
        return 0.0f;
    }
    pid->get = get;
    pid->set = set;

    err = set - get;
    pid->err = rad_format(err);
    pid->Pout = pid->kp * pid->err;
    pid->Iout += pid->ki * pid->err;
    pid->Dout = pid->kd * error_delta;
    abs_limit(&pid->Iout, pid->max_iout);
    pid->out = pid->Pout + pid->Iout + pid->Dout;
    abs_limit(&pid->out, pid->max_out);
    return pid->out;
}

//pid数据清理
static void Gimbal_PID_clear(Gimbal_PID_t *gimbal_pid_clear)
{
    if (gimbal_pid_clear == NULL)
    {
        return;
    }
    gimbal_pid_clear->err = gimbal_pid_clear->set = gimbal_pid_clear->get = 0.0f;
    gimbal_pid_clear->out = gimbal_pid_clear->Pout = gimbal_pid_clear->Iout = gimbal_pid_clear->Dout = 0.0f;
}

