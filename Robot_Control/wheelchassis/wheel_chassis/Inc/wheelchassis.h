#ifndef __WHEELCHASSIS_H
#define __WHEELCHASSIS_H

#include "main.h"
#include "can.h"
#include "removecontrol.h"

#define PI 3.1415926
#define TIMESTEP 0.001f //时间步长，和定时中断频率有关
#define REMOVE_K 0.02 //底盘控制值和遥控通道值的比例关系

typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float IntegralLimit; // 积分限幅
    float OutputLimit;   // 输出限幅
}PID_Params;

typedef struct
{
    float Chassis_R;
    float Wheel_R;
    float Acceleration;
    float Deceleration;
}Chassis_Params;

//电机的数据
typedef struct
{
    float angle;
    float speed;
    float TorqueCurrent;
    int8_t temperature;
}MotorData;

typedef struct
{
    float Vx;
    float Vy;
    float Vw;

}MotionControl;

void wheelChassis_Init(void);

void M3508_CAN_SendCurrent(float *current);
void M3508_GetFeedbackData(MotorData *data);

void wheelChassis_MotorSpeedControl(float *ExpectSpeed, float *Current_Output, MotorData *data);

#endif
