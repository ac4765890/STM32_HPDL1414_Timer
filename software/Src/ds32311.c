#include "i2c.h"
#include "common.h"
#include <string.h>

#define ds3231Addr 0XD0

#define i2cReadByte(re, addr)               HAL_I2C_Master_Receive(&hi2c1, addr, re, 1, 400)
#define i2cReadString(re, size, addr)       HAL_I2C_Master_Receive(&hi2c1, addr, re, size, 1000)
#define i2cWriteByte(data, addr)            HAL_I2C_Master_Transmit(&hi2c1, addr, data, 1, 400)
#define i2cWriteString(data, size, addr)    HAL_I2C_Master_Transmit(&hi2c1, addr, data, size, 1000)

bool is12Model = FALSE;

void reverse_intcpy(u8 *restrict dst, const u8 *restrict src, size_t n)
{
    size_t i;

    for (i=0; i < n; ++i)
        dst[n-1-i] = src[i];
}

void popEnd(int index, u8 *buff, int size)
{
    int temp = buff[index];
    for (int i = index; i < size; i++){
        buff[i] = buff[i+1];
    }
    buff[size-1] = temp;
}

u8 ds3231DecToBcd(const u8 val) 
{
    return (u8)((val/10*16) + (val%10));
}

u8 ds3231BcdToDec(const u8 val) 
{
    return (u8)((val>>4)*10 + (val&0x0f));
}

u8 ds3231ReadControlByte(char which) 
{
    u8 re;
    u8 temp[2] = {0x0E, 0x0F};
    i2cWriteByte(&temp[which], ds3231Addr);
    i2cReadByte(&re, ds3231Addr);
    
    return re;
}

void ds3231WriteControlByte(char which, u8 control) 
{
    u8 temp[2] = {0, control};
    temp[0] = which ? 0x0E : 0x0F;
    i2cWriteString(temp, 2, ds3231Addr);
}

void ds3231EnableINTCN(bool INT)
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    temp_buffer = INT ? (temp_buffer | 0x04) : (temp_buffer & 0xfb);

    ds3231WriteControlByte(temp_buffer, 0);
}

bool ds3231CheckINTCN()
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    return (temp_buffer & 0x04) ? FALSE : TRUE;
}

bool ds3231CheckEOSC()
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    return (temp_buffer & 0xf0) ? FALSE : TRUE;
}

bool ds3231OscillatorCheck()
{
    u8 temp_buffer = ds3231ReadControlByte(1);
    return (temp_buffer & 0xf0) ? FALSE : TRUE;
}

void ds3231Enable32kHz(bool TF)
{
    u8 temp_buffer = ds3231ReadControlByte(1);
    temp_buffer = TF ? (temp_buffer | 0x0f) : (temp_buffer & 0xf7);
    ds3231WriteControlByte(temp_buffer, 1);
}

void ds3231EnableOscillator(bool TF, bool battery, u8 frequency)
{
    if (frequency > 3) frequency = 3; 
    
    u8 temp_buffer = ds3231ReadControlByte(0) & 0xe7;
    temp_buffer = battery ? (temp_buffer | 0x40) : (temp_buffer & 0xbf);
    temp_buffer = TF ? (temp_buffer & 0xfb) : (temp_buffer | 0xf0);
    
    frequency <<= 3;
    temp_buffer |= frequency;
    
    ds3231WriteControlByte(temp_buffer, 0);
}

bool ds3231CheckIfAlarm(const bool isAlarm0) 
{
    u8 temp_buffer = ds3231ReadControlByte(1);
    u8 re = isAlarm0 ? (temp_buffer & 0x02) : (temp_buffer & 0x01);
    temp_buffer &= isAlarm0 ? 0xfd : 0xfe;

    ds3231WriteControlByte(temp_buffer, 1);
    return re;
}

bool ds3231checkAlarmEnabled(const bool isAlarm0)
{
	u8 temp_buffer = ds3231ReadControlByte(0);
	u8 result = isAlarm0 ? temp_buffer & 0x01 : temp_buffer & 0x02;
	return result;
}

void ds3231SetAlarmStatus(const bool on, const bool isAlarm0)
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    if(on){
        temp_buffer |= isAlarm0 ? 0x06 : 0x05;
    } else {
        temp_buffer &= isAlarm0 ? 0xfd : 0xfe;
    }
    
    ds3231WriteControlByte(temp_buffer, 0);
}

//y mo d h  m s dw
//y mo d dw h m s
//s m  h dw d mo y
void ds3231GetTime(u8 *time)
{
    u8 temp[7];
    temp[0] = 0x00;
    i2cWriteByte(&temp[0], ds3231Addr);
    if (i2cReadString(temp, 7, ds3231Addr) != HAL_OK){
        HAL_I2C_DeInit(&hi2c1);
        HAL_I2C_Init(&hi2c1);
        return;
    }

    temp[2] &= (temp[2] & 0x40) ? 0x1f : 0x3f;
    temp[5] &= 0x7f;
    for (u8 i = 0; i < 7; i++)
        temp[i] = ds3231BcdToDec(temp[i]);
    
    reverse_intcpy(time, temp, 7);
    popEnd(3, time, 7);
}

void ds3231SetTime(u8 *time) // s m h dw d mo y
{
    bool h12;
    u8 temp_buffer;
    u8 string[7];

    reverse_intcpy(string, time, 7);
    temp_buffer = string[0];
    string[0] = string[1];
    string[1] = string[2];
    string[2] = string[3];
    string[3] = temp_buffer;
    temp_buffer = 0x02;
		
    //check busy
    while (ds3231ReadControlByte(1) & 0x04);



    if (i2cWriteByte(&temp_buffer, ds3231Addr) != HAL_OK)
        return;
    if (i2cReadByte(&temp_buffer, ds3231Addr) != HAL_OK)
        return;

    h12 = temp_buffer & 0x40;

    if (h12){
        if (string[2] > 12){
            string[2] = ds3231DecToBcd(string[2] - 0x0c) | 0x60;
        } else {
            string[2] = ds3231DecToBcd(string[2]) & 0xdf;
        }
    } else {
        string[2] = ds3231DecToBcd(string[2]) & 0xbf;
    }

    temp_buffer = 0x00;
    if (i2cWriteByte(&temp_buffer, ds3231Addr) != HAL_OK)
        return;

    temp_buffer = string[2];
    for (u8 i = 0; i < 7; i++)
        string[i] = ds3231DecToBcd(string[i]);
    string[2] = temp_buffer;
    
    if (i2cWriteString(string, 7, ds3231Addr) != HAL_OK)
        return;
    
    temp_buffer = ds3231ReadControlByte(0);
    ds3231WriteControlByte(1, (temp_buffer & 0x7f));

    ds3231ReadControlByte(1);
}

void ds3231SetClockMode(bool h12)
{   
    u8 temp_buffer = 0x02;
    is12Model = !h12;
    i2cWriteByte(&temp_buffer, ds3231Addr);
    i2cReadByte(&temp_buffer, ds3231Addr);

    
    if (h12){
        temp_buffer = temp_buffer | 0x40;
    } else {
        temp_buffer = temp_buffer & 0xbf;
    }

    u8 temp[2] = {0x02, temp_buffer};
    i2cWriteString(temp, 2, ds3231Addr);
}
