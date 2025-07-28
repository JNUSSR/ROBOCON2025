/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "drv_bsp.h"

#include "dvc_serialplot.h"
#include "dvc_motor.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

Class_Serialplot serialplot;
Class_Motor_C620 motor;

float Target_Angle, Now_Angle, Target_Omega, Now_Omega;

uint32_t Counter = 0;

static char Variable_Assignment_List[][SERIALPLOT_RX_VARIABLE_ASSIGNMENT_MAX_LENGTH] = {
    //电机调PID
    "pa",
    "ia",
    "da",
    "po",
    "io",
    "do",
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief CAN报文回调函数
 *
 * @param Rx_Buffer CAN接收的信息结构体
 */
void CAN_Motor_Call_Back(Struct_CAN_Rx_Buffer *Rx_Buffer)
{
    switch (Rx_Buffer->Header.StdId)
    {
<<<<<<< HEAD
        case (0x201):
=======
        case (0x205):
>>>>>>> 1edc67784f2830cb14053f9af588562884439518
        {
            motor.CAN_RxCpltCallback(Rx_Buffer->Data);
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
        // 电机调PID
        case(0):
        {
            motor.PID_Angle.Set_K_P(serialplot.Get_Variable_Value());
        }
        break;
        case(1):
        {
            motor.PID_Angle.Set_K_I(serialplot.Get_Variable_Value());
        }
        break;
        case(2):
        {
            motor.PID_Angle.Set_K_D(serialplot.Get_Variable_Value());
        }
        break;
        case(3):
        {
            motor.PID_Omega.Set_K_P(serialplot.Get_Variable_Value());
        }
        break;
        case(4):
        {
            motor.PID_Omega.Set_K_I(serialplot.Get_Variable_Value());
        }
        break;
        case(5):
        {
            motor.PID_Omega.Set_K_D(serialplot.Get_Variable_Value());
        }
        break;
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_CAN1_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */

    BSP_Init(BSP_DC24_LU_ON | BSP_DC24_LD_ON | BSP_DC24_RU_ON | BSP_DC24_RD_ON);
    CAN_Init(&hcan1, CAN_Motor_Call_Back);
    UART_Init(&huart2, UART_Serialplot_Call_Back, SERIALPLOT_RX_VARIABLE_ASSIGNMENT_MAX_LENGTH);

    serialplot.Init(&huart2, 6, (char **)Variable_Assignment_List);

<<<<<<< HEAD
    motor.PID_Angle.Init(15.0f, 1.25f, 0.0f, 0.0f, 15.0f * PI, 15.0f * PI);
    motor.PID_Omega.Init(180.0f, 65.0f, 0.0f, 0.0f, 2500.0f, 2500.0f);
    motor.Init(&hcan1, CAN_Motor_ID_0x201, Control_Method_ANGLE);
    
    // 启动时立即设置目标位置为-3π（送球位置）
    motor.Set_Target_Angle(-3.0f * PI);
    int motion_stage = 0; // 0: 送球阶段, 1: 完成阶段
=======
    motor.Init(&hcan1, CAN_Motor_ID_0x205, Control_Method_ANGLE);
    
    // 送球运动状态标志位：0=送球阶段, 1=回到初始位置阶段
    // 此处默认为送球阶段，可以修改为1以回到初始位置阶段
    int motion_stage = 0; 
    
    // 初始化送球阶段的PID参数（较大的参数用于快速送球动作）
    // 角度环：P=18.0, I=1.25 (用于精确位置控制)
    // 速度环：P=200.0, I=65.0 (用于快速响应)
    motor.PID_Angle.Init(18.0f, 1.25f, 0.0f, 0.0f, 15.0f * PI, 15.0f * PI);
    motor.PID_Omega.Init(200.0f, 65.0f, 0.0f, 0.0f, 2500.0f, 2500.0f);

>>>>>>> 1edc67784f2830cb14053f9af588562884439518
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
<<<<<<< HEAD
    {
        //如果计时到2000ms并且还在送球阶段，就回到0位置
        Counter++;
        if(Counter >= 2000 && motion_stage == 0)
        {
            motion_stage = 1;
            Counter = 0;
            motor.Set_Target_Angle(0.0f); // 回到初始位置
        }
=======
    {   
        // ========== 送球阶段 (motion_stage == 0) ==========
        if (motion_stage == 0)
        {
            // 设置送球阶段的PID参数（高增益，用于快速准确的送球动作）
            motor.PID_Angle.Set_K_P(18.0f);   // 角度环比例增益
            motor.PID_Angle.Set_K_I(1.25f);   // 角度环积分增益
            motor.PID_Omega.Set_K_P(200.0f);  // 速度环比例增益
            motor.PID_Omega.Set_K_I(65.0f);   // 速度环积分增益
            motor.Set_Target_Angle(-3.0f * PI); // 设置送球目标位置(-3π弧度)

            // 送球阶段计时器，每个循环+1 (约1ms/次)
            Counter++;
            if(Counter >= 1800)  // 1.8秒后切换到回收阶段
            {
                Counter = 0;          // 重置计时器
                motion_stage = 1;     // 切换到回到初始位置阶段
            }
        }
        
        // ========== 回到初始位置阶段 (motion_stage == 1) ==========
        if (motion_stage == 1)
        {
            // 设置回收阶段的PID参数（较低增益，用于平稳回到初始位置）
            motor.PID_Angle.Set_K_P(10.0f);   // 角度环比例增益（降低）
            motor.PID_Angle.Set_K_I(0.8f);    // 角度环积分增益（降低）
            motor.PID_Omega.Set_K_P(110.0f);  // 速度环比例增益（降低）
            motor.PID_Omega.Set_K_I(45.0f);   // 速度环积分增益（降低）
            motor.Set_Target_Angle(0.0f);     // 设置回收目标位置(0弧度，初始位置)
            
            // 注意：当前版本在此阶段会持续运行，没有自动切换回送球阶段
            // 如需循环执行，可在此处添加计时和状态切换逻辑
        }
        
>>>>>>> 1edc67784f2830cb14053f9af588562884439518

        //串口绘图显示内容

        Target_Angle = motor.Get_Target_Angle();
        Now_Angle = motor.Get_Now_Angle();
        Target_Omega = motor.Get_Target_Omega();
        Now_Omega = motor.Get_Now_Omega();
        serialplot.Set_Data(4, &Target_Angle, &Now_Angle, &Target_Omega, &Now_Omega);
        serialplot.TIM_Write_PeriodElapsedCallback();

        //输出数据到电机
        motor.TIM_PID_PeriodElapsedCallback();

        //通信设备回调数据
        TIM_CAN_PeriodElapsedCallback();
        TIM_UART_PeriodElapsedCallback();

        //延时1ms
        HAL_Delay(0);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Activate the Over-Drive mode
     */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
