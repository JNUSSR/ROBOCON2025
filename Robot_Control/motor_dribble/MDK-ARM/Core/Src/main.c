/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <math.h>
#include <stdbool.h>

#include "drv_bsp.h"

#include "dvc_serialplot.h"
#include "dvc_motor.h"

// 添加PI常数定义
#ifndef PI
#define PI 3.14159265358979323846f
#endif

/* USER CODE END Includes */

/* USER CODE BEGIN PV */

Class_Serialplot serialplot;
Class_Motor_C610 motor1;  // 电调ID: 0x206
Class_Motor_C610 motor2;  // 电调ID: 0x207

float Target_Angle, Now_Angle, Target_Omega, Now_Omega;
float Target_Angle2, Now_Angle2, Target_Omega2, Now_Omega2;  // motor2的角度和速度变量

uint32_t Counter = 0;

// 精准角度控制相关变量
typedef enum {
    ANGLE_STATE_POSITION_A = 0,  // 位置A：0度
    ANGLE_STATE_POSITION_B = 1   // 位置B：180度
} Enum_Angle_State;

typedef struct {
    Enum_Angle_State current_state;     // 当前角度状态
    float position_a_angle;             // 位置A的目标角度 (rad)
    float position_b_angle;             // 位置B的目标角度 (rad)
    float angle_tolerance;              // 角度到位容差 (rad)
    uint32_t state_switch_counter;      // 状态切换计数器
    uint32_t state_switch_interval;     // 状态切换间隔时间 (ms)
    bool angle_reached;                 // 角度是否到位标志
    bool auto_switch_enable;            // 自动切换使能
} Struct_Angle_Control;

// 初始化角度控制结构体
Struct_Angle_Control angle_control = {
    .current_state = ANGLE_STATE_POSITION_A,
    .position_a_angle = 0.0f,           // 0度
    .position_b_angle = PI,             // 180度 (π弧度)
    .angle_tolerance = 0.05f,           // 容差±0.05弧度 (约±2.86度)
    .state_switch_counter = 0,
    .state_switch_interval = 3000,      // 3秒切换一次
    .angle_reached = false,
    .auto_switch_enable = true
};

static char Variable_Assignment_List[][SERIALPLOT_RX_VARIABLE_ASSIGNMENT_MAX_LENGTH] = {
    //电机调PID
    "pa",
    "ia",
    "da",
    "po",
    "io",
    "do",
    // 电机角度控制
    "cmd",  // 控制命令：0=转到0度，1=转到90度
};

// 电机角度控制变量
float Motor_Control_Command = 0.0f;  // 0=转到0度，1=转到90度

/* USER CODE END PV */

/* USER CODE BEGIN 0 */

/**
 * @brief 设置目标角度到位置A（两个电机同步）
 */
void Set_Target_To_Position_A(void)
{
    motor1.Set_Target_Angle(angle_control.position_a_angle);
    motor2.Set_Target_Angle(angle_control.position_a_angle);  // motor2同步动作
    angle_control.current_state = ANGLE_STATE_POSITION_A;
    angle_control.angle_reached = false;
    angle_control.state_switch_counter = 0;
}

/**
 * @brief 设置目标角度到位置B（两个电机同步）
 */
void Set_Target_To_Position_B(void)
{
    motor1.Set_Target_Angle(angle_control.position_b_angle);
    motor2.Set_Target_Angle(angle_control.position_b_angle);  // motor2同步动作
    angle_control.current_state = ANGLE_STATE_POSITION_B;
    angle_control.angle_reached = false;
    angle_control.state_switch_counter = 0;
}

/**
 * @brief 检查角度是否到位
 * @return true: 角度已到位, false: 角度未到位
 */
bool Check_Angle_Reached(void)
{
    float angle_error = fabs(motor1.Get_Target_Angle() - motor1.Get_Now_Angle());
    
    if (angle_error <= angle_control.angle_tolerance)
    {
        angle_control.angle_reached = true;
        return true;
    }
    else
    {
        angle_control.angle_reached = false;
        return false;
    }
}

/**
 * @brief 精准角度控制任务
 */
void Precision_Angle_Control_Task(void)
{
    // 检查角度是否到位
    Check_Angle_Reached();
    
    // 自动切换逻辑
    if (angle_control.auto_switch_enable && angle_control.angle_reached)
    {
        angle_control.state_switch_counter++;
        
        if (angle_control.state_switch_counter >= angle_control.state_switch_interval)
        {
            // 切换到另一个状态
            if (angle_control.current_state == ANGLE_STATE_POSITION_A)
            {
                Set_Target_To_Position_B();
            }
            else
            {
                Set_Target_To_Position_A();
            }
        }
    }
}

