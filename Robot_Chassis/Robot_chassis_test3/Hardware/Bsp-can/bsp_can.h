/****************************************************************************
 *  Copyright (C) 2018 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#ifndef __BSP_CAN
#define __BSP_CAN

#include "can.h"

#define FEEDBACK_ID_BASE      0x201
#define CAN_CONTROL_ID_BASE   0x1ff
#define CAN_CONTROL_ID_EXTEND 0x2ff
#define MOTOR_MAX_NUM         7
#define MOTOR_4_YAW						5
#define MOTOR_5_PITCH					4

#define MOTOR_6_TRIGGER				6

//rm电机统一数据结构体，在这里我多了id，对比
/**
typedef struct
{
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    int16_t last_ecd;
} motor_measure_t;
**/
typedef struct
{
    uint16_t can_id;
    uint16_t rotor_angle;
    int16_t  rotor_speed;
    int16_t  torque_current;
    uint8_t  temp;
		uint16_t ecd;
		uint16_t last_ecd;
		int16_t  set_voltage;
		uint16_t current_set;
		float motor_gyro_set;
}moto_info_t;

void can_user_init(CAN_HandleTypeDef* hcan);
void set_motor_voltage(uint8_t id_range, int16_t v1, int16_t v2, int16_t v3, int16_t v4);

//返回底盘电机变量地址，通过指针方式获取原始数据,i的范围是0-3，对应0x201-0x204,
extern const moto_info_t *get_Chassis_Motor_Measure_Point(uint8_t i);
//返回yaw电机变量地址，通过指针方式获取原始数据
extern const moto_info_t *get_Yaw_Gimbal_Motor_Measure_Point(void);
//返回pitch电机变量地址，通过指针方式获取原始数据
extern const moto_info_t *get_Pitch_Gimbal_Motor_Measure_Point(void);
//返回trigger电机变量地址，通过指针方式获取原始数据
extern const moto_info_t *get_Trigger_Motor_Measure_Point(void);

#endif
