#ifndef __OLED_H
#define __OLED_H

#include "i2c.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define OLED_ADDR 0x78  // 只保留这一个！

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen);

#endif