#ifndef __HPDL1414_H__
#define __HPDL1414_H__

#include "main.h"
#include "stm32f1xx_hal.h"

#define HPDL_SUCCESS    1
#define HPDL_FAILED     0

#define HPDL_PIN_D0     GPIO_PIN_6
#define HPDL_PIN_D1     GPIO_PIN_5
#define HPDL_PIN_D2     GPIO_PIN_4
#define HPDL_PIN_D3     GPIO_PIN_3
#define HPDL_PIN_D4     GPIO_PIN_8
#define HPDL_PIN_D5     GPIO_PIN_9
#define HPDL_PIN_D6     GPIO_PIN_2

#define HPDL_GROUP_D0   GPIOA
#define HPDL_GROUP_D1   GPIOA
#define HPDL_GROUP_D2   GPIOA
#define HPDL_GROUP_D3   GPIOA
#define HPDL_GROUP_D4   GPIOB
#define HPDL_GROUP_D5   GPIOB
#define HPDL_GROUP_D6   GPIOA

#define HPDL_PIN_A0     GPIO_PIN_4
#define HPDL_PIN_A1     GPIO_PIN_5

#define HPDL_GROUP_A0   GPIOB
#define HPDL_GROUP_A1   GPIOB

extern void hpdl1414Init(void);

extern void hpdl1414SendChar(uint8_t index, const char data, GPIO_TypeDef* WR_GROUP, const uint16_t WR_PIN);
extern void hpdl1414ClearAll(GPIO_TypeDef* WR_GROUP, const uint16_t WR_PIN);
extern void hpdl1414SendString(const char* str, const uint8_t clear, const uint8_t resv, GPIO_TypeDef* WR_GROUP, const uint16_t WR_PIN);

#endif
