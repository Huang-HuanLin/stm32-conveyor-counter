#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void);
void BT_SendData(uint16_t count, uint16_t alarm, uint8_t state);
void BT_SendString(const char *str);
void BT_SendIfChanged(uint16_t count, uint16_t alarm, uint8_t state);
uint16_t BT_GetData(uint8_t *buf, uint16_t max_len);
uint16_t BT_HasData(void);
void BT_ClearRx(void);
void BT_StartReceive(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */