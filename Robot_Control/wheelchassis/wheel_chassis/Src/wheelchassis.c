#include "wheelchassis.h"

#include <math.h>

#include "ibus.h"

uint8_t G_FeedbackData[4][8]; // 反馈数据
MotionControl G_Motion; // 运动控制结构体
PID_Params G_Speed_PID_Params; // 速度环PID
PID_Params G_DifSpeed_PID_Params; // 差速环PID
PID_Params G_Angle_PID_Params; // 角度环PID
Chassis_Params G_Chassis_Params; // 运动参数
extern ProcessedChannelValue G_Value; //遥控器的通道值
extern TIM_HandleTypeDef htim6;


static void M3508_CAN_SendData(uint8_t *data);

static void M3508_CAN_Config(void);

void M3508_GetFeedbackData(MotorData *data);

void M3508_CAN_SendCurrent(float *current);

static void wheelChassis_MotionControl(MotionControl motion, MotorData *data);

void wheelChassis_MotorSpeedControl(float *ExpectSpeed, float *Current_Output, MotorData *data);

static void wheelChassis_Init_PID_Params(void);

#define SPEED_DEADZONE 0.05f // 速度死区，实际可根据需求调整

/**
 * @brief 轮式底盘模块总初始化函数。
 *
 * 功能：
 * - 配置并启动CAN通信，包括滤波器和中断。
 * - 调用内部初始化函数，设置所有PID控制器参数和底盘物理参数（如加减速、半径等）。
 * - 将底盘初始状态设置为等待（Wait）。
 *
 * 调用时机：
 * - 在主程序（如main.c）的初始化阶段，所有依赖的硬件（如CAN1）初始化之后，主循环开始之前调用一次。
 *
 * 注意事项：
 * - 这是一个必须调用的函数，它确保了底盘模块所有功能的正常运行前提。
 * - 依赖于CubeMX生成的 `hcan1` 句柄。
 */
void wheelChassis_Init(void)
{
    M3508_CAN_Config();
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING);
    wheelChassis_Init_PID_Params();
}

/**
 * @brief 通过 CAN 总线发送8字节数据。
 *
 * 功能：
 * - 封装标准ID为0x200的CAN报文，发送8字节数据。
 *
 * @param data 指向8字节数据的指针，数据不足8字节需补齐。
 *
 * 注意事项：
 * - 发送前需确保CAN已启动。
 * - 发送失败无返回值，建议结合HAL_CAN_GetError()排查。
 */
void M3508_CAN_SendData(uint8_t *data)
{
    uint32_t TxMailbox;

    CAN_TxHeaderTypeDef TxHeader;
    TxHeader.StdId = 0x200;
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 0x08;
    TxHeader.TransmitGlobalTime = DISABLE;

    HAL_CAN_AddTxMessage(&hcan1, &TxHeader, data, &TxMailbox);
}

/**
 * @brief 配置 CAN 滤波器，仅接收0x201~0x204。
 *
 * 功能：
 * - 设置CAN为掩码模式，只允许0x201~0x204报文进入FIFO0。
 *
 * 注意事项：
 * - 需在CAN启动前调用。
 * - 多滤波器场景下需合理分配FilterBank。
 */
void M3508_CAN_Config(void)
{
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0; // 使用滤波器组0
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; //错误：原来这里是CAN_FILTER_FIFO
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}


/**
 * @brief CAN FIFO0消息挂起回调。
 *
 * 功能：
 * - 接收并解析0x201~0x204反馈报文，存入全局反馈缓冲区。
 *
 * @param hcan CAN句柄指针。
 *
 * 注意事项：
 * - 仅处理0x201~0x204，其他ID自动丢弃。
 * - 需在HAL库中注册为回调。
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    uint8_t Rxdata[8];

    CAN_RxHeaderTypeDef RxHeader;

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, Rxdata);

    switch (RxHeader.StdId)
    {
        case 0x201:
            for (int i = 0; i < 8; i++)
            {
                G_FeedbackData[0][i] = Rxdata[i];
            }
            break;
        case 0x202:
            for (int i = 0; i < 8; i++)
            {
                G_FeedbackData[1][i] = Rxdata[i];
            }
            break;
        case 0x203:
            for (int i = 0; i < 8; i++)
            {
                G_FeedbackData[2][i] = Rxdata[i];
            }
            break;
        case 0x204:
            for (int i = 0; i < 8; i++)
            {
                G_FeedbackData[3][i] = Rxdata[i];
            }
            break;
        default:
        {
            break;
        }
    }
}

/**
 * @brief 解析反馈数据，填充MotorData数组。
 *
 * 功能：
 * - 将全局反馈缓冲区的原始字节数据，解析为角度（度）、速度（r/s）、电流（A）、温度（℃）。
 *
 * @param data MotorData数组，长度至少为4。
 *
 * 注意事项：
 * - 需保证反馈缓冲区已被最新报文填充。
 * - 角度范围0~360，速度单位r/s。
 */
