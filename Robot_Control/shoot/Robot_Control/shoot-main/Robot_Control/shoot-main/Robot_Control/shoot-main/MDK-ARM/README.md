# STM32F427 射击反馈系统

## 项目简介

这是一个基于STM32F427微控制器的射击反馈系统项目，使用STM32CubeMX生成基础代码，Keil MDK-ARM作为开发环境。项目实现了CAN通信、UART通信、PID控制等功能模块。

## 功能特性

- **CAN通信模块**: 实现CAN总线通信功能
- **UART通信**: 串口通信和调试输出
- **PID控制算法**: 实现精确的PID控制
- **DMA传输**: 高效的数据传输
- **GPIO控制**: 通用输入输出控制
- **定时器功能**: 精确的定时控制

## 硬件要求

- STM32F427IIHx微控制器
- CAN收发器
- UART转USB模块
- 相关传感器和执行器

## 开发环境

- **IDE**: Keil MDK-ARM v5
- **代码生成**: STM32CubeMX
- **编译器**: ARM Compiler v6
- **目标芯片**: STM32F427IIHx
- **调试器**: ST-Link

## 项目结构

```
shoot/
├── Core/                    # 核心代码
│   ├── Src/               # 源文件
│   │   ├── main.c         # 主程序
│   │   ├── can.c          # CAN通信
│   │   ├── uart.c         # 串口通信
│   │   ├── gpio.c         # GPIO控制
│   │   ├── dma.c          # DMA传输
│   │   ├── tim.c          # 定时器
│   │   └── ...
│   └── Inc/               # 头文件
│       ├── main.h
│       ├── can.h
│       ├── uart.h
│       └── ...
├── Drivers/                # STM32 HAL驱动
│   ├── STM32F4xx_HAL_Driver/
│   └── CMSIS/
├── MDK-ARM/               # Keil项目文件
│   ├── test_feedback.uvprojx
│   └── ...
├── User/                   # 用户自定义代码
├── test_feedback.ioc      # CubeMX配置文件
└── README.md              # 项目说明
```

## 编译和烧录

### 方法1: 使用Keil MDK-ARM
1. 打开 `MDK-ARM/test_feedback.uvprojx` 项目文件
2. 编译项目 (F7)
3. 连接ST-Link调试器
4. 烧录程序到STM32F427

### 方法2: 使用STM32CubeIDE
1. 导入项目到STM32CubeIDE
2. 编译项目
3. 烧录程序

## 通信协议

### CAN通信
- 波特率: 500kbps
- 数据格式: 标准帧
- 功能: 设备间通信

### UART通信
- 波特率: 115200
- 数据位: 8
- 停止位: 1
- 功能: 调试输出和参数配置

## 代码说明

### 主要源文件
- `main.c`: 主程序入口，系统初始化和主循环
- `can.c`: CAN通信模块，处理CAN消息收发
- `uart.c`: 串口通信模块，调试输出和参数配置
- `gpio.c`: GPIO控制模块，引脚配置和控制
- `dma.c`: DMA传输模块，高效数据传输
- `tim.c`: 定时器模块，精确定时控制

### 配置参数
主要配置参数位于各模块的头文件中，可根据实际需求调整。

## 克隆和编译

```bash
# 克隆项目
git clone https://github.com/Melody-ovo-a/shoot.git
cd shoot

# 使用Keil MDK-ARM打开项目
# 打开 MDK-ARM/test_feedback.uvprojx
# 编译并烧录
```

## 版本历史

- v1.0.0: 初始版本，实现基础功能

## 贡献指南

欢迎提交Issue和Pull Request来改进项目。

## 许可证

本项目采用MIT许可证。

## 联系方式

如有问题，请通过GitHub Issues联系。