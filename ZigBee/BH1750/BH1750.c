#include "BH1750.h"
#include "OnBoard.h"
 
void halMcuWaitUs(uint16 usec)
{
    while(usec--)
    {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
}
//ÒÔmsÑÓÊ±
void halMcuWaitMs(uint16 msec)
{
    while(msec--)
        halMcuWaitUs(1000);
}
 
void Delay_us()
{
  halMcuWaitUs(1);
 // MicroWait(1);
}
void Delay_5us()
{
  halMcuWaitUs(5);
  //MicroWait(5);
}
void Delay_10us()
{
  halMcuWaitUs(10);
  //MicroWait(10);
}
void Delay_ms(int n)
{
  halMcuWaitMs(n);
}
/****************************/
static void start_i2c(void)
{
  SDA_W() ;
  LIGHT_DTA_1();//
  LIGHT_SCK_1() ;//
  Delay_us() ;
  LIGHT_DTA_0() ;
  Delay_us()  ;
  LIGHT_SCK_0() ;
  Delay_us()  ;
  //Delay()  ;
}
 
static void stop_i2c(void)
{
  SDA_W() ;
  LIGHT_DTA_0() ;
  Delay_us();
  LIGHT_SCK_1() ;
  Delay_us();
  LIGHT_DTA_1() ;
  Delay_us();
  LIGHT_SCK_0() ;
  Delay_us();  
}
static char i2c_send(unsigned char val)                 
{
        int i;
        char error=0;
        SDA_W();
        for(i=0x80;i>0;i/=2)
		{
			if(val&i)
				LIGHT_DTA_1();
			else
				LIGHT_DTA_0();
			Delay_us();
			LIGHT_SCK_1() ; 
			Delay_us();
			LIGHT_SCK_0() ;
			Delay_us();					
		}
        LIGHT_DTA_1();
        SDA_R();
        Delay_us();
        //Delay_us();
        LIGHT_SCK_1() ; 
        Delay_us();
        if(LIGHT_DTA())
            error=1;
        Delay_us();
        LIGHT_SCK_0() ;
        return error;
        
}
static char i2c_read(char ack)
{
        int i;
        char val=0;
        LIGHT_DTA_1();
        //SDA_R();
        for(i=0x80;i>0;i/=2)
                {
                        
                        LIGHT_SCK_1() ;
                        Delay_us();
                        SDA_R();
                        //SDA_W();
                        //LIGHT_DTA_0();
                        //LIGHT_DTA_0() ;
                        
                        //Delay_us();
                        if(LIGHT_DTA())
                                val=(val|i);
                        Delay_us();
                        //SDA_R();
                        LIGHT_SCK_0() ;
                        Delay_us();
                        
                        
                }
        SDA_W();
        if(ack)
                LIGHT_DTA_0();
        else
                LIGHT_DTA_1();
        Delay_us();
        LIGHT_SCK_1() ;
        Delay_us();
        LIGHT_SCK_0() ;
        LIGHT_DTA_1();
        return val;
        
}
unsigned short get_light(void)
{        
        unsigned char ack1=1;
        unsigned char ack2=1;
        unsigned char ack3=1;
        unsigned char ack4=1;
        unsigned char ack5=1;
        unsigned char ack6=1;
        unsigned char ack7=1;
        
        unsigned char t0;
        unsigned char t1;
        unsigned short t;
 
        P1DIR |= (1 << 1);
        Delay_ms(200);
 
		start_i2c();
		ack1=i2c_send(0x46);
		if(ack1)
				return 255;
		ack2=i2c_send(0x01);
		if(ack2)
				return 254;
		stop_i2c();           //init
		start_i2c();
		ack3=i2c_send(0x46);
		if(ack3)
				return 253;
		ack4=i2c_send(0x01);
		if(ack4)
				return 252;
		stop_i2c();//power
		start_i2c();
		ack5=i2c_send(0x46);
		if(ack5)
				return 251;
		ack6=i2c_send(0x10);
		if(ack6)
				return 250;
		stop_i2c();                     
        Delay_ms(1500);
        start_i2c();
        
		ack7=i2c_send(0x47);
		if(ack7)
				return 249;
                        
        t0 = i2c_read(1);
        t1 = i2c_read(0);
        stop_i2c();
        t =  ((short)t0)<<8;
        t |= t1;
        return t;
}
 