#ifndef __PID_H
#define __PID_H

#include <main.h>

typedef enum 
{
	PID_POSITION_SPEED = 0,
	PID_POSITION_ANGLE,
	PID_DELTA_SPEED
}PID_mode;

typedef struct
{
	PID_mode mode;			//模式	位置式速度；位置式角度；增量式速度
	float Kp;
	float Ki;
	float Kd;
	
	float Max_iout;		//最大积分输出限制
	float Max_out;		//最大总输出限制
	
	float measure;		//测量值
	float target;			//目标值
	float DeadBand;		//死区
	
	float Pout;
	float Iout;
	float Dout;
	float D_item;			//微分项	
	float error[3];		//偏差		0最新
	float OUT;				//输出
}PID_typedef;


void PID_total_init(void);

/**
  * @brief          pid 结构体数据初始化
  * @param[out]     pid: PID结构数据指针
  * @param[in]      mode: PID_POSITION_SPEED: 位置式PID，速度
  *                 			PID_POSITION_ANGLE: 位置式PID，角度
	*  											PID_DELTA_SPEED		：差分式PID，速度
  * @param[in]      kp:PID参数p
	* @param[in] 			ki:PID参数i
	* @param[in] 			kd:PID参数d
  * @param[in]      Max_iout:pid最大积分输出
  * @param[in]      Max_out:pid最大输出
	* @param[in]			deadband:PID死区
  * @retval         none
  */
void PID_Init(PID_typedef *PID,PID_mode Mode,float kp,float ki,float kd,float Max_iout,float Max_out,float deadband);

/**
  * @brief          pid计算
  * @param[out]     pid: PID结构数据指针
  * @param[in]      measure:反馈测量数据
	* @param[in] 			target: 目标值
  * @retval         none
  */
float PID_calc(PID_typedef *PID, float measure, float target);

#endif












