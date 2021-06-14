#ifndef __KEY_H__
#define __KEY_H__

extern char keyUpFlag;
extern char keyDownFlag;
extern char keyLeftFlag;
extern char keyRightFlag;
extern char keySPressFlag;
extern char keyLPressFlag;

extern char keyPressFlag;

extern void keyInit(void);
extern void longPressCheck(void);
 
#endif