void M3508_GetFeedbackData(MotorData *data)
{
    for (int i = 0; i < 4; i++)
    {
        data[i].angle = ((G_FeedbackData[i][0] << 8) | G_FeedbackData[i][1]) * 360.0f / 8191.0f;
        data[i].speed = (int16_t) ((G_FeedbackData[i][2] << 8) | G_FeedbackData[i][3]) / 60.0f / (3591.0f / 187.0f);
        // 单位：r/s
        data[i].TorqueCurrent = (int16_t) ((G_FeedbackData[i][4] << 8) | G_FeedbackData[i][5]) * 20.0f / 16384.0f;
        data[i].temperature = (int8_t) G_FeedbackData[i][6];
    }
}

/**
 * @brief 发送4路电机电流指令。
 *
 * 功能：
 * - 将4路电流（A）转换为16位整数，打包为8字节，通过CAN发送。
 *
 * @param current 4路电流数组，单位A。
 *
 * 注意事项：
 * - 电流范围建议-20A~+20A，超出可能溢出。
 * - 需确保CAN已启动。
 */
void M3508_CAN_SendCurrent(float *current)
{
    uint8_t data[8];
    int16_t tmp[4];
    for (int i = 0; i < 4; i++)
    {
        tmp[i] = current[i] * 16384 / 20;
    }
    int j = 0;
    for (int i = 0; i < 8; i += 2)
    {
        data[i] = tmp[i - j] >> 8;
        data[i + 1] = tmp[i - j];
        j++;
    }
    M3508_CAN_SendData(data);
}

void wheelChassis_Init_PID_Params(void)
{
    // 速度环PID参数
    G_Speed_PID_Params.Kp = 1.9f;
    G_Speed_PID_Params.Ki = 0.01f;
    G_Speed_PID_Params.Kd = 0.0f;
    G_Speed_PID_Params.IntegralLimit = 100.0f;
    G_Speed_PID_Params.OutputLimit = 10.0f; // 最大输出5A电流

    // 差速补偿PID参数
    G_DifSpeed_PID_Params.Kp = 0.03f;
    G_DifSpeed_PID_Params.Ki = 0.01f;
    G_DifSpeed_PID_Params.Kd = 0.0f;
    G_DifSpeed_PID_Params.IntegralLimit = 10.0f;
    G_DifSpeed_PID_Params.OutputLimit = 5.0f; // 补偿量限幅
}

/**
 * @brief 速度环PID控制，输出4路电机控制量。
 *
 * 功能：
 * - 对每个电机进行PI(D)速度闭环，输出电流指令。
 * - 内含积分限幅，防止积分饱和。
 *
 * @param ExpectSpeed 期望速度数组（r/s）。
 * @param Current_Output 输出控制量数组（A）。
 * @param data MotorData数组。
 *
 * 注意事项：
 * - 需先更新MotorData的实际速度。
 * - PID参数需根据实际调试。
 */
void wheelChassis_MotorSpeedControl(float *ExpectSpeed, float *Current_Output, MotorData *data)
{
    static float Integral[4];
    static float Error[4], Errorlast[4];
    float Kp = G_Speed_PID_Params.Kp;
    float Ki = G_Speed_PID_Params.Ki;
    float Kd = G_Speed_PID_Params.Kd;
    float IntegralLimit = G_Speed_PID_Params.IntegralLimit;
    float OutputLimit = G_Speed_PID_Params.OutputLimit;

    for (int i = 0; i < 4; i++)
    {
        Error[i] = ExpectSpeed[i] - data[i].speed;

        // 死区处理：期望速度接近0时，清零积分和误差
        if (fabsf(ExpectSpeed[i]) < SPEED_DEADZONE)
        {
            Integral[i] = 0;
            Error[i] = 0;
            Errorlast[i] = 0;
        }

        // 更新积分项
        Integral[i] += Error[i];
        // 积分限幅
        if (Integral[i] > IntegralLimit)
        {
            Integral[i] = IntegralLimit;
        }
        if (Integral[i] < -IntegralLimit)
        {
            Integral[i] = -IntegralLimit;
        }

        // 计算PID输出
        Current_Output[i] = Kp * Error[i] + Ki * Integral[i] + Kd * (Error[i] - Errorlast[i]);

        // 输出限幅
        if (Current_Output[i] > OutputLimit)
        {
            Current_Output[i] = OutputLimit;
        }
        if (Current_Output[i] < -OutputLimit)
        {
            Current_Output[i] = -OutputLimit;
        }

        Errorlast[i] = Error[i];
    }
}

