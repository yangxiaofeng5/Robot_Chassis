#include "Gimbal_Task.h"
#include "Gimbal_behaviour.h"
#include "bsp_imu.h"

#include "arm_math.h"
#include "user_lib.h"


//��̨���������������
static Gimbal_Control_t gimbal_control;
//��̨��ʼ��
static void GIMBAL_Init(Gimbal_Control_t *gimbal_init);
//��̨���ݸ���
static void GIMBAL_Feedback_Update(Gimbal_Control_t *gimbal_feedback_update);
//��̨״̬����
static void GIMBAL_Set_Mode(Gimbal_Control_t *gimbal_set_mode);
//��̨״̬�л��������ݣ������������״̬�л���������״̬����Ŀ��ֵ
static void GIMBAL_Mode_Change_Control_Transit(Gimbal_Control_t *gimbal_mode_change);
//������̨������
static void GIMBAL_Set_Contorl(Gimbal_Control_t *gimbal_set_control);
//��̨����pid����
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop);

static void gimbal_motor_absolute_angle_control(Gimbal_Motor_t *gimbal_motor);
static void gimbal_motor_relative_angle_control(Gimbal_Motor_t *gimbal_motor);
static void gimbal_motor_raw_angle_control(Gimbal_Motor_t *gimbal_motor);

//������̨��������ֵ����ԽǶ�
static fp32 motor_ecd_to_angle_change(uint16_t ecd, uint16_t offset_ecd);

//�������ǽǶȿ����£��Կ��Ƶ�Ŀ��ֵ�������Է��������ԽǶ�
static void GIMBAL_absolute_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add);
static void GIMBAL_relative_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add);
static void GIMBAL_PID_Init(Gimbal_PID_t *pid, fp32 maxout, fp32 max_iout, fp32 kp, fp32 ki, fp32 kd);
static fp32 GIMBAL_PID_Calc(Gimbal_PID_t *pid, fp32 get, fp32 set, fp32 error_delta);

static void Gimbal_PID_clear(Gimbal_PID_t *gimbal_pid_clear);

void Gimbal_Task(void const * argument)
{
//	printf("In Gimbal_Task!\n");
	  //�ȴ������������������������
    osDelay(GIMBAL_TASK_INIT_TIME);
	  //��̨��ʼ��
    GIMBAL_Init(&gimbal_control);
	//���ﲦ���ŵ��Լ���������ȥ��
	//�˴�����Ҫ�ӵ��߼��
	for(;;)
	{
		GIMBAL_Set_Mode(&gimbal_control);                    //������̨����ģʽ
		GIMBAL_Mode_Change_Control_Transit(&gimbal_control); //����ģʽ�л� �������ݹ���
    GIMBAL_Feedback_Update(&gimbal_control);             //��̨���ݷ���
    GIMBAL_Set_Contorl(&gimbal_control);                 //������̨������
    GIMBAL_Control_loop(&gimbal_control);                //��̨����PID����		
		osDelay (10);
	}
}

void gimbal_total_pid_clear(Gimbal_Control_t *gimbal_clear)                                               
	{                                                                                        
	Gimbal_PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_absolute_angle_pid);   
	Gimbal_PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_relative_angle_pid);   
	pid_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_gyro_pid);                    
																																												 
	Gimbal_PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_absolute_angle_pid); 
	Gimbal_PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_relative_angle_pid); 
	pid_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_gyro_pid);                  
	}

