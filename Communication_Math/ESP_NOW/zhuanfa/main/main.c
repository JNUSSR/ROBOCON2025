/*
 * 串口转ESP-NOW发送工程
 * 功能：接收串口字符串，解析成传感器数据结构体，通过ESP-NOW发送
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"

// ESP-NOW 配置参数
#define ESPNOW_CHANNEL        1
#define ESPNOW_WIFI_MODE      WIFI_MODE_STA
#define ESPNOW_ENCRYPT        false

// 点对点通信的MAC地址配置
#define PEER_MAC_ADDR         {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}  // 接收端MAC地址
#define LOCAL_MAC_ADDR        {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}  // 发送端MAC地址

// 数据结构
typedef struct {
    char sudu1[7];   // 速度1，截取前6个字符
    char x[7];       // X坐标，截取前6个字符
    char y[7];       // Y坐标，截取前6个字符
    char sudu2[7];   // 速度2，截取前6个字符
} data_t;

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

#define BUF_SIZE (1024)
#define TASK_STACK_SIZE (4096)

// 字符串分隔标志
#define START_FLAG '$'
#define END_FLAG '#'
#define MAX_DATA_LEN 256  // 最大数据长度

static const char *TAG = "SERIAL_ESPNOW";
static espnow_config_t g_espnow_config;
static bool g_espnow_initialized = false;
static bool g_send_completed = true; // 发送完成标志

// ESP-NOW 发送回调函数
static void espnow_send_cb(const wifi_tx_info_t *info, esp_now_send_status_t status)
{
    // 等待一段时间后再发送回调日志
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "ESP-NOW 数据发送成功");
        g_espnow_config.status = ESPNOW_STATUS_CONNECTED;
    } else {
        ESP_LOGE(TAG, "ESP-NOW 数据发送失败，状态: %d", status);
        g_espnow_config.status = ESPNOW_STATUS_ERROR;
    }
    
    g_send_completed = true; // 设置发送完成标志
}

// 初始化ESP-NOW
esp_err_t espnow_init(void)
{
    if (g_espnow_initialized) {
        ESP_LOGW(TAG, "ESP-NOW 已经初始化");
        return ESP_OK;
    }

    // 设置默认配置
    uint8_t peer_mac[6] = PEER_MAC_ADDR;
    uint8_t local_mac[6] = LOCAL_MAC_ADDR;
    
    memcpy(g_espnow_config.peer_mac, peer_mac, 6);
    memcpy(g_espnow_config.local_mac, local_mac, 6);
    g_espnow_config.channel = ESPNOW_CHANNEL;
    g_espnow_config.encrypt = ESPNOW_ENCRYPT;
    g_espnow_config.status = ESPNOW_STATUS_IDLE;

    ESP_LOGI(TAG, "开始初始化ESP-NOW...");

    // 初始化NVS
    esp_err_t nvs_ret = nvs_flash_init();
    if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_ret);
    ESP_LOGI(TAG, "NVS初始化完成");

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 初始化WiFi
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    
    // 设置本地MAC地址
    ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, g_espnow_config.local_mac));
    
    ESP_ERROR_CHECK(esp_wifi_start());

    // 初始化ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
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
    
    ESP_LOGI(TAG, "ESP-NOW 发送端初始化完成");
    ESP_LOGI(TAG, "对端MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
             g_espnow_config.peer_mac[0], g_espnow_config.peer_mac[1], g_espnow_config.peer_mac[2],
             g_espnow_config.peer_mac[3], g_espnow_config.peer_mac[4], g_espnow_config.peer_mac[5]);
    ESP_LOGI(TAG, "本地MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
             g_espnow_config.local_mac[0], g_espnow_config.local_mac[1], g_espnow_config.local_mac[2],
             g_espnow_config.local_mac[3], g_espnow_config.local_mac[4], g_espnow_config.local_mac[5]);

    return ESP_OK;
}

// 解析带标志的字符串，提取$...#之间的内容
esp_err_t parse_flag_data(const char *str, char *extracted_data, size_t max_len)
{
    if (str == NULL || extracted_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 查找开始标志
    const char *start = strchr(str, START_FLAG);
    if (start == NULL) {
        ESP_LOGW(TAG, "未找到开始标志 '%c'", START_FLAG);
        return ESP_ERR_NOT_FOUND;
    }
    
    // 查找结束标志
    const char *end = strchr(start + 1, END_FLAG);
    if (end == NULL) {
        ESP_LOGW(TAG, "未找到结束标志 '%c'", END_FLAG);
        return ESP_ERR_NOT_FOUND;
    }
    
    // 计算数据长度
    size_t data_len = end - start - 1;
    if (data_len == 0) {
        ESP_LOGW(TAG, "标志之间没有数据");
        return ESP_ERR_INVALID_SIZE;
    }
    
    if (data_len >= max_len) {
        ESP_LOGW(TAG, "数据长度超过最大限制: %d >= %d", data_len, max_len);
        return ESP_ERR_INVALID_SIZE;
    }
    
    // 提取数据
    strncpy(extracted_data, start + 1, data_len);
    extracted_data[data_len] = '\0';
    
    ESP_LOGI(TAG, "成功提取数据: %s", extracted_data);
    return ESP_OK;
}

// 解析串口字符串为数据，截取前几个字符到结构体
esp_err_t parse_data(const char *str, data_t *data)
{
    if (str == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 期望格式: "sudu1:123.4,x:567.8,y:901.2,sudu2:345.6"
    // 或者: "123.4,567.8,901.2,345.6"
    
    char *str_copy = strdup(str);
    if (str_copy == NULL) {
        return ESP_ERR_NO_MEM;
    }

    // 去除换行符和回车符
    char *newline = strchr(str_copy, '\n');
    if (newline) *newline = '\0';
    newline = strchr(str_copy, '\r');
    if (newline) *newline = '\0';

    // 去除首尾空格
    while (*str_copy == ' ') str_copy++;
    char *end = str_copy + strlen(str_copy) - 1;
    while (end > str_copy && *end == ' ') end--;
    *(end + 1) = '\0';

    // 尝试解析带标签的格式 "sudu1:123.4,x:567.8,y:901.2,sudu2:345.6"
    if (strstr(str_copy, "sudu1:") != NULL) {
        char *token = strtok(str_copy, ",");
        while (token != NULL) {
            // 去除token前后的空格
            while (*token == ' ') token++;
            char *token_end = token + strlen(token) - 1;
            while (token_end > token && *token_end == ' ') token_end--;
            *(token_end + 1) = '\0';
            
            if (strncmp(token, "sudu1:", 6) == 0) {
                // 截取前6个字符到sudu1字段
                strncpy(data->sudu1, token + 6, sizeof(data->sudu1) - 1);
                data->sudu1[sizeof(data->sudu1) - 1] = '\0';
            } else if (strncmp(token, "x:", 2) == 0) {
                // 截取前6个字符到x字段
                strncpy(data->x, token + 2, sizeof(data->x) - 1);
                data->x[sizeof(data->x) - 1] = '\0';
            } else if (strncmp(token, "y:", 2) == 0) {
                // 截取前6个字符到y字段
                strncpy(data->y, token + 2, sizeof(data->y) - 1);
                data->y[sizeof(data->y) - 1] = '\0';
            } else if (strncmp(token, "sudu2:", 6) == 0) {
                // 截取前6个字符到sudu2字段
                strncpy(data->sudu2, token + 6, sizeof(data->sudu2) - 1);
                data->sudu2[sizeof(data->sudu2) - 1] = '\0';
            }
            token = strtok(NULL, ",");
        }
    } else {
        // 尝试解析简单格式 "123.4,567.8,901.2,345.6"
        const char *p = str_copy;
        for (int idx = 0; idx < 4; idx++) {
            char *field = NULL;
            switch (idx) {
                case 0: field = data->sudu1; break;
                case 1: field = data->x; break;
                case 2: field = data->y; break;
                case 3: field = data->sudu2; break;
        }
            // 查找下一个逗号
            const char *comma = strchr(p, ',');
            int len = 0;
            if (comma) {
                len = comma - p;
            } else {
                len = strlen(p);
            }
            // 如果字段非空，赋值，否则填"e"
            if (len > 0) {
                int copylen = len > 6 ? 6 : len;
                strncpy(field, p, copylen);
                field[copylen] = '\0';
            } else {
                strcpy(field, "e");
            }
            // 跳到下一个字段
            if (comma)
                p = comma + 1;
            else
                p += len;
        }
    }

    free(str_copy);
    return ESP_OK;
}

// 发送数据
esp_err_t espnow_send_data(const data_t *data)
{
    if (!g_espnow_initialized) {
        ESP_LOGE(TAG, "ESP-NOW 未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL) {
        ESP_LOGE(TAG, "数据为空");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = esp_now_send(g_espnow_config.peer_mac, (const uint8_t *)data, sizeof(data_t));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "发送数据失败: %s", esp_err_to_name(ret));
        g_espnow_config.status = ESPNOW_STATUS_ERROR;
    } else {
        // 等待一段时间后再发送日志
        vTaskDelay(150 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "发送数据: sudu1=%s, x=%s, y=%s, sudu2=%s",
                 data->sudu1, data->x, data->y, data->sudu2);
    }

    return ret;
}

// 串口数据处理任务
static void serial_task(void *arg)
{
    // 初始化ESP-NOW
    esp_err_t ret = espnow_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW 初始化失败");
        return;
    }

    // 配置USB串口
    usb_serial_jtag_driver_config_t usb_config = {
        .rx_buffer_size = BUF_SIZE,
        .tx_buffer_size = BUF_SIZE,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_config));
    ESP_LOGI(TAG, "USB串口初始化完成");

    // 分配接收缓冲区
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    if (data == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return;
    }

    ESP_LOGI(TAG, "等待串口数据输入...");
    ESP_LOGI(TAG, "支持的数据格式:");
    ESP_LOGI(TAG, "1. 带标志格式: $sudu1:123.4,x:567.8,y:901.2,sudu2:345.6#");
    ESP_LOGI(TAG, "2. 带标签格式: sudu1:123.4,x:567.8,y:901.2,sudu2:345.6");
    ESP_LOGI(TAG, "3. 简单格式: 123.4,567.8,901.2,345.6");
    ESP_LOGI(TAG, "注意: 每个字段最多截取前6个字符");
    ESP_LOGI(TAG, "标志说明: 开始标志='%c', 结束标志='%c'", START_FLAG, END_FLAG);

    while (1) {
        // 只有在发送完成时才处理新的串口数据
        if (g_send_completed) {
            int len = usb_serial_jtag_read_bytes(data, (BUF_SIZE - 1), 5 / portTICK_PERIOD_MS);

            // 处理接收到的数据
            if (len > 0) {
                data[len] = '\0';
                
                // 尝试解析带标志的格式
                char extracted_data[MAX_DATA_LEN];
                esp_err_t flag_ret = parse_flag_data((char*)data, extracted_data, sizeof(extracted_data));
                
                if (flag_ret == ESP_OK) {
                    // 带标志格式解析成功，继续解析数据内容
                    data_t parsed_data;
                    esp_err_t parse_ret = parse_data(extracted_data, &parsed_data);
                    if (parse_ret == ESP_OK) {
                        ESP_LOGI(TAG, "解析结果: sudu1=%s, x=%s, y=%s, sudu2=%s", parsed_data.sudu1, parsed_data.x, parsed_data.y, parsed_data.sudu2);
                        
                        // 设置发送未完成标志
                        g_send_completed = false;
                        
                        esp_err_t send_ret = espnow_send_data(&parsed_data);
                        if (send_ret != ESP_OK) {
                            ESP_LOGE(TAG, "发送结构体数据失败");
                            g_send_completed = true; // 发送失败，重置标志
                        } else {
                            ESP_LOGI(TAG, "开始发送数据，等待发送完成...");
                        }
                    } else {
                        ESP_LOGE(TAG, "标志内数据解析失败");
                        g_send_completed = false;
                        esp_err_t send_ret = esp_now_send(g_espnow_config.peer_mac, (uint8_t*)extracted_data, strlen(extracted_data) + 1);
                        if (send_ret != ESP_OK) {
                            ESP_LOGE(TAG, "发送提取数据失败");
                            g_send_completed = true;
                        } else {
                            ESP_LOGI(TAG, "开始发送提取数据，等待发送完成...");
                        }
                    }
                } else {
                    // 尝试解析不带标志的格式
                    data_t parsed_data;
                    esp_err_t parse_ret = parse_data((char*)data, &parsed_data);
                    if (parse_ret == ESP_OK) {
                        ESP_LOGI(TAG, "解析结果: sudu1=%s, x=%s, y=%s, sudu2=%s", parsed_data.sudu1, parsed_data.x, parsed_data.y, parsed_data.sudu2);
                        
                        // 设置发送未完成标志
                        g_send_completed = false;
                        
                        esp_err_t send_ret = espnow_send_data(&parsed_data);
                        if (send_ret != ESP_OK) {
                            ESP_LOGE(TAG, "发送结构体数据失败");
                            g_send_completed = true; // 发送失败，重置标志
                        } else {
                            ESP_LOGI(TAG, "开始发送数据，等待发送完成...");
                        }
                    } else {
                        // 解析失败，直接发送原始字符串
                        ESP_LOGE(TAG, "解析数据失败，直接发送原始字符串");
                        g_send_completed = false;
                        esp_err_t send_ret = esp_now_send(g_espnow_config.peer_mac, data, len + 1);
                        if (send_ret != ESP_OK) {
                            ESP_LOGE(TAG, "发送原始字符串失败");
                            g_send_completed = true; // 发送失败，重置标志
                        } else {
                            ESP_LOGI(TAG, "开始发送原始字符串，等待发送完成...");
                        }
                    }
                }
            }
        } else {
            // 发送未完成，等待一段时间
            vTaskDelay(10); // 等待10ms再检查
        }
        vTaskDelay(1); // 防止看门狗报警，1 tick延时
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "串口转ESP-NOW发送工程启动");
    ESP_LOGI(TAG, "版本: 1.0");
    ESP_LOGI(TAG, "功能: 串口接收数据 -> 解析 -> ESP-NOW发送");
    
    // 创建串口处理任务
    xTaskCreate(serial_task, "serial_task", TASK_STACK_SIZE, NULL, 10, NULL);
} 