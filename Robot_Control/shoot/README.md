# ROBOCON 摩擦带发射装置控制系统

## 项目简介
本项目为 ROBOCON 机器人比赛设计的摩擦带发射装置控制系统，基于 STM32F427 微控制器，采用 M5065 无刷电机进行闭环速度控制。系统通过 PWM 波形直接驱动电机，利用电调（ESC）M口信号输入捕获测得电机转速，并结合 PID 算法实现高精度、高响应的摩擦轮发射速度调节，适用于高要求的射球/发射任务。

## 主要功能
- **M5065 电机闭环控制**：基于电调 M 口测速反馈，实时 PID 调速，确保摩擦轮转速精准。
- **PWM 调速**：通过定时器 PWM 输出直接控制无刷电机驱动。
- **M口测速反馈**：利用定时器输入捕获获取电调 M 口输出的转速信号，实现闭环。
- **串口调试**：支持 UART 数据上报与参数调试。
- **多电机支持**：可同时控制多组摩擦轮。

## 软硬件环境
- **主控芯片**：STM32F427IIHx
- **电机型号**：M5065 无刷电机（不带编码器）
- **驱动板**：支持 PWM 信号输入的无刷电机电调（ESC），并带 M 口测速输出
- **通信接口**：UART（用于调试和数据上报）
- **开发环境**：
  - Keil MDK-ARM v5
  - STM32CubeMX
  - ARM Compiler v6
  - ST-Link 调试器

## 代码结构
```
shoot-main/
├── Core/                # STM32 HAL 及外设驱动
│   ├── Src/             # 主程序、外设驱动实现
│   └── Inc/             # 头文件
├── Drivers/             # 底层库（CMSIS、HAL、DSP等）
├── MDK-ARM/             # Keil 工程文件、启动文件
├── User/                # 用户自定义代码（PWM、UART、PID等）
│   ├── drv_uart.c/h     # UART 驱动
│   ├── pid.c/h          # PID 控制算法
│   └── ...
├── test_feedback.ioc    # STM32CubeMX 工程配置
└── README.md            # 项目说明
```

## 快速上手
1. **克隆项目**
   ```bash
   git clone https://github.com/Melody-ovo-a/shoot.git
   cd shoot-main
   ```
2. **使用 Keil MDK-ARM 打开工程**
   - 打开 `MDK-ARM/test_feedback.uvprojx`
   - 编译（F7）
   - 连接 ST-Link，下载程序到 STM32F427
3. **硬件连接**
   - M5065 电机通过电调（ESC）连接，PWM 信号由 STM32 TIM5 输出
   - 电调 M 口信号接入 STM32 定时器输入捕获通道，用于测速
   - UART 用于调试与数据上报

## 主要控制流程
1. **系统初始化**：初始化 GPIO、DMA、UART、定时器、PWM、PID 参数等。
2. **主循环**：
   - 读取电调 M 口信号频率，计算摩擦轮当前转速
   - 以 1200RPM（可调）为目标，调用 PID 算法计算输出
   - 通过 PWM 调节电机驱动，实现闭环速度控制
   - 通过 UART 上报状态，便于调试
3. **中断与回调**：
   - 定时器输入捕获中断用于 M 口测速
   - 支持多电机并行闭环控制

## 典型代码片段
```c
// 主循环核心逻辑
Tx_1 = freq1 / 48 * 60; // M口测速
output_pid[0] = PID_calc(&Motor_pid[0], Tx_1, 1200); // PID 调速
__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, Motor_pid[0].OUT); // PWM 输出
```

## 贡献与支持
- 欢迎提交 Issue 和 Pull Request 以优化本项目。
- 如需定制或技术支持，请通过 GitHub Issues 联系。

## 许可证
本项目采用 MIT 许可证。 