static void GIMBAL_Init(Gimbal_Control_t *gimbal_init)
{
//�����ò��ϸ�����pid
//    static const fp32 Pitch_speed_pid[3] = {PITCH_SPEED_PID_KP, PITCH_SPEED_PID_KI, PITCH_SPEED_PID_KD};
//    static const fp32 Yaw_speed_pid[3] = {YAW_SPEED_PID_KP, YAW_SPEED_PID_KI, YAW_SPEED_PID_KD};
    //�������ָ���ȡ
    gimbal_init->gimbal_yaw_motor.gimbal_motor_measure = get_Yaw_Gimbal_Motor_Measure_Point();
    gimbal_init->gimbal_pitch_motor.gimbal_motor_measure = get_Pitch_Gimbal_Motor_Measure_Point();
    //����������ָ���ȡ
    gimbal_init->gimbal_INT_angle_point = get_INS_angle_point();
    gimbal_init->gimbal_INT_gyro_point = get_MPU6500_Gyro_Data_Point();
    //ң��������ָ���ȡ
    gimbal_init->gimbal_rc_ctrl = get_remote_control_point();
    //��ʼ�����ģʽ
    gimbal_init->gimbal_yaw_motor.gimbal_motor_mode = gimbal_init->gimbal_yaw_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_init->gimbal_pitch_motor.gimbal_motor_mode = gimbal_init->gimbal_pitch_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    //��ʼ��yaw���pid
    GIMBAL_PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_absolute_angle_pid, 
		YAW_GYRO_ABSOLUTE_PID_MAX_OUT, 
		YAW_GYRO_ABSOLUTE_PID_MAX_IOUT, 
		YAW_GYRO_ABSOLUTE_PID_KP, 
		YAW_GYRO_ABSOLUTE_PID_KI, 
		YAW_GYRO_ABSOLUTE_PID_KD);
    GIMBAL_PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_relative_angle_pid, 
		YAW_ENCODE_RELATIVE_PID_MAX_OUT, YAW_ENCODE_RELATIVE_PID_MAX_IOUT, 
		YAW_ENCODE_RELATIVE_PID_KP, 
		YAW_ENCODE_RELATIVE_PID_KI, 
		YAW_ENCODE_RELATIVE_PID_KD);
		pid_init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_gyro_pid, 
				YAW_SPEED_PID_KP,
				YAW_SPEED_PID_KI,
				YAW_SPEED_PID_KD,
				YAW_SPEED_PID_MAX_OUT,
				YAW_SPEED_PID_MAX_IOUT);
		
    //��ʼ��pitch���pid
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
		
    //�������PID
    gimbal_total_pid_clear(gimbal_init);

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
    //��̨���ݸ���
		//pitch��
    gimbal_feedback_update->gimbal_pitch_motor.absolute_angle = *(gimbal_feedback_update->gimbal_INT_angle_point + INS_PITCH_ADDRESS_OFFSET);
    gimbal_feedback_update->gimbal_pitch_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_pitch_motor.gimbal_motor_measure->ecd,
                                                                                          gimbal_feedback_update->gimbal_pitch_motor.offset_ecd);
    gimbal_feedback_update->gimbal_pitch_motor.motor_gyro = *(gimbal_feedback_update->gimbal_INT_gyro_point + INS_GYRO_Y_ADDRESS_OFFSET);
		
    gimbal_feedback_update->gimbal_yaw_motor.absolute_angle = *(gimbal_feedback_update->gimbal_INT_angle_point + INS_YAW_ADDRESS_OFFSET);
    gimbal_feedback_update->gimbal_yaw_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_yaw_motor.gimbal_motor_measure->ecd,
                                                                                        gimbal_feedback_update->gimbal_yaw_motor.offset_ecd);
    gimbal_feedback_update->gimbal_yaw_motor.motor_gyro = arm_cos_f32(gimbal_feedback_update->gimbal_pitch_motor.relative_angle) * (*(gimbal_feedback_update->gimbal_INT_gyro_point + INS_GYRO_Z_ADDRESS_OFFSET))
                                                        - arm_sin_f32(gimbal_feedback_update->gimbal_pitch_motor.relative_angle) * (*(gimbal_feedback_update->gimbal_INT_gyro_point + INS_GYRO_X_ADDRESS_OFFSET));
}

//������ԽǶ�
static fp32 motor_ecd_to_angle_change(uint16_t ecd, uint16_t offset_ecd)
{
    int32_t relative_ecd = ecd - offset_ecd;
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
//��̨״̬�л����棬����״̬�л�����
static void GIMBAL_Mode_Change_Control_Transit(Gimbal_Control_t *gimbal_mode_change)
{
    if (gimbal_mode_change == NULL)
    {
        return;
    }
    //yaw���״̬���л�������������
    if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_yaw_motor.raw_cmd_current = gimbal_mode_change->gimbal_yaw_motor.current_set = gimbal_mode_change->gimbal_yaw_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_GYRO && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        gimbal_mode_change->gimbal_yaw_motor.absolute_angle_set = gimbal_mode_change->gimbal_yaw_motor.absolute_angle;
    }
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCONDE && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        gimbal_mode_change->gimbal_yaw_motor.relative_angle_set = gimbal_mode_change->gimbal_yaw_motor.relative_angle;
    }
    gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode;

    //pitch���״̬���л���������
    if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_pitch_motor.raw_cmd_current = gimbal_mode_change->gimbal_pitch_motor.current_set = gimbal_mode_change->gimbal_pitch_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_GYRO && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        gimbal_mode_change->gimbal_pitch_motor.absolute_angle_set = gimbal_mode_change->gimbal_pitch_motor.absolute_angle;
    }
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCONDE && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        gimbal_mode_change->gimbal_pitch_motor.relative_angle_set = gimbal_mode_change->gimbal_pitch_motor.relative_angle;
    }

    gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode;
}

