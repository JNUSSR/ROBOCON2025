#include "espnow_comm.h"
#include "espnow_config.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_err.h"
#include <string.h>

static const char *TAG = "ESPNOW_COMM";

// 全局变量
static espnow_config_t g_espnow_config;
static espnow_status_callback_t g_status_callback = NULL;
static espnow_data_callback_t g_data_callback = NULL;
static bool g_espnow_initialized = false;

// ESP-NOW 接收回调函数
static void espnow_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    if (data_len == sizeof(sensor_data_t)) {
        sensor_data_t received_data;
        memcpy(&received_data, data, sizeof(sensor_data_t));
        
        ESP_LOGI(TAG, "接收到传感器数据: sudu1=%s, x=%s, y=%s, sudu2=%s",
                 received_data.sudu1, received_data.x, received_data.y, received_data.sudu2);
        
        // 调用数据回调函数
        if (g_data_callback) {
            g_data_callback(&received_data);
        }
        
        // 更新状态为已连接
        g_espnow_config.status = ESPNOW_STATUS_CONNECTED;
        if (g_status_callback) {
            g_status_callback(ESPNOW_STATUS_CONNECTED);
        }
    }
}

// ESP-NOW 发送回调函数
static void espnow_send_cb(const wifi_tx_info_t *info, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "ESP-NOW 数据发送成功");
        g_espnow_config.status = ESPNOW_STATUS_CONNECTED;
    } else {
        ESP_LOGE(TAG, "ESP-NOW 数据发送失败，状态: %d", status);
        g_espnow_config.status = ESPNOW_STATUS_ERROR;
    }
    
    if (g_status_callback) {
        g_status_callback(g_espnow_config.status);
    }
}

esp_err_t espnow_comm_init(const espnow_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    if (g_espnow_initialized) {
        ESP_LOGW(TAG, "ESP-NOW 已经初始化");
        return ESP_OK;
    }

    // 复制配置
    memcpy(&g_espnow_config, config, sizeof(espnow_config_t));

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 初始化WiFi
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    
    // 设置本地MAC地址（可选）
    ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, g_espnow_config.local_mac));
    
    ESP_ERROR_CHECK(esp_wifi_start());

    // 初始化ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

    // 添加对端设备
    esp_now_peer_info_t peer_info = {
        .channel = g_espnow_config.channel,
        .ifidx = ESP_IF_WIFI_STA,
        .encrypt = g_espnow_config.encrypt
    };
    memcpy(peer_info.peer_addr, g_espnow_config.peer_mac, 6);
    
    esp_err_t ret = esp_now_add_peer(&peer_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "添加对端设备失败: %s", esp_err_to_name(ret));
        return ret;
    }

    g_espnow_initialized = true;
    g_espnow_config.status = ESPNOW_STATUS_IDLE;
    
    ESP_LOGI(TAG, "ESP-NOW 点对点通信初始化完成");
    espnow_config_print(&g_espnow_config);

    return ESP_OK;
}

esp_err_t espnow_comm_deinit(void)
{
    if (!g_espnow_initialized) {
        ESP_LOGW(TAG, "ESP-NOW 未初始化");
        return ESP_OK;
    }

    // 删除对端设备
    esp_now_del_peer(g_espnow_config.peer_mac);
    
    // 反初始化ESP-NOW
    esp_now_deinit();
    
    // 停止WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    
    g_espnow_initialized = false;
    g_espnow_config.status = ESPNOW_STATUS_DISCONNECTED;
    
    ESP_LOGI(TAG, "ESP-NOW 通信已反初始化");
    
    return ESP_OK;
}

esp_err_t espnow_send_sensor_data(const sensor_data_t *data)
{
    if (!g_espnow_initialized) {
        ESP_LOGE(TAG, "ESP-NOW 未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL) {
        ESP_LOGE(TAG, "数据为空");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = esp_now_send(g_espnow_config.peer_mac, (const uint8_t *)data, sizeof(sensor_data_t));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "发送数据失败: %s", esp_err_to_name(ret));
        g_espnow_config.status = ESPNOW_STATUS_ERROR;
    } else {
        ESP_LOGI(TAG, "发送传感器数据: sudu1=%s, x=%s, y=%s, sudu2=%s",
                 data->sudu1, data->x, data->y, data->sudu2);
    }

    return ret;
}

esp_err_t espnow_recv_sensor_data(sensor_data_t *data)
{
    if (!g_espnow_initialized) {
        ESP_LOGE(TAG, "ESP-NOW 未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL) {
        ESP_LOGE(TAG, "数据缓冲区为空");
        return ESP_ERR_INVALID_ARG;
    }

    // 注意：实际的数据接收是通过回调函数完成的
    // 这个函数主要用于获取最后接收到的数据
    ESP_LOGW(TAG, "请使用回调函数接收数据");
    
    return ESP_ERR_NOT_SUPPORTED;
}

espnow_status_t espnow_get_status(void)
{
    return g_espnow_config.status;
}

esp_err_t espnow_set_status_callback(espnow_status_callback_t callback)
{
    g_status_callback = callback;
    return ESP_OK;
}

esp_err_t espnow_set_data_callback(espnow_data_callback_t callback)
{
    g_data_callback = callback;
    return ESP_OK;
} 