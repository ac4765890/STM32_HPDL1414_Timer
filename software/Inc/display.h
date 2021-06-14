#ifndef __DISPLAY_H__
#define __DISPLAY_H__

extern char displayReturn;
extern char displayEnter;

extern char rollFlag;
extern char onePlusFlash;

extern void displayInit(void);
extern void ds3231Init(void);
extern void displayString(char startIndex, const char* string, const uint8_t clear, const uint8_t resv);
extern void displaySetRollBuff(char startIndex, const char* string);
extern void displayShadowString(const char* string, const char* mask);
extern void displaySetRollTime(const int _rollTime);
extern void displayRollStart(void);
extern void displaySetStrBuff(char index, const char data);
extern void displaySetStrBuffArr(char index, const char* data);
extern void displayCheck(void);

#endif 