//��̨����������
static void GIMBAL_Set_Contorl(Gimbal_Control_t *gimbal_set_control)
{
    if (gimbal_set_control == NULL)
    {
        return;
    }

    fp32 add_yaw_angle = 0.0f;
    fp32 add_pitch_angle = 0.0f;

    gimbal_behaviour_control_set(&add_yaw_angle, &add_pitch_angle, gimbal_set_control);
    //yaw���ģʽ����
    if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //rawģʽ�£�ֱ�ӷ��Ϳ���ֵ
        gimbal_set_control->gimbal_yaw_motor.raw_cmd_current = add_yaw_angle;
    }
    else if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyroģʽ�£������ǽǶȿ���
        GIMBAL_absolute_angle_limit(&gimbal_set_control->gimbal_yaw_motor, add_yaw_angle);
    }
    else if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //encondeģʽ�£��������Ƕȿ���
        GIMBAL_relative_angle_limit(&gimbal_set_control->gimbal_yaw_motor, add_yaw_angle);
    }

    //pitch���ģʽ����
    if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //rawģʽ�£�ֱ�ӷ��Ϳ���ֵ
        gimbal_set_control->gimbal_pitch_motor.raw_cmd_current = add_pitch_angle;
    }
    else if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyroģʽ�£������ǽǶȿ���
        GIMBAL_absolute_angle_limit(&gimbal_set_control->gimbal_pitch_motor, add_pitch_angle);
    }
    else if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //encondeģʽ�£��������Ƕȿ���
        GIMBAL_relative_angle_limit(&gimbal_set_control->gimbal_pitch_motor, add_pitch_angle);
    }
}

//������ ����������
static void GIMBAL_absolute_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add)
{
    static fp32 bias_angle;
    static fp32 angle_set;
    if (gimbal_motor == NULL)
    {
        return;
    }
    //��ǰ�������Ƕ�
    bias_angle = rad_format(gimbal_motor->absolute_angle_set - gimbal_motor->absolute_angle);
    //��̨��ԽǶ�+ ���Ƕ� + �����Ƕ� ������� ����е�Ƕ�
    if (gimbal_motor->relative_angle + bias_angle + add > gimbal_motor->max_relative_angle)
    {
        //�����������е�Ƕȿ��Ʒ���
        if (add > 0.0f)
        {
            //�����һ��������ӽǶȣ�
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
    gimbal_motor->relative_angle_set += add;
    //�Ƿ񳬹���� ��Сֵ
    if (gimbal_motor->relative_angle_set > gimbal_motor->max_relative_angle)
    {
        gimbal_motor->relative_angle_set = gimbal_motor->max_relative_angle;
    }
    else if (gimbal_motor->relative_angle_set < gimbal_motor->min_relative_angle)
    {
        gimbal_motor->relative_angle_set = gimbal_motor->min_relative_angle;
    }
}

//��̨����״̬ʹ�ò�ͬ����pid
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop)
{
    if (gimbal_control_loop == NULL)
    {
        return;
    }
    //yaw��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro�Ƕȿ���
        gimbal_motor_absolute_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }

    //pitch��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro�Ƕȿ���
        gimbal_motor_absolute_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
}

static void gimbal_motor_absolute_angle_control(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_absolute_angle_pid, gimbal_motor->absolute_angle, gimbal_motor->absolute_angle_set, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = pid_calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro_set,gimbal_motor->motor_gyro);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

static void gimbal_motor_relative_angle_control(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }

    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, gimbal_motor->relative_angle, gimbal_motor->relative_angle_set, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = pid_calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro_set, gimbal_motor->motor_gyro);
    //����ֵ��ֵ
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

//�����PID�õ����ڴ�.h�ļ������̨pid�ṹ��
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

//pid��������
static void Gimbal_PID_clear(Gimbal_PID_t *gimbal_pid_clear)
{
    if (gimbal_pid_clear == NULL)
    {
        return;
    }
    gimbal_pid_clear->err = gimbal_pid_clear->set = gimbal_pid_clear->get = 0.0f;
    gimbal_pid_clear->out = gimbal_pid_clear->Pout = gimbal_pid_clear->Iout = gimbal_pid_clear->Dout = 0.0f;
}

