#include "common.h"
#include "hpdl1414.h"
#include "tim.h"
#include "display.h"
#include "key.h"
#include "ds3231.h"
#include <string.h>
#include <math.h>

#define DIS_STATUS_MAIN       0
#define DIS_STATUS_MENU       1

#define DIS_MENU_ROLL           0
#define DIS_MENU_TIME           1
#define DIS_MENU_COUNT          2

static uint16_t _wrPin[] = {GPIO_PIN_0, GPIO_PIN_15, GPIO_PIN_14, GPIO_PIN_13};
static GPIO_TypeDef* _wrGroup[] = {GPIOD, GPIOC, GPIOC, GPIOC};

char time[7] = {49, 3, 22, 20, 22, 33, 5};
char timeShadow[7];
char weekStr[4];
char displayTimeBuff[] = "2019-03-22 20:22:33 FIR";
char timeIndex;

char htuBuff[2];

char displayBuff[16];
char displayBuffMask[16];
char displayRollBuff[64] = "2019-03-22 FRI TEMP:22C HUM:51% ";

char flashFlag;
char lightFlag;
char onePlusFlash;
char flashShadow[16];

int rollTime = 250;
char rollCount;
char rollIndex;
char rollFlag;
char rollScreenIndex = 8;
char rollStrIndex;

char displayReturn;
char displayEnter;

char displayStatus[2] = {1,0};
char displayMenuFlag[2];
char displayMenuIndex;

void weekNum2Str(int num, char *str)
{
    switch(num){
        case 1:
            strcpy(str, "MON");
            break;
        case 2:
            strcpy(str, "TUE");
            break;
        case 3:
            strcpy(str, "WED");
            break;
        case 4:
            strcpy(str, "THU");
            break;
        case 5:
            strcpy(str, "FRI");
            break;
        case 6:
            strcpy(str, "SAT");
            break;
        case 7:
            strcpy(str, "SUN");
            break;
    }
}

void displayClearAll()
{
    for (int i = 0; i < 4; i++)
        hpdl1414ClearAll(_wrGroup[i], _wrPin[i]);
}

//startIndex: 0~15
void displayString(char startIndex, const char* string, const uint8_t clear, const uint8_t resv)
{
    char chipCS;
    if (!resv)
        for (int i = startIndex, j = 0; i < 16 && string[j]; i++, j++){
            chipCS = i/4;
            if (!(i%4))
                hpdl1414ClearAll(_wrGroup[chipCS], _wrPin[chipCS]);
            hpdl1414SendChar(i%4, string[j], _wrGroup[chipCS], _wrPin[chipCS]);
        }
    else 
        for (int i = startIndex, j = 0; i < 16 && string[j]; i++, j++){
            chipCS = i/4;
            if (!(i%4))
                hpdl1414ClearAll(_wrGroup[chipCS], _wrPin[chipCS]);
            hpdl1414SendChar(3-i%4, string[j], _wrGroup[chipCS], _wrPin[chipCS]);
        }

}

void displayShadowString(const char* string, const char* mask)
{
	char chipCS;
    for (int i = 0, j = 0; i < 16 && string[j]; i++, j++){
            chipCS = i/4;
            if (mask[j])
                hpdl1414SendChar(3-i%4, ' ', _wrGroup[chipCS], _wrPin[chipCS]);
            else
                hpdl1414SendChar(3-i%4, string[j], _wrGroup[chipCS], _wrPin[chipCS]);       
    }
}

//rollTime: ms
void displaySetRollBuff(char startIndex, const char* string)
{
    rollStrIndex = 0;
    rollScreenIndex = startIndex;

    int len = strlen(string);
    len  = (len > 64) ? 64 : len;
    memcpy(displayRollBuff, string, len);
}

void displaySetRollTime(const int _rollTime)
{
    rollTime = _rollTime;
    __HAL_TIM_SET_AUTORELOAD(&htim2, rollTime-1);
}

void displayRollStart()
{
    rollFlag = TRUE;

    hpdl1414ClearAll(_wrGroup[2], _wrPin[2]);
    hpdl1414ClearAll(_wrGroup[3], _wrPin[3]);

    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&htim2);
}

void displayRollPasue()
{
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
}

void displayRollResume()
{
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
}

void displaySetStrBuff(char index, const char data)
{
    displayBuff[index] = data;
    displayBuffMask[index] = 1;
}

void displaySetStrBuffArr(char index, const char* data)
{
    int len = strlen(data);
    len = (len > 16) ? 16 : len;
    memcpy(displayBuff, data, len);
    memset(displayBuffMask, 0x00, 16);
    memset(displayBuffMask+index, 0x01, len);
}

