#ifndef __DS3231_H__
#define __DS3231_H__

#define BOOL    char

#define DS3231_SOURCE_TYPE          char
#define DS3231_INTERRPUT_WAVE       0
#define DS3231_INTERRPUT_AlARM      1

#define DS3231_FREQ_TYPE        char
#define DS3231_FREQ_1           0
#define DS3231_FREQ_1024        1
#define DS3231_FREQ_4096        2
#define DS3231_FREQ_8192        3

#define I2C_DEV             hi2c1
#define I2C_TIMEOUT_MS      100

#define I2C_STAUTS_TYPE     HAL_StatusTypeDef
#define I2C_OK              HAL_OK
#define I2C_ERR_BUSY        HAL_BUSY
#define I2C_ERR_TIMEOUT     HAL_TIMEOUT

extern BOOL ds3231SetTime(const char* _time);
extern BOOL ds3231GetTime(char* _time);
extern BOOL ds3231CheckBusy(void);

extern void ds3231EnableOSC(void);
extern void ds3231SetInterSource(const DS3231_SOURCE_TYPE source);
extern void ds3231SetWaveFreq(const DS3231_FREQ_TYPE freq);
extern void ds3231SetBatteryWaveOut(BOOL outPut);
#endif // !__DS3231_H__
