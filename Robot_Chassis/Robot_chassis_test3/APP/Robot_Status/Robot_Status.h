#ifndef __Robot_Status_H
#define __Robot_Status_H
#ifdef __cplusplus
 extern "C" {
#endif

typedef enum
{
	ROBOT_STATUS_INIT = 0;		//机器人整车初始化状态
	ROBOT_STATUS_STOPED = 1;	//机器人整车停止
	ROBOT_STATUS_MOVING = 2;	//机器人整车运动
	ROBOT_STATUS_FAILYUE = 3;	//机器人整车出现故障,目测不太需要
	
} Robot_Status_t;	 
	 
	 
#ifdef __cplusplus
}
#endif
#endif /*__Gimbal_Task1_H */