float wheelChassis_AbsoluteValue(float num)
{
    if (num < 0)
    {
        return -num;
    }
    return num;
}

//对遥控器输入的数据进行平滑、限幅并赋值给G_Motion
//在wheelChassis_MotionControl()前调用
void wheelChassis_RemoveControl(ProcessedChannelValue value, MotionControl *motion)
{
    motion->Vy = 0.025f * value.Ry;
    motion->Vx = 0.025f * value.Rx;
    motion->Vw = 0.015f * value.Lx;
}

void wheelChassis_MotorSpeedDifferenceOffset(MotionControl motion, MotorData *data, float *output)
{
    static float error[2];
    static float errorintegral[2];
    static float errorlast[2];
    float Kp = G_DifSpeed_PID_Params.Kp;
    float Ki = G_DifSpeed_PID_Params.Ki;
    float Kd = G_DifSpeed_PID_Params.Kd;
    float IntegralLimit = G_DifSpeed_PID_Params.IntegralLimit;
    float OutputLimit = G_DifSpeed_PID_Params.OutputLimit;
    const float DIFF_DEADZONE = 0.05f; // 死区阈值，可根据实际调整

    // 对角1（电机1和3）
    error[0] = data[0].speed + data[2].speed + 2 * motion.Vw;
    // 死区处理，静止时清零积分
    if (fabsf(error[0]) < DIFF_DEADZONE)
    {
        errorintegral[0] = 0;
        errorlast[0] = 0;
        error[0] = 0;
    }
    errorintegral[0] += error[0];
    // 积分限幅
    if (errorintegral[0] > IntegralLimit) errorintegral[0] = IntegralLimit;
    if (errorintegral[0] < -IntegralLimit) errorintegral[0] = -IntegralLimit;
    // PID输出
    output[0] = Kp * error[0] + Ki * errorintegral[0] + Kd * (error[0] - errorlast[0]);
    // 输出限幅
    if (output[0] > OutputLimit) output[0] = OutputLimit;
    if (output[0] < -OutputLimit) output[0] = -OutputLimit;
    errorlast[0] = error[0];

    // 对角2（电机2和4）
    error[1] = data[1].speed + data[3].speed + 2 * motion.Vw;
    if (fabsf(error[1]) < DIFF_DEADZONE)
    {
        errorintegral[1] = 0;
        errorlast[1] = 0;
        error[1] = 0;
    }
    errorintegral[1] += error[1];
    if (errorintegral[1] > IntegralLimit) errorintegral[1] = IntegralLimit;
    if (errorintegral[1] < -IntegralLimit) errorintegral[1] = -IntegralLimit;
    output[1] = Kp * error[1] + Ki * errorintegral[1] + Kd * (error[1] - errorlast[1]);
    if (output[1] > OutputLimit) output[1] = OutputLimit;
    if (output[1] < -OutputLimit) output[1] = -OutputLimit;
    errorlast[1] = error[1];
}

void wheelChassis_MotionControl(MotionControl motion, MotorData *data)
{
    float ExpectSpeed[4];
    float Current_Output[4];
    float offset[2];

    ExpectSpeed[0] = -motion.Vy - motion.Vx - motion.Vw;
    ExpectSpeed[1] = +motion.Vy - motion.Vx - motion.Vw;
    ExpectSpeed[2] = +motion.Vy + motion.Vx - motion.Vw;
    ExpectSpeed[3] = -motion.Vy + motion.Vx - motion.Vw;

    // wheelChassis_MotorSpeedDifferenceOffset(motion,data,offset);
    //
    // ExpectSpeed[0] -= offset[0];
    // ExpectSpeed[2] -= offset[0];
    // ExpectSpeed[1] -= offset[1];
    // ExpectSpeed[3] -= offset[1];

    wheelChassis_MotorSpeedControl(ExpectSpeed, Current_Output, data);

    M3508_CAN_SendCurrent(Current_Output);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6)
    {
        MotorData data[4];
        M3508_GetFeedbackData(data);
        wheelChassis_RemoveControl(G_Value, &G_Motion);
        wheelChassis_MotionControl(G_Motion, data);
    }
}
