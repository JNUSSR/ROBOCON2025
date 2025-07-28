#ifndef ESPNOW_CONFIG_H
#define ESPNOW_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// ESP-NOW 配置参数
#define ESPNOW_CHANNEL        1
#define ESPNOW_WIFI_MODE      WIFI_MODE_STA
#define ESPNOW_ENCRYPT        false

// 点对点通信的MAC地址配置
// 请根据实际需要修改这些MAC地址
#define PEER_MAC_ADDR         {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}  // 对端设备MAC地址
#define LOCAL_MAC_ADDR        {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}  // 本机MAC地址（可选）

// 传感器数据结构
typedef struct {
    char sudu1[7];   // 速度1，截取前6个字符
    char x[7];       // X坐标，截取前6个字符
    char y[7];       // Y坐标，截取前6个字符
    char sudu2[7];   // 速度2，截取前6个字符
} sensor_data_t;

// ESP-NOW 状态枚举
typedef enum {
    ESPNOW_STATUS_IDLE = 0,
    ESPNOW_STATUS_CONNECTED,
    ESPNOW_STATUS_DISCONNECTED,
    ESPNOW_STATUS_ERROR
} espnow_status_t;

// ESP-NOW 配置结构体
typedef struct {
    uint8_t peer_mac[6];
    uint8_t local_mac[6];
    uint8_t channel;
    bool encrypt;
    espnow_status_t status;
} espnow_config_t;

// 函数声明
esp_err_t espnow_config_init(espnow_config_t *config);
esp_err_t espnow_config_set_peer_mac(espnow_config_t *config, const uint8_t *mac);
esp_err_t espnow_config_set_local_mac(espnow_config_t *config, const uint8_t *mac);
void espnow_config_print(const espnow_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // ESPNOW_CONFIG_H 