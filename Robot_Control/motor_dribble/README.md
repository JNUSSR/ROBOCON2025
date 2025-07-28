# STM32F427 电机控制项目 (motor_dribble)

## 项目概述

本项目基于大疆A板STM32F427IIHx微控制器，实现了多电机协调控制系统，主要用于机械臂抓取和释放球类物体的控制。项目集成了CAN总线电机控制、气缸控制、串口通信调试等功能。

## 硬件配置

- **主控制器**: 大疆A板 STM32F427IIHx
- **电机**: 
  - 2个C610电机控制器 + 2006电机 (ID: 0x206, 0x207)
  - 1个C620电机控制器 + 3508电机 (ID: 0x205)
- **通信接口**: 
  - CAN1总线用于电机通信
  - UART2用于上位机通信调试
- **控制器件**: 气缸控制(GPIOF Pin0)

## 功能特性

### 1. 电机控制功能
- **角度控制**: 支持精确的角度定位控制
- **速度控制**: 双环PID控制(角度环+速度环)
- **多电机协调**: 支持多个电机同步运动

### 2. 气缸控制功能
- **气缸推出**: `ballpush()` - 控制气缸向下推
- **气缸收回**: `ballpull()` - 控制气缸向上拉
- **延时控制**: `ballgap()` - 动作间隔控制

### 3. 预设动作模式

#### 2006电机控制模式
- **模式0**: 转到0度位置
- **模式1**: 转到90度位置(18π弧度)

#### 3508电机控制模式  
- **模式0**: 转到0度位置
- **模式1**: 转到180度位置(47π弧度)

#### 综合控制模式
- **模式0**: 2006电机转到0度
- **模式1**: 2006电机转到90度 + 气缸推拉动作序列

## 控制参数

### PID参数设置
- **角度PID**: Kp=15.0, Ki=0.0, Kd=0.0
- **速度PID**: 
  - 2006电机: Kp=200.0, Ki=100.0, Kd=0.0
  - 3508电机: Kp=200.0, Ki=100.0, Kd=0.0

### 气缸控制参数
```c
const int push = 5000;  // 推出延时(ms)
const int pull = 5000;  // 收回延时(ms) 
const int gap = 1000;   // 动作间隔(ms)
```

## 串口通信协议

### 可调参数列表
通过串口可以实时调节以下参数：

| 参数名 | 功能说明 |
|--------|----------|
| pa | 角度PID的Kp参数 |
| ia | 角度PID的Ki参数 |
| da | 角度PID的Kd参数 |
| po | 速度PID的Kp参数 |
| io | 速度PID的Ki参数 |
| do | 速度PID的Kd参数 |
| cmd1 | 2006电机控制命令(0/1) |
| cmd2 | 3508电机控制命令(0/1) |
| cmd3 | 综合控制命令(0/1) |

### 串口配置
- **波特率**: 根据UART2配置
- **数据格式**: 8位数据位，1位停止位，无校验
- **协议**: 支持SerialPlot上位机调试

## 使用说明

### 1. 编译和烧录
1. 使用Keil MDK打开`MDK-ARM/test_feedback.uvprojx`项目文件
2. 编译项目
3. 通过ST-Link或J-Link烧录到STM32F427

### 2. 调试步骤
1. 连接CAN总线到电机控制器
2. 连接UART2到上位机
3. 上电后系统自动初始化
4. 通过串口发送控制命令调节参数

### 3. 控制命令示例
```
pa=20.0    // 设置角度PID的Kp为20.0
cmd1=1     // 2006电机转到90度位置
cmd3=1     // 执行完整抓球动作
```

## 文件结构

```
motor_dribble/
├── Core/Src/main.c          # 主程序文件
├── MDK-ARM/                 # Keil项目文件
│   ├── test_feedback.uvprojx
│   └── test_feedback/       # 编译输出
└── README.md               # 本说明文件
```

## 注意事项

1. **电机ID配置**: 确保CAN总线上的电机ID配置正确
2. **电源要求**: 确保24V电源供应充足
3. **安全操作**: 调试时注意机械运动安全
4. **PID调参**: 建议先从小参数开始逐步调节
5. **气缸控制**: 注意气缸动作时序，避免机械干涉

## 开发环境

- **IDE**: Keil MDK-ARM
- **HAL库**: STM32F4xx HAL Driver
- **调试工具**: SerialPlot上位机
- **编程器**: ST-Link/J-Link

## 参考资料

### 技术文档
- [STM32F427xx参考手册](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-415-stm32f407-417-stm32f427-437-and-stm32f429-439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F4xx HAL驱动用户手册](https://www.st.com/resource/en/user_manual/dm00105879-description-of-stm32f4-hal-and-ll-drivers-stmicroelectronics.pdf)
- [CAN总线协议标准](https://www.iso.org/standard/63648.html)

### 大疆官方资料
- [大疆开发板A型用户手册](https://www.robomaster.com/zh-CN/products/components/general/development-board-type-a)
- [RoboMaster电机使用说明](https://www.robomaster.com/zh-CN/products/components/general/M3508)
- [大疆电调C610/C620使用手册](https://www.robomaster.com/zh-CN/products/components/general/C610)

### 开发工具
- [Keil MDK-ARM官方网站](https://www.keil.com/mdk5/)
- [STM32CubeMX配置工具](https://www.st.com/en/development-tools/stm32cubemx.html)

### 开源资源
- [中科大RoboWalker战队电控开源代码](https://github.com/yssickjgd/robowalker_train)

## 版权信息

Copyright (c) 2023 STMicroelectronics.
All rights reserved. 