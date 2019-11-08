#ifndef __BH1750_H
#define __BH1750_H
 
#include <ioCC2530.h>
 
#define st(x)      do { x } while (__LINE__ == -1)
#define HAL_IO_SET(port, pin, val)        HAL_IO_SET_PREP(port, pin, val)
#define HAL_IO_SET_PREP(port, pin, val)   st( P##port##_##pin## = val; )
#define HAL_IO_GET(port, pin)   HAL_IO_GET_PREP( port,pin)
#define HAL_IO_GET_PREP(port, pin)   ( P##port##_##pin)
 
 
#define LIGHT_SCK_0()         HAL_IO_SET(1,3,0)
#define LIGHT_SCK_1()         HAL_IO_SET(1,3,1)
#define LIGHT_DTA_0()         HAL_IO_SET(1,1,0)
#define LIGHT_DTA_1()         HAL_IO_SET(1,1,1)
 
#define LIGHT_DTA()          HAL_IO_GET(1,1)
#define LIGHT_SCK()          HAL_IO_GET(1,3)
 
#define SDA_W() (P1DIR |=(1 << 1)  )
#define SDA_R() (P1DIR &=~(1 << 1) )
 
                       
#define LIGHT_INIT()                           \
do{                                            \
	P1SEL &= ~0x08;                        \
	P1DIR |=0x08;                           \
	P1_3 = 1;                                  \
	                                 \
	P1SEL &= ~0x02;                        \
	P1DIR |= 0x02;                         \
	P1_1 = 1; 	                             \
}while(0)
	
extern unsigned short get_light(void);

extern void Delay_ms(int);
 
#endif // __BH1750_H