#include "ibus.h"

extern UART_HandleTypeDef UART_IBUS_RX_HANDLE;
extern UART_HandleTypeDef UART_TX_HANDLE;

uint8_t Ibusdata[32];
uint16_t Ibus_Channeldata[14];


void ibus_init(void)
{
    HAL_UARTEx_ReceiveToIdle_IT(&UART_IBUS_RX_HANDLE, Ibusdata, 32);
}

void IbusToChannel(uint8_t Ibusdata[32],uint16_t Ibus_Channeldata[14])
{
    for (uint8_t i = 0; i < 14; i ++)
    {
        Ibus_Channeldata[i] = Ibusdata[3 + 2 * i] << 8 | Ibusdata[2 + 2 * i];
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == UART_IBUS_RX_HANDLE.Instance)
    {
        if (Size == 32)
        {
            IbusToChannel(Ibusdata,Ibus_Channeldata);
            RemoveControl_ProcessData(Ibus_Channeldata);
        }
        HAL_UARTEx_ReceiveToIdle_IT(&UART_IBUS_RX_HANDLE, Ibusdata, 32);
    }
}

void uart_printf(const char *format, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    HAL_UART_Transmit(&UART_TX_HANDLE, (uint8_t*)buffer, len, HAL_MAX_DELAY);
}