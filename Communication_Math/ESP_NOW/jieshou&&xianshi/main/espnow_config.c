#include "espnow_config.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_err.h"
#include <string.h>

static const char *TAG = "ESPNOW_CONFIG";

// 默认MAC地址
static const uint8_t DEFAULT_PEER_MAC[6] = PEER_MAC_ADDR;
static const uint8_t DEFAULT_LOCAL_MAC[6] = LOCAL_MAC_ADDR;

esp_err_t espnow_config_init(espnow_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置结构体为空");
        return ESP_ERR_INVALID_ARG;
    }

    // 初始化默认配置
    memcpy(config->peer_mac, DEFAULT_PEER_MAC, 6);
    memcpy(config->local_mac, DEFAULT_LOCAL_MAC, 6);
    config->channel = ESPNOW_CHANNEL;
    config->encrypt = ESPNOW_ENCRYPT;
    config->status = ESPNOW_STATUS_IDLE;

    ESP_LOGI(TAG, "ESP-NOW 配置初始化完成");
    espnow_config_print(config);

    return ESP_OK;
}

esp_err_t espnow_config_set_peer_mac(espnow_config_t *config, const uint8_t *mac)
{
    if (config == NULL || mac == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(config->peer_mac, mac, 6);
    ESP_LOGI(TAG, "对端MAC地址已更新");
    
    return ESP_OK;
}

esp_err_t espnow_config_set_local_mac(espnow_config_t *config, const uint8_t *mac)
{
    if (config == NULL || mac == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(config->local_mac, mac, 6);
    ESP_LOGI(TAG, "本地MAC地址已更新");
    
    return ESP_OK;
}

void espnow_config_print(const espnow_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置结构体为空");
        return;
    }

    ESP_LOGI(TAG, "=== ESP-NOW 配置信息 ===");
    ESP_LOGI(TAG, "对端MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             config->peer_mac[0], config->peer_mac[1], config->peer_mac[2],
             config->peer_mac[3], config->peer_mac[4], config->peer_mac[5]);
    ESP_LOGI(TAG, "本地MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             config->local_mac[0], config->local_mac[1], config->local_mac[2],
             config->local_mac[3], config->local_mac[4], config->local_mac[5]);
    ESP_LOGI(TAG, "通道: %d", config->channel);
    ESP_LOGI(TAG, "加密: %s", config->encrypt ? "是" : "否");
    ESP_LOGI(TAG, "状态: %d", config->status);
    ESP_LOGI(TAG, "========================");
} 