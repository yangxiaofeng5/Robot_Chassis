#ifndef __Robot_Status_H
#define __Robot_Status_H
#ifdef __cplusplus
 extern "C" {
#endif

typedef enum
{
	ROBOT_STATUS_INIT = 0;		//������������ʼ��״̬
	ROBOT_STATUS_STOPED = 1;	//����������ֹͣ
	ROBOT_STATUS_MOVING = 2;	//�����������˶�
	ROBOT_STATUS_FAILYUE = 3;	//�������������ֹ���,Ŀ�ⲻ̫��Ҫ
	
} Robot_Status_t;	 
	 
	 
#ifdef __cplusplus
}
#endif
#endif /*__Gimbal_Task1_H */
