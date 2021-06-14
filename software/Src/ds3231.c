#include "ds3231.h"
#include "i2c.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define FALSE   0
#define TRUE    1

#define DS3231_DEV_ADDR             0XD0
#define DS3231_REG_ADDR_CTRL        0x0E
#define DS3231_REG_ADDR_STATUS      0x0F
#define DS3231_REG_ADDR_HOUR        0x02


#define I2C_READ_BYTE(RE, ADDR)               HAL_I2C_Master_Receive(&I2C_DEV, ADDR, (uint8_t*)RE, 1, I2C_TIMEOUT_MS)
#define I2C_READ_STRING(RE, SIZE, ADDR)       HAL_I2C_Master_Receive(&I2C_DEV, ADDR, (uint8_t*)RE, SIZE, I2C_TIMEOUT_MS)
#define I2C_WRITE_BYTE(DATA, ADDR)             HAL_I2C_Master_Transmit(&I2C_DEV, ADDR, (uint8_t*)DATA, 1, I2C_TIMEOUT_MS)
#define I2C_WRITE_STRING(DATA, SIZE, ADDR)    HAL_I2C_Master_Transmit(&I2C_DEV, ADDR, (uint8_t*)DATA, SIZE, I2C_TIMEOUT_MS)

#define DS3231_DEC2BCD(VAL)     (char)((VAL/10*16) + (VAL%10))
#define DS3231_BCD2DEC(VAL)     (char)((VAL>>4)*10 + (VAL&0x0F))

char temp;
char tempArr[2];
I2C_STAUTS_TYPE status;

void reverse_cpy(char *dst, const char *src, size_t n)
{
    size_t i;

    for (i=0; i < n; ++i)
        dst[n-1-i] = src[i];
}

void popIndex(uint8_t from, uint8_t to, char *buff)
{
    char temp = buff[from];
    for (uint8_t i = from; i < to; i++){
        buff[i] = buff[i+1];
    }
    buff[to] = temp;
}

void popEnd(uint8_t from, char *buff, uint8_t size)
{
    popIndex(from, size-1, buff);
}

I2C_STAUTS_TYPE ds3231ReadReg(const char reg ,char *re)
{
    status = I2C_OK;
    if ((status = I2C_WRITE_BYTE(&reg, DS3231_DEV_ADDR)) != I2C_OK)
        return status;
    if ((status = I2C_READ_BYTE(re, DS3231_DEV_ADDR)) != I2C_OK)
        return status;

    return status;
}

I2C_STAUTS_TYPE ds3231WriteReg(const char reg ,const char data)
{
    tempArr[0] = reg;
    tempArr[1] = data;
    status = I2C_OK;
    if ((status = I2C_WRITE_STRING(&tempArr, 2, DS3231_DEV_ADDR)) != I2C_OK)
        return status;

    return status;
}

//in: y mo d  h  m s  dw
//i2c:s m  h  dw d mo y
BOOL ds3231SetTime(const char* _time)
{
    char time[8];
    reverse_cpy(time+1, _time, 7);
    popIndex(1, 4, time);

    for (int i = 1; i < 8; i++)
        time[i] = DS3231_DEC2BCD(time[i]);
    
    time[0] = 0x00;

    temp = DS3231_REG_ADDR_HOUR;
    if (I2C_WRITE_BYTE(&temp, DS3231_DEV_ADDR) != I2C_OK)
        return FALSE;
    if (I2C_READ_BYTE(&temp, DS3231_DEV_ADDR) != I2C_OK)
        return FALSE;

    //1=12, 0=24
    //pm=1, am=0
    if (temp & 0x40){
        if (_time[3] > 12){
            time[3] = (DS3231_DEC2BCD(_time[3]-12) | 0x60);
        } else {
            time[3] = (DS3231_DEC2BCD(_time[3]) & 0xDF);
        }
    } else {
        time[3] = (DS3231_DEC2BCD(_time[3]) & 0x3F);
    }

    if (I2C_WRITE_STRING(time, 8, DS3231_DEV_ADDR) != I2C_OK)
        return FALSE;
    return TRUE;
}

//read:s m  h  dw d mo y
BOOL ds3231GetTime(char* _time)
{
    char time[7];
    temp = 0x00;
    if (I2C_WRITE_BYTE(&temp, DS3231_DEV_ADDR) != I2C_OK)
        return FALSE;
    if (I2C_READ_STRING(time, 7, DS3231_DEV_ADDR) != I2C_OK)
        return FALSE;

    time[2] &= (time[2] & 0x40) ? 0x1F : 0x3F;
    time[5] &= 0x1F;

    reverse_cpy(_time, time, 7);
    popEnd(3, _time, 7);

    for (char i = 0; i < 7; i++)
        _time[i] = DS3231_BCD2DEC(_time[i]);
    return TRUE;
}

BOOL ds3231CheckBusy()
{
    ds3231ReadReg(DS3231_REG_ADDR_STATUS, &temp);
    return (temp & 0x40) ? TRUE : FALSE;
}

void ds3231EnableOSC()
{
    ds3231ReadReg(DS3231_REG_ADDR_CTRL, &temp);
    temp &= 0x7F;
    ds3231WriteReg(DS3231_REG_ADDR_CTRL, temp);
}

void ds3231SetInterSource(const DS3231_SOURCE_TYPE source)
{
    ds3231ReadReg(DS3231_REG_ADDR_CTRL, &temp);
    if (source == DS3231_INTERRPUT_WAVE){
        temp &= 0xFB;
    } else if (source == DS3231_INTERRPUT_AlARM){
        temp |= 0x40;
    }
    ds3231WriteReg(DS3231_REG_ADDR_CTRL, temp);
}

void ds3231SetWaveFreq(DS3231_FREQ_TYPE freq)
{
    ds3231ReadReg(DS3231_REG_ADDR_CTRL, &temp);
    freq <<= 3;
    temp &= 0xE7;
    temp |= freq;
    ds3231WriteReg(DS3231_REG_ADDR_CTRL, temp);
}

void ds3231SetBatteryWaveOut(BOOL outPut)
{
    ds3231ReadReg(DS3231_REG_ADDR_CTRL, &temp);
    temp &= 0xBF;
    temp |= (outPut ? 0x40 : 0x00);
    ds3231WriteReg(DS3231_REG_ADDR_CTRL, temp);
}
