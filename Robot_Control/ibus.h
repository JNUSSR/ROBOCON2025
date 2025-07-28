#ifndef IBUS_H
#define IBUS_H

#include "main.h"
#include "removecontrol.h"
#include <stdarg.h>
#include <stdio.h>

#define UART_IBUS_RX_HANDLE huart6
#define UART_TX_HANDLE huart6

void ibus_init(void);
void uart_printf(const char *format, ...);

#endif //IBUS_H
