#include <stdio.h>
#include "esp_timer.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "lvgl.h"
#include "demos/lv_demos.h"

// ESP-NOW 模块化头文件
#include "espnow_config.h"
#include "espnow_comm.h"

#define EXAMPLE_PIN_NUM_SCLK 39
#define EXAMPLE_PIN_NUM_MOSI 38
#define EXAMPLE_PIN_NUM_MISO 40

#define EXAMPLE_SPI_HOST SPI2_HOST

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (80 * 1000 * 1000)

#define EXAMPLE_PIN_NUM_LCD_DC 42
#define EXAMPLE_PIN_NUM_LCD_RST -1
#define EXAMPLE_PIN_NUM_LCD_CS 45

#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8

#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 320

#define EXAMPLE_PIN_NUM_BK_LIGHT 1

#define LCD_BL_LEDC_TIMER LEDC_TIMER_0
#define LCD_BL_LEDC_MODE LEDC_LOW_SPEED_MODE

#define LCD_BL_LEDC_CHANNEL LEDC_CHANNEL_0
#define LCD_BL_LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LCD_BL_LEDC_DUTY (1024)
#define LCD_BL_LEDC_FREQUENCY (10000)

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1

static const char *TAG = "lvgl_example";
static lv_disp_drv_t disp_drv;
static SemaphoreHandle_t lvgl_api_mux = NULL;

esp_lcd_panel_handle_t panel_handle;

// 传感器显示相关的全局变量
lv_obj_t *label_accel_x;
lv_obj_t *label_accel_y;
lv_obj_t *label_accel_z;
lv_obj_t *label_gyro_x;
lv_obj_t *label_gyro_y;
lv_obj_t *label_gyro_z;

lv_timer_t *sensor_timer = NULL;

// ESP-NOW 配置和状态
static espnow_config_t espnow_config;
// 全局变量，保存接收到的字符串
static char received_str[128] = {0};
static bool data_received = false;

bool lvgl_lock(int timeout_ms)
{
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_api_mux, timeout_ticks) == pdTRUE;
}

void lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_api_mux);
}

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_flush_ready(&disp_drv);
    return false;
}

static void example_increase_lvgl_tick(void *arg)
{
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

void lv_port_disp_init(void)
{
    static lv_disp_draw_buf_t draw_buf;
    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf2);
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES);

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;

    lv_disp_drv_register(&disp_drv);
}

void display_init(void)
{
    ESP_LOGI(TAG, "SPI BUS init");
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(EXAMPLE_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Install panel IO");

    esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = example_notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_SPI_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_LOGI(TAG, "Install ST7789 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
}

void bsp_brightness_init(void)
{
    gpio_set_direction(EXAMPLE_PIN_NUM_BK_LIGHT, GPIO_MODE_OUTPUT);
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, 1);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LCD_BL_LEDC_MODE,
        .timer_num = LCD_BL_LEDC_TIMER,
        .duty_resolution = LCD_BL_LEDC_DUTY_RES,
        .freq_hz = LCD_BL_LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LCD_BL_LEDC_MODE,
        .channel = LCD_BL_LEDC_CHANNEL,
        .timer_sel = LCD_BL_LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = EXAMPLE_PIN_NUM_BK_LIGHT,
        .duty = 0,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void bsp_brightness_set_level(uint8_t level)
{
    if (level > 100)
    {
        ESP_LOGE(TAG, "Brightness value out of range");
        return;
    }

    uint32_t duty = (level * (LCD_BL_LEDC_DUTY - 1)) / 100;

    ESP_ERROR_CHECK(ledc_set_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL));

    ESP_LOGI(TAG, "LCD brightness set to %d%%", level);
}

void lvgl_tick_timer_init(uint32_t ms)
{
    ESP_LOGI(TAG, "Install LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, ms * 1000));
}

static void task(void *param)
{
    while (1)
    {
        uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        while (1)
        {
            if (lvgl_lock(-1))
            {
                task_delay_ms = lv_timer_handler();
                lvgl_unlock();
            }
            if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS)
            {
                task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
            }
            else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS)
            {
                task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
            }
            vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
        }
    }
}

// ESP-NOW 数据接收回调函数
static void espnow_data_callback(const sensor_data_t *data)
{
    if (data) {
        // 直接保存结构体内容到全局变量
        memcpy(received_str, data, sizeof(sensor_data_t));
        data_received = true;
        ESP_LOGI(TAG, "接收到ESP-NOW结构体: sudu1=%s, x=%s, y=%s, sudu2=%s jiaodu=%s" , data->sudu1, data->x, data->y, data->sudu2,data->jiaodu);
    }
}

// ESP-NOW 状态回调函数
static void espnow_status_callback(espnow_status_t status)
{
    switch (status) {
        case ESPNOW_STATUS_CONNECTED:
            ESP_LOGI(TAG, "ESP-NOW 连接成功");
            break;
        case ESPNOW_STATUS_DISCONNECTED:
            ESP_LOGW(TAG, "ESP-NOW 连接断开");
            break;
        case ESPNOW_STATUS_ERROR:
            ESP_LOGE(TAG, "ESP-NOW 连接错误");
            break;
        default:
            ESP_LOGI(TAG, "ESP-NOW 状态: %d", status);
            break;
    }
}

// 初始化ESP-NOW点对点通信
static void espnow_init(void)
{
    // 初始化ESP-NOW配置
    ESP_ERROR_CHECK(espnow_config_init(&espnow_config));
    
    // 设置回调函数
    ESP_ERROR_CHECK(espnow_set_data_callback(espnow_data_callback));
    ESP_ERROR_CHECK(espnow_set_status_callback(espnow_status_callback));
    
    // 初始化ESP-NOW通信
    ESP_ERROR_CHECK(espnow_comm_init(&espnow_config));
    
    ESP_LOGI(TAG, "ESP-NOW 点对点通信初始化完成");
}

// 修改传感器数据更新回调
static void sensor_callback(lv_timer_t *timer)
{
    static char last_sudu1[7] = "0";
    static char last_x[7] = "0";
    static char last_y[7] = "0";
    static char last_sudu2[7] = "0";
    static char last_jiaodu[7]="0";

    if (!data_received) {
        // 未收到新数据，显示上一次的值
        lv_label_set_text(label_accel_x, last_sudu1);
        lv_label_set_text(label_accel_y, last_x);
        lv_label_set_text(label_accel_z, last_y);
        lv_label_set_text(label_gyro_x, last_sudu2);
        lv_label_set_text(label_gyro_y, last_jiaodu);
        return;
    }

    const sensor_data_t *pdata = (const sensor_data_t *)received_str;

    // 只要不是"e"，就更新对应的值，否则保持上一次的显示
    if (strcmp(pdata->sudu1, "e") != 0) {
        strncpy(last_sudu1, pdata->sudu1, sizeof(last_sudu1) - 1);
        last_sudu1[sizeof(last_sudu1) - 1] = '\0';
    }
    lv_label_set_text(label_accel_x, last_sudu1);

    if (strcmp(pdata->x, "e") != 0) {
        strncpy(last_x, pdata->x, sizeof(last_x) - 1);
        last_x[sizeof(last_x) - 1] = '\0';
    }
    lv_label_set_text(label_accel_y, last_x);

    if (strcmp(pdata->y, "e") != 0) {
        strncpy(last_y, pdata->y, sizeof(last_y) - 1);
        last_y[sizeof(last_y) - 1] = '\0';
    }
    lv_label_set_text(label_accel_z, last_y);

    if (strcmp(pdata->sudu2, "e") != 0) {
        strncpy(last_sudu2, pdata->sudu2, sizeof(last_sudu2) - 1);
        last_sudu2[sizeof(last_sudu2) - 1] = '\0';
    }
    lv_label_set_text(label_gyro_x, last_sudu2);
if (strcmp(pdata->jiaodu, "e") != 0) {///////////////////////////////////未修改
        strncpy(last_jiaodu, pdata->jiaodu, sizeof(last_jiaodu) - 1);
        last_jiaodu[sizeof(last_jiaodu) - 1] = '\0';
    }
    lv_label_set_text(label_gyro_y, last_jiaodu);
}/////////////////////////////////////////////////////////
// 创建传感器显示界面
void lvgl_sensor_ui_init(lv_obj_t *parent)
{
    lv_obj_t *list = lv_list_create(parent);
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));

    // ESP-NOW 状态显示
    lv_obj_t *list_item = lv_list_add_btn(list, NULL, "ESP-NOW");
    lv_obj_t *status_label = lv_label_create(list_item);
    lv_label_set_text(status_label, " ");

    // 出手速度
    list_item = lv_list_add_btn(list, NULL, "sudu1");
    label_accel_x = lv_label_create(list_item);
    lv_label_set_text(label_accel_x, "0");

    // 雷达坐标
    list_item = lv_list_add_btn(list, NULL, "x");
    label_accel_y = lv_label_create(list_item);
    lv_label_set_text(label_accel_y, "0");

    list_item = lv_list_add_btn(list, NULL, "y");
    label_accel_z = lv_label_create(list_item);
    lv_label_set_text(label_accel_z, "0");

    list_item = lv_list_add_btn(list, NULL, "sudu2");
    label_gyro_x = lv_label_create(list_item);
    lv_label_set_text(label_gyro_x, "0");
    ////////////////////////////////////未修改
    list_item = lv_list_add_btn(list, NULL, "jiaodu");
    label_gyro_y = lv_label_create(list_item);
    lv_label_set_text(label_gyro_y, "0");
    ///////////////////////////////////////////////
    // 创建定时器更新ESP-NOW数据
    sensor_timer = lv_timer_create(sensor_callback, 100, NULL);
}

void app_main(void)
{
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 初始化ESP-NOW
    espnow_init();
    
    lvgl_api_mux = xSemaphoreCreateRecursiveMutex();
    lv_init();
    display_init();
    lv_port_disp_init();
    lvgl_tick_timer_init(EXAMPLE_LVGL_TICK_PERIOD_MS);
    bsp_brightness_init();
    bsp_brightness_set_level(80);
    
    if (lvgl_lock(-1))
    {
        // 创建传感器显示界面
        lvgl_sensor_ui_init(lv_scr_act());
        lvgl_unlock();
    }
    
    xTaskCreatePinnedToCore(task, "bsp_lv_port_task", 1024 * 20, NULL, 5, NULL, 1);
}


