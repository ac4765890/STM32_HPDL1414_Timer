#include "common.h"
#include "gpio.h"
#include "display.h"

#define KEY_COUNTOUT 500

#define MENU_SET_ROLL_TIME      0
#define MENU_SET_TIME           1

#define KEY_UP      0
#define KEY_DOWN    1
#define KEY_LEFT    2
#define KEY_RIGHT   3
#define KEY_PRESS   4

char keyUpFlag;
char keyDownFlag;
char keyLeftFlag;
char keyRightFlag;
char keyPressFlag;
char keyLPressFlag;
char keySPressFlag;

void keyInit()
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9|
                        GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

}

void longPressCheck()
{
    uint16_t count = 0;
    if (keyPressFlag){
        keyPressFlag = FALSE;

        while ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) != GPIO_PIN_SET) && count < KEY_COUNTOUT){
            HAL_Delay(1);
            count++;
        }

        if (count >= KEY_COUNTOUT){
            //long press
            printf("LOG: lp/n");
            keyLPressFlag = TRUE;
        } else {
            printf("LOG: sp/n");
            keySPressFlag = TRUE;
        }
        return;
    }
}
