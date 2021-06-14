/**
  ******************************************************************************
  * File Name          : TIM.h
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __tim_H
#define __tim_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

/* USER CODE BEGIN Private defines */
#define RTC_ERR_YEAR    1
#define RTC_ERR_MON     2
#define RTC_ERR_DATE    3
#define RTC_ERR_HOUR    4
#define RTC_ERR_MIN     5
#define RTC_ERR_SEC     6
#define RTC_OK          0
/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);

/* USER CODE BEGIN Prototypes */
extern int8_t RTC_CheckTime(int syear, int smon, int sday, int hour, int min, int sec);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ tim_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