/**
 * @brief CAN报文回调函数
 *
 * @param Rx_Buffer CAN接收的信息结构体
 */
void CAN_Motor_Call_Back(Struct_CAN_Rx_Buffer *Rx_Buffer)
{
    switch (Rx_Buffer->Header.StdId)
    {
        case (0x201):  // 临时添加0x201测试
        case (0x206):
        {
            // 添加调试：翻转LED指示接收到数据
            static uint32_t led_counter = 0;
            led_counter++;
            if (led_counter % 100 == 0) {
                // 每100次接收翻转一次LED，确认接收到数据
                HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_11); // 假设PE11是LED引脚，根据你的硬件调整
            }
            
            motor1.CAN_RxCpltCallback(Rx_Buffer->Data);
        }
        break;
        case (0x207):  // motor2的CAN回调处理
        {
            motor2.CAN_RxCpltCallback(Rx_Buffer->Data);
        }
        break;
        // 添加默认分支来捕获其他ID的消息
        default:
        {
            // 调试：检查是否收到其他ID的消息
            static uint32_t other_id_counter = 0;
            other_id_counter++;
            // 可以在这里设置断点查看Rx_Buffer->Header.StdId的值
        }
        break;
    }
}

/**
 * @brief HAL库UART接收DMA空闲中断
 *
 * @param huart UART编号
 * @param Size 长度
 */
void UART_Serialplot_Call_Back(uint8_t *Buffer, uint16_t Length)
{
    serialplot.UART_RxCpltCallback(Buffer);
    switch (serialplot.Get_Variable_Index())
    {
        // 电机调PID（两个电机同步调整）
        case(0):
        {
            motor1.PID_Angle.Set_K_P(serialplot.Get_Variable_Value());
            motor2.PID_Angle.Set_K_P(serialplot.Get_Variable_Value());  // motor2同步设置
        }
        break;
        case(1):
        {
            motor1.PID_Angle.Set_K_I(serialplot.Get_Variable_Value());
            motor2.PID_Angle.Set_K_I(serialplot.Get_Variable_Value());  // motor2同步设置
        }
        break;
        case(2):
        {
            motor1.PID_Angle.Set_K_D(serialplot.Get_Variable_Value());
            motor2.PID_Angle.Set_K_D(serialplot.Get_Variable_Value());  // motor2同步设置
        }
        break;
        case(3):
        {
            motor1.PID_Omega.Set_K_P(serialplot.Get_Variable_Value());
            motor2.PID_Omega.Set_K_P(serialplot.Get_Variable_Value());  // motor2同步设置
        }
        break;
        case(4):
        {
            motor1.PID_Omega.Set_K_I(serialplot.Get_Variable_Value());
            motor2.PID_Omega.Set_K_I(serialplot.Get_Variable_Value());  // motor2同步设置
        }
        break;
        case(5):
        {
            motor1.PID_Omega.Set_K_D(serialplot.Get_Variable_Value());
            motor2.PID_Omega.Set_K_D(serialplot.Get_Variable_Value());  // motor2同步设置
        }
        break;
        // 电机角度控制命令
        case(6):
        {
            Motor_Control_Command = serialplot.Get_Variable_Value();
            // 根据命令值控制电机角度
            if (Motor_Control_Command < 0.5f) {
                Motor_Rotate_0_Degree();  // 转到0度
            } else {
                Motor_Rotate_90_Degree(); // 转到90度
            }
        }
        break;
    }
}

