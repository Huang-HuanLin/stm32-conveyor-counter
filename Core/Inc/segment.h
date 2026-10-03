#ifndef __SEGMENT_H
#define __SEGMENT_H

#include "stdint.h"
#include "main.h"
void SEG_Scan(void);
void SEG_ShowNumber(uint32_t num);
void SEG_ShowString(const char *str);
void SEG_Init(void);
void SEG_WriteSegData(uint8_t data);

#endif