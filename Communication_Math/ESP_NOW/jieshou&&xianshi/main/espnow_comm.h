#ifndef ESPNOW_COMM_H
#define ESPNOW_COMM_H

#include "espnow_config.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// ESP-NOW 通信初始化
esp_err_t espnow_comm_init(const espnow_config_t *config);

// ESP-NOW 通信反初始化
esp_err_t espnow_comm_deinit(void);

// 发送传感器数据
esp_err_t espnow_send_sensor_data(const sensor_data_t *data);

// 接收传感器数据
esp_err_t espnow_recv_sensor_data(sensor_data_t *data);

// 获取连接状态
espnow_status_t espnow_get_status(void);

// 设置状态回调函数
typedef void (*espnow_status_callback_t)(espnow_status_t status);
esp_err_t espnow_set_status_callback(espnow_status_callback_t callback);

// 设置数据接收回调函数
typedef void (*espnow_data_callback_t)(const sensor_data_t *data);
esp_err_t espnow_set_data_callback(espnow_data_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // ESPNOW_COMM_H 