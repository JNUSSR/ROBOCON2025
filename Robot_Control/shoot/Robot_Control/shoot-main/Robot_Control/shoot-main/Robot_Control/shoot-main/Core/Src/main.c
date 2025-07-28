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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "dvc_serialplot.h"

#include "pid.h"
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

float Tx_1, Tx_2, Tx_3, Tx_4;
float output_pid[3];
uint32_t freq1,freq2,freq3;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;

extern PID_typedef 			Motor_pid[14];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



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
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */
   PID_total_init(); // PID参数初始化

  // 初始化并启动PWM输出（用于电机控制）
  HAL_TIM_PWM_Init(&htim5);
  Uart_Init(&huart2, NULL, 0, NULL);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);

  // 启动输入捕获中断（用于测速）
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // 电机1测速
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1); // 电机2测速
  HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1); // 电机3测速
  
  // 串口初始化，用于数据上报
  serialplot.Init(&huart2, 3, NULL);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, 840);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 840);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, 840);
    HAL_Delay(6000); // 等待1秒，确保系统稳定
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //计算三个电机的转速（单位：RPM）
    //freqX为输入捕获得到的频率，48为编码器每转脉冲数，*60为每分钟
    Tx_1 = freq1 / 48 * 60;
    Tx_2 = freq2 / 48 * 60;
    Tx_3 = freq3 / 48 * 60;
    // if (Tx_1 > 800) Tx_1 = 800; // 限制电机1转速
    // if (Tx_2 > 600) Tx_2 = 600; // 限制电机2转速
    // if (Tx_3 > 600) Tx_3 = 600; // 限制电机3转速

    // PID闭环调速，目标转速为1200RPM
    output_pid[0] = PID_calc(&Motor_pid[0], Tx_1, 1200);
    output_pid[1] = PID_calc(&Motor_pid[1], Tx_2, 1200);
    output_pid[2] = PID_calc(&Motor_pid[2], Tx_3, 1200);

    // 限幅，防止PWM输出超出范围
    if (output_pid[0] < 0) output_pid[0] = 0;
    if (output_pid[0] > __HAL_TIM_GET_AUTORELOAD(&htim5)) output_pid[0] = __HAL_TIM_GET_AUTORELOAD(&htim5);
    if (output_pid[1] < 0) output_pid[1] = 0;
    if (output_pid[1] > __HAL_TIM_GET_AUTORELOAD(&htim5)) output_pid[1] = __HAL_TIM_GET_AUTORELOAD(&htim5);
    if (output_pid[2] < 0) output_pid[2] = 0;
    if (output_pid[2] > __HAL_TIM_GET_AUTORELOAD(&htim5)) output_pid[2] = __HAL_TIM_GET_AUTORELOAD(&htim5);

    // 设置PWM占空比，实现对电机的速度控制
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, Motor_pid[0].OUT);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, Motor_pid[1].OUT);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, Motor_pid[2].OUT);

    //发送数据到上位机，便于调试和监控
        serialplot.Set_Data(6, &Tx_1, &output_pid[0], &Tx_2, &output_pid[1],&Tx_3, &output_pid[3]);
        serialplot.TIM_Add_PeriodElapsedCallback();
        TIM_UART_PeriodElapsedCallback();

    // 延时10ms，控制主循环频率
    HAL_Delay(10);

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
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
/**
  * @brief 定时器输入捕获中断回调函数
  * @param htim: 定时器句柄
  * @note  用于捕获电调（ESC）M口输出信号周期，计算电机转速。TIM2、TIM4、TIM8的CH1分别对应电机1、2、3的测速。
  */
uint32_t current_capture = 0; // 当前捕获值
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) // TIM2用于电机1测速（M口->TIM2_CH1）
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
               uint32_t before_capture=current_capture;
                // 读取当前捕获值 
               uint32_t current_capture=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
                if (current_capture < before_capture) {
                    current_capture += 65535; // 处理溢出
                }
           freq1=1000000/(current_capture-before_capture); // 1MHz定时器时基
        }
    }
    else if (htim->Instance == TIM4) // TIM4用于电机2测速（M口->TIM4_CH1）
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {

            uint32_t before_capture=current_capture;
              uint32_t current_capture=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
                if (current_capture < before_capture) {
                    current_capture += 65535; // 处理溢出
                }
           freq2=1000000/(current_capture-before_capture); // 1MHz定时器时基
        }
    }
    else if (htim->Instance == TIM8) // TIM8用于电机3测速（M口->TIM8_CH1）
      {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
              {
                  uint32_t before_capture=current_capture;
                    uint32_t current_capture=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
                      if (current_capture < before_capture) {
                          current_capture += 65535; // 处理溢出
                      }
                  freq3=1000000/(current_capture-before_capture); // 1MHz定时器时基
              }
      }
}
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

#ifdef  USE_FULL_ASSERT
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