/* USER CODE END 0 */

  /* USER CODE BEGIN 2 */

    CAN_Init(&hcan1, CAN_Motor_Call_Back);
    UART_Init(&huart2, UART_Serialplot_Call_Back, SERIALPLOT_RX_VARIABLE_ASSIGNMENT_MAX_LENGTH);

    // 更新串口指令数量为7个
    serialplot.Init(&huart2, 7, (char **)Variable_Assignment_List);

    // 优化的PID参数设置 - 针对精准角度控制
    // 角度环PID：较高的P值保证快速响应，适量的I值消除稳态误差，适当的D值减少超调
    motor1.PID_Angle.Init(20.0f, 0.5f, 1.0f, 0.0f, 20.0f * PI, 20.0f * PI);
    // 速度环PID：保持原有参数，已经调试良好
    motor1.PID_Omega.Init(200.0f, 100.0f, 0.0f, 0.0f, 8000.0f, 8000.0f);
    
    // 初始化电机为角度控制模式
    motor1.Init(&hcan1, CAN_Motor_ID_0x206, Control_Method_ANGLE, 1.0f);
    
    // 初始化motor2，使用相同的PID参数
    motor2.PID_Angle.Init(20.0f, 0.5f, 1.0f, 0.0f, 20.0f * PI, 20.0f * PI);
    motor2.PID_Omega.Init(200.0f, 100.0f, 0.0f, 0.0f, 8000.0f, 8000.0f);
    motor2.Init(&hcan1, CAN_Motor_ID_0x207, Control_Method_ANGLE, 1.0f);

    // 设置初始目标角度为位置A
    Set_Target_To_Position_A();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
        // 强制发送测试（用于确认CAN发送功能）
        static uint32_t test_counter = 0;
        test_counter++;
        if (test_counter == 1000) {  // 1秒后发送一次测试数据
            // 直接发送一个小的电流值，看电机是否有反应
            motor1.Set_Out(500.0f);  // 发送500的输出值测试
            motor1.Output();         // 立即输出
            motor2.Set_Out(500.0f);  // motor2同样测试
            motor2.Output();         // 立即输出
        }

        // 精准角度控制任务
        Precision_Angle_Control_Task();
        
		//串口绘图显示内容 - motor1数据
        Target_Angle = motor1.Get_Target_Angle();
        Now_Angle = motor1.Get_Now_Angle();
        Target_Omega = motor1.Get_Target_Omega();
        Now_Omega = motor1.Get_Now_Omega();
        
        //获取motor2数据
        Target_Angle2 = motor2.Get_Target_Angle();
        Now_Angle2 = motor2.Get_Now_Angle();
        Target_Omega2 = motor2.Get_Target_Omega();
        Now_Omega2 = motor2.Get_Now_Omega();
        
        serialplot.Set_Data(5, &Target_Angle, &Now_Angle, &Target_Omega, &Now_Omega, &Motor_Control_Command);
        serialplot.TIM_Write_PeriodElapsedCallback();

        //CAN发送数据给两个电机
        motor1.TIM_PID_PeriodElapsedCallback();
        motor2.TIM_PID_PeriodElapsedCallback();

        //通信设备回调数据
        TIM_CAN_PeriodElapsedCallback();
		
		//UART发送数据给上位机
        TIM_UART_PeriodElapsedCallback();

        //延时1ms
        HAL_Delay(1);

    /* USER CODE END WHILE */ 

/**
 * @brief 控制电机回到0度位置
 * @param None
 * @retval None
 */
void Motor_Control_Zero_Position(void)
{
    motor1.Set_Target_Angle(0.0f);
}

/**
 * @brief 控制电机旋转7圈（14π弧度）
 * @param None
 * @retval None
 */
void Motor_Control_Seven_Rotations(void)
{
    motor1.Set_Target_Angle(14.0f * PI);
}

/**
 * @brief 控制电机相对当前位置旋转指定角度
 * @param relative_angle_deg 相对角度（度），正值为顺时针，负值为逆时针
 * @note 考虑减速比1:36，输出轴90度对应电机轴3240度
 */
void Motor_Rotate_Relative_Angle(float relative_angle_deg) {
  // 获取当前角度
  float current_angle = motor1.Get_Now_Angle();
  
  // 计算目标角度（考虑减速比1:36）
  // 输出轴角度 * 36 = 电机轴角度
  float motor_shaft_angle_rad = (relative_angle_deg * 36.0f) * (PI / 180.0f);
  float target_angle = current_angle + motor_shaft_angle_rad;
  
  // 设置目标角度
  motor1.Set_Target_Angle(target_angle);
}

/**
 * @brief 检查电机是否到达目标位置
 * @param tolerance_deg 允许的角度误差（度），默认建议1-2度
 * @return true: 已到达目标位置, false: 未到达目标位置
 */
bool Motor_Is_At_Target_Position(float tolerance_deg) {
  // 获取当前角度和目标角度
  float current_angle = motor1.Get_Now_Angle();
  float target_angle = motor1.Get_Target_Angle();
  
  // 计算角度误差
  float angle_error_rad = fabs(target_angle - current_angle);
  
  // 转换容差为弧度
  float tolerance_rad = tolerance_deg * (PI / 180.0f);
  
  // 判断是否在容差范围内
  return (angle_error_rad <= tolerance_rad);
}

/**
 * @brief 控制两个电机同步旋转90°
 * @param 
 */
void Motor_Rotate_90_Degree() {
  // 设置motor1目标角度
  motor1.Set_Target_Angle(18.0f * PI);
  // 设置motor2目标角度（同步动作）
  motor2.Set_Target_Angle(18.0f * PI);
}

/**
 * @brief 控制两个电机同步回到0度
 * @param 
 */
void Motor_Rotate_0_Degree() {
  // 设置motor1目标角度
  motor1.Set_Target_Angle(0.0f);
  // 设置motor2目标角度（同步动作）
  motor2.Set_Target_Angle(0.0f);
}

  /* USER CODE END WHILE */ 