#ifndef __DS18B20_H__
#define __DS18B20_H__
#include"iocc2530.h"

extern unsigned char Ds18b20Initial(void);
extern unsigned char ReadDs18B20(void);
extern void floatReadDs18B20(void);

extern unsigned char V1,V2;
extern float fValue;
extern unsigned int Temp;
extern unsigned short Flag;

#endif