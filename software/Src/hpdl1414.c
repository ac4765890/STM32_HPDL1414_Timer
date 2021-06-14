#include "hpdl1414.h"
#include "gpio.h"

static uint16_t _dxaxPin[] = {HPDL_PIN_D0, HPDL_PIN_D1, HPDL_PIN_D2, HPDL_PIN_D3, 
                              HPDL_PIN_D4, HPDL_PIN_D5, HPDL_PIN_D6, HPDL_PIN_A0, HPDL_PIN_A1};

static GPIO_TypeDef* _dxaxGroup[] = {HPDL_GROUP_D0, HPDL_GROUP_D1, HPDL_GROUP_D2, HPDL_GROUP_D3, 
                                     HPDL_GROUP_D4, HPDL_GROUP_D5, HPDL_GROUP_D6, HPDL_GROUP_A0, HPDL_GROUP_A1};

void hpdl1414Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    for (int i = 0; i < 9; i++)
        HAL_GPIO_WritePin(_dxaxGroup[i], _dxaxPin[i], GPIO_PIN_RESET);

    for (int i = 0; i < 9; i++){
        GPIO_InitStruct.Pin = _dxaxPin[i];
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(_dxaxGroup[i], &GPIO_InitStruct);
    }
}

void hpdl1414SendChar(uint8_t index, const char data, GPIO_TypeDef* WR_GROUP, const uint16_t WR_PIN)
{ 
    HAL_GPIO_WritePin(HPDL_GROUP_A0, HPDL_PIN_A0, index & 0x01);
    HAL_GPIO_WritePin(HPDL_GROUP_A1, HPDL_PIN_A1, index & 0x02);
    
    uint8_t mask = 0x01;
    for (int i = 0; i < 7; i++){
        HAL_GPIO_WritePin(_dxaxGroup[i], _dxaxPin[i], data & mask);
        mask <<= 1;
    }
    
    HAL_GPIO_WritePin(WR_GROUP, WR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(WR_GROUP, WR_PIN, GPIO_PIN_SET);
}

void hpdl1414ClearAll(GPIO_TypeDef* WR_GROUP, const uint16_t WR_PIN)
{
    for (int i = 0; i < 4; i++)
        hpdl1414SendChar(i, ' ', WR_GROUP, WR_PIN);
}

void hpdl1414SendString(const char* str, const uint8_t clear, const uint8_t resv, GPIO_TypeDef* WR_GROUP, const uint16_t WR_PIN)
{
    if (clear)
        hpdl1414ClearAll(WR_GROUP, WR_PIN);

    if (!resv)
        for (int i = 0; str[i] && i < 4; i++){
            hpdl1414SendChar(i, str[i], WR_GROUP, WR_PIN);
        }
    else 
        for (int i = 0; str[i] && i < 4; i++){
            hpdl1414SendChar(i, str[3-i], WR_GROUP, WR_PIN);
        }
}
