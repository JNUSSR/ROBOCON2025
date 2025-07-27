# ESP-NOW 点对点通信模块化配置

## 概述

本项目将ESP-NOW从广播模式改为点对点模式，并将配置模块化保存在单独的文件中，提高了代码的可维护性和可配置性。

## 文件结构

```
main/
├── espnow_config.h      # ESP-NOW配置头文件
├── espnow_config.c      # ESP-NOW配置实现
├── espnow_comm.h        # ESP-NOW通信模块头文件
├── espnow_comm.c        # ESP-NOW通信模块实现
├── main.c              # 主程序文件
└── README_ESPNOW.md    # 本说明文档
```

## 配置说明

### 1. MAC地址配置

在 `espnow_config.h` 中修改MAC地址配置：

```c
// 对端设备MAC地址（需要根据实际设备修改）
#define PEER_MAC_ADDR         {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}

// 本机MAC地址（可选，用于固定本机MAC）
#define LOCAL_MAC_ADDR        {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
```

### 2. 通信参数配置

```c
#define ESPNOW_CHANNEL        1        // WiFi通道
#define ESPNOW_WIFI_MODE      WIFI_MODE_STA  // WiFi模式
#define ESPNOW_ENCRYPT        false    // 是否加密
```

## 使用方法

### 1. 初始化ESP-NOW

```c
#include "espnow_config.h"
#include "espnow_comm.h"

// 初始化ESP-NOW配置
espnow_config_t config;
ESP_ERROR_CHECK(espnow_config_init(&config));

// 设置回调函数
ESP_ERROR_CHECK(espnow_set_data_callback(your_data_callback));
ESP_ERROR_CHECK(espnow_set_status_callback(your_status_callback));

// 初始化ESP-NOW通信
ESP_ERROR_CHECK(espnow_comm_init(&config));
```

### 2. 数据接收回调

```c
static void your_data_callback(const sensor_data_t *data)
{
    // 处理接收到的传感器数据
    printf("接收到数据: sudu=%d, x=%d, y=%d, z=%d\n",
           data->sudu, data->x, data->y, data->z);
}
```

### 3. 状态回调

```c
static void your_status_callback(espnow_status_t status)
{
    switch (status) {
        case ESPNOW_STATUS_CONNECTED:
            printf("ESP-NOW 连接成功\n");
            break;
        case ESPNOW_STATUS_DISCONNECTED:
            printf("ESP-NOW 连接断开\n");
            break;
        case ESPNOW_STATUS_ERROR:
            printf("ESP-NOW 连接错误\n");
            break;
    }
}
```

### 4. 发送数据

```c
sensor_data_t data = {
    .sudu = 100,
    .x = 200,
    .y = 300,
    .z = 400
};

esp_err_t ret = espnow_send_sensor_data(&data);
if (ret != ESP_OK) {
    printf("发送失败: %s\n", esp_err_to_name(ret));
}
```

### 5. 反初始化

```c
espnow_comm_deinit();
```

## 数据结构

### sensor_data_t

```c
typedef struct {
    int sudu;      // 出手速度
    int x;         // 雷达X坐标
    int y;         // 雷达Y坐标
    int z;         // 雷达Z坐标
} sensor_data_t;
```

### espnow_config_t

```c
typedef struct {
    uint8_t peer_mac[6];      // 对端MAC地址
    uint8_t local_mac[6];     // 本地MAC地址
    uint8_t channel;          // WiFi通道
    bool encrypt;             // 是否加密
    espnow_status_t status;   // 连接状态
} espnow_config_t;
```

## 状态枚举

```c
typedef enum {
    ESPNOW_STATUS_IDLE = 0,        // 空闲状态
    ESPNOW_STATUS_CONNECTED,       // 已连接
    ESPNOW_STATUS_DISCONNECTED,    // 已断开
    ESPNOW_STATUS_ERROR           // 错误状态
} espnow_status_t;
```

## 注意事项

1. **MAC地址配置**：确保对端设备的MAC地址配置正确，否则无法建立连接。

2. **通道配置**：确保发送端和接收端使用相同的WiFi通道。

3. **数据格式**：发送和接收的数据结构必须完全一致。

4. **错误处理**：建议在回调函数中添加适当的错误处理逻辑。

5. **资源管理**：在程序结束时调用 `espnow_comm_deinit()` 释放资源。

## 调试信息

模块会输出详细的调试信息，包括：
- 配置信息
- 连接状态变化
- 数据收发日志
- 错误信息

可以通过修改日志级别来控制输出信息的详细程度。 