void keyCheck()
{
    if (keySPressFlag){
        keySPressFlag = FALSE;

        if (displayStatus[DIS_STATUS_MAIN]){
            displayStatus[DIS_STATUS_MAIN] = FALSE;
            displayStatus[DIS_STATUS_MENU] = TRUE;
            displayMenuIndex = 0;

            displayString(0, "SET ROLL TIME", TRUE, TRUE);
            return;
        }

        if (displayStatus[DIS_STATUS_MENU]){
            displayStatus[DIS_STATUS_MAIN] = FALSE;
            displayStatus[DIS_STATUS_MENU] = FALSE;

            switch (displayMenuIndex){
                case DIS_MENU_ROLL:
                    displayMenuFlag[DIS_MENU_ROLL] = TRUE;
                    displayClearAll();
                    sprintf(displayBuff, "%04d", rollTime);
                    displayString(0, displayBuff, TRUE, TRUE);
                    rollIndex = 0;
                    break;
                case DIS_MENU_TIME:
                    displayMenuFlag[DIS_MENU_TIME] = TRUE;
                    displayClearAll();
                    weekNum2Str(time[6], weekStr);
                    sprintf(displayTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d %s ", (int)time[0]+1970, time[1], time[2], time[3], time[4], time[5], weekStr);
                    displayString(0, displayTimeBuff, TRUE, TRUE);
                    timeIndex = 0;
                    break;
            }
            return;
        }
    }

    if (keyLPressFlag){
        keyLPressFlag = FALSE;

        if (displayStatus[DIS_STATUS_MAIN])
            return;

        if (displayStatus[DIS_STATUS_MENU]){
            displayStatus[DIS_STATUS_MAIN] = TRUE;
            displayStatus[DIS_STATUS_MENU] = FALSE;

            displayString(0, displayTimeBuff+11, TRUE, TRUE);
            return;
        }

        displayStatus[DIS_STATUS_MENU] = TRUE;
        displayStatus[DIS_STATUS_MAIN] = FALSE;
        if (displayMenuFlag[DIS_MENU_ROLL]){
            displayMenuFlag[DIS_MENU_ROLL] = FALSE;

            displaySetRollTime(rollTime);
            displayString(0, "SET ROLL TIME", TRUE, TRUE);
            return;
        }

        if (displayMenuFlag[DIS_MENU_TIME]){
            displayMenuFlag[DIS_MENU_TIME] = FALSE;

            ds3231SetTime(time);
            displayString(0, "SET 3231 TIME", TRUE, TRUE);
            return;
        }

    }

    if (keyUpFlag || keyDownFlag){
        if (displayStatus[DIS_STATUS_MENU]){
            keyUpFlag ? displayMenuIndex++ : displayMenuIndex--;
            displayMenuIndex %= DIS_MENU_COUNT;

            switch(displayMenuIndex){
                case DIS_MENU_ROLL:
                    displayString(0, "SET ROLL TIME", TRUE, TRUE);
                    break;
                case DIS_MENU_TIME:
                    displayString(0, "SET 3231 TIME", TRUE, TRUE);
                    break;
            }

            keyUpFlag = keyDownFlag = FALSE;
            return;
        }

        if (displayMenuFlag[DIS_MENU_ROLL]){
            keyUpFlag ?  (rollTime+=10) : (rollTime-=10);
            if (rollTime < 0)
                rollTime = 990;
            else
                rollTime = rollTime % 1000;
            sprintf(displayBuff, "%04d", rollTime);
            displayString(0, displayBuff, TRUE, TRUE);

            keyUpFlag = keyDownFlag = FALSE;
            return;
        }

        if (displayMenuFlag[DIS_MENU_TIME]){
            keyUpFlag ? time[timeIndex]++ : time[timeIndex]--;

            if (RTC_CheckTime((int)time[0]+1970, time[1], time[2], time[3], time[4], time[5]) != RTC_OK){
                switch(timeIndex){
                    case 0:
                        time[0] = 49;
                        break;
                    case 1:case 2:
                        time[timeIndex] = 1;
                        break;
                    default:
                        time[timeIndex] = 0;
                }
            }
            if (!(time[6] %= 8))
                time[6] += 1;
            weekNum2Str(time[6], weekStr);
            sprintf(displayTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d %s ", (int)time[0]+1970, time[1], time[2], time[3], time[4], time[5], weekStr);
            if (timeIndex >= 5)
                displayString(0, displayTimeBuff+11, TRUE, TRUE);
            else 
                displayString(0, displayTimeBuff, TRUE, TRUE);
            
            keyUpFlag = keyDownFlag = FALSE;
            return;
        }
    }

    if (keyLeftFlag || keyRightFlag){

        if (displayMenuFlag[DIS_MENU_TIME]){
            keyLeftFlag ? timeIndex-- : timeIndex++;

            timeIndex %= 7;

            if (timeIndex >= 5)
                displayString(0, displayTimeBuff+11, TRUE, TRUE);
            else 
                displayString(0, displayTimeBuff, TRUE, TRUE);

            keyLeftFlag = keyRightFlag = FALSE;
            return;
        }
    }
}

void startFlash()
{
    __HAL_TIM_SET_AUTORELOAD(&htim1, 300);
    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&htim1);
}

void stopFlash()
{
    __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE);
    HAL_TIM_Base_Stop_IT(&htim1);
}

void flashCheck()
{
    if (!displayStatus[DIS_MENU_ROLL] && !displayStatus[DIS_MENU_TIME]){
        if (!flashFlag){
            flashFlag = TRUE;
            startFlash();
        }
    } else {
        if (flashFlag){
            flashFlag = FALSE;
            stopFlash();
        }
    }

    if (flashFlag && onePlusFlash){
        onePlusFlash = FALSE;
        switch(displayMenuIndex){
            case DIS_MENU_ROLL:
                memset(flashShadow, 0, 16);
                if (lightFlag)
                    memset(flashShadow, 1, 4);
                displayShadowString(displayBuff, flashShadow);
                break;
            case DIS_MENU_TIME:
                memset(flashShadow, 0, 16);
                if (lightFlag){
                    if (timeIndex == 0){
                        memset(flashShadow, 1, 4);
                    } else if (timeIndex == 5){
                        memset(flashShadow+6, 1, 2);
                    } else if (timeIndex == 6){
                        memset(flashShadow+9, 1, 3);
                    } else {
                        memset(flashShadow+5+3*(timeIndex-1), 1, 2);
                    }
                }
                if (timeIndex >= 5)
                    displayShadowString(displayTimeBuff+11, flashShadow);
                else 
                    displayShadowString(displayTimeBuff, flashShadow);
                break;
        }
        lightFlag = !lightFlag;
    }
}

void displayInit()
{
    memset(displayBuffMask, 1, 8);
    displayString(0, "HPDL1414 TIMER ", TRUE, TRUE);
    HAL_Delay(2000);

	displaySetRollTime(250);
	displayRollStart();
}

void ds3231Init()
{
    while(ds3231CheckBusy()){
        HAL_Delay(10);
    }
    
    ds3231EnableOSC();
    ds3231SetBatteryWaveOut(0);

    //ds3231SetTime(time);
    ds3231GetTime(time);
    weekNum2Str(time[6], weekStr);
    sprintf(displayTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d ", (int)time[0]+1970, time[1], time[2], time[3], time[4], time[5]);
    sprintf(displayRollBuff, "%4d-%02d-%02d %s TEMP:%dC HUM:%d%% ", 
            (int)time[0]+1970, time[1], time[2], 
            weekStr, htuBuff[0], htuBuff[1]);
}

void ds3231Check()
{
    memcpy(timeShadow+3, time, 3);
    displayRollPasue();
    ds3231GetTime(time);
    displayRollResume();
    weekNum2Str(time[6], weekStr);
    sprintf(displayTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d ", (int)time[0]+1970, time[1], time[2], time[3], time[4], time[5]);
    sprintf(displayRollBuff, "%4d-%02d-%02d %s TEMP:%dC HUM:%d%% ", 
            (int)time[0]+1970, time[1], time[2],
            weekStr, htuBuff[0], htuBuff[1]);
    
    for (int i = 3; i < 6; i++){
        if (time[i] != timeShadow[i])
            memset(displayBuffMask+(i-3)*3, 1, 2);
    }
}

void displayCheck()
{
    keyCheck();
    flashCheck();
    if (displayStatus[DIS_STATUS_MAIN])
        ds3231Check();
    

    //check mask
    char chipCS;
    for (int i = 0; i < 16; i++){
        if (!displayBuffMask[i])
            continue;
            
        displayBuffMask[i] = FALSE;
        chipCS = i/4;
        hpdl1414SendChar(3-i%4, displayTimeBuff[i+11], _wrGroup[chipCS], _wrPin[chipCS]);
    }

    //check roll
    if (rollFlag && displayStatus[DIS_STATUS_MAIN]){
        rollFlag = 0;
        
        //rollCount++;
        if (rollScreenIndex > 8){
            displayString(rollScreenIndex, displayRollBuff, TRUE, TRUE);
            rollScreenIndex -= 1;
        } else if (displayRollBuff[rollStrIndex]){
            rollStrIndex++;
            displayString(rollScreenIndex, displayRollBuff+rollStrIndex, TRUE, TRUE);
        } else {
            rollScreenIndex = 15;
            rollStrIndex = 0;
        }
    }
}
