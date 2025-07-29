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
Class_Motor_C610 Ball_Collection_motor[2];

float Target_Angle, Now_Angle, Target_Omega, Now_Omega;

uint32_t Counter = 0;

static char Variable_Assignment_List[][SERIALPLOT_RX_VARIABLE_ASSIGNMENT_MAX_LENGTH] = {
    //收球电机调PID
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
        case (0x206):
        {
            Ball_Collection_motor[0].CAN_RxCpltCallback(Rx_Buffer->Data);
        }
        break;
        case (0x207):
        {
            Ball_Collection_motor[1].CAN_RxCpltCallback(Rx_Buffer->Data);
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
        // 收球电机调PID (只调节第一个电机，第二个电机使用相同参数)
        case(0):
        {
            Ball_Collection_motor[0].PID_Angle.Set_K_P(serialplot.Get_Variable_Value());
        }
        break;
        case(1):
        {
            Ball_Collection_motor[0].PID_Angle.Set_K_I(serialplot.Get_Variable_Value());
        }
        break;
        case(2):
        {
            Ball_Collection_motor[0].PID_Angle.Set_K_D(serialplot.Get_Variable_Value());
        }
        break;
        case(3):
        {
            Ball_Collection_motor[0].PID_Omega.Set_K_P(serialplot.Get_Variable_Value());
        }
        break;
        case(4):
        {
            Ball_Collection_motor[0].PID_Omega.Set_K_I(serialplot.Get_Variable_Value());
        }
        break;
        case(5):
        {
            Ball_Collection_motor[0].PID_Omega.Set_K_D(serialplot.Get_Variable_Value());
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

    // 循环初始化两个收球电机
    for(int i = 0; i < 2; i++)
    {
        Ball_Collection_motor[i].PID_Angle.Init(3.50f, 0.08f, 0.0f, 0.0f, 25.0f * PI, 25.0f * PI);
        Ball_Collection_motor[i].PID_Omega.Init(40.0f, 4.50f, 0.0f, 0.0f, 3000.0f, 3000.0f);
    }
    
    // 分别初始化两个收球电机的CAN ID
    Ball_Collection_motor[0].Init(&hcan1, CAN_Motor_ID_0x206, Control_Method_ANGLE);
    Ball_Collection_motor[1].Init(&hcan1, CAN_Motor_ID_0x207, Control_Method_ANGLE);
    
    // 循环设置目标位置 (收球阶段：转动5圈)
    for(int i = 0; i < 2; i++)
    {
        Ball_Collection_motor[i].Set_Target_Angle(-10.0f * PI);
    }
    int Collect_stage = 0; // 0: 收球阶段, 1: 完成阶段
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        //如果计时到4000ms并且还在收球阶段，就回到0位置
        Counter++;
        if(Counter >= 4000 && Collect_stage == 0)
        {
            Collect_stage = 1;
            Counter = 0;
            // 循环设置两个电机回到初始位置 (收球完成后归位)
            for(int i = 0; i < 2; i++)
            {
                Ball_Collection_motor[i].Set_Target_Angle(0.0f);
            }
        }

        //串口绘图显示内容 (显示第一个收球电机的数据)
        Target_Angle = Ball_Collection_motor[0].Get_Target_Angle();
        Now_Angle = Ball_Collection_motor[0].Get_Now_Angle();
        Target_Omega = Ball_Collection_motor[0].Get_Target_Omega();
        Now_Omega = Ball_Collection_motor[0].Get_Now_Omega();
        serialplot.Set_Data(4, &Target_Angle, &Now_Angle, &Target_Omega, &Now_Omega);
        serialplot.TIM_Write_PeriodElapsedCallback();

        //输出数据到收球电机
        for(int i = 0; i < 2; i++)
        {
            Ball_Collection_motor[i].TIM_PID_PeriodElapsedCallback();
        }

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
