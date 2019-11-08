#include "rainfall.h" 
#include "OnBoard.h"

#define RainfallIO P0_7       //温度传感器引脚
//延时函数
void Delay_us(void) //1 us延时
{
    MicroWait(1);   
}

void Delay_10us(void) //10 us延时
{
   MicroWait(10);
}

void Delay_ms(unsigned int Time)//n ms延时
{
  unsigned char i;
  while(Time--)
  {
    for(i=0;i<100;i++)
     Delay_10us();
  }
}

void Delay_s(unsigned int k)
{
    while(k)
    {
        Delay_ms(1000);
        k--;
    }
}

void RainfallDelay(unsigned int k) //时钟频率为32M 
{
  while (k--) {
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP"); } 
}

void RainfallInputInitial(void)//设置端口为输入
{
    P0DIR &= 0x7f;
}

void RainfallOutputInitial(void)//设置端口为输出
{
    P0DIR |= 0x80;
}

//rainfall初始化  初始化成功返回0x00，失败返回0x01
unsigned char RainfallInitial(void)
{
    unsigned char Status = 0x00;
    unsigned int CONT_1 = 0;
    unsigned char Flag_1 = 1;
    RainfallOutputInitial();
    RainfallIO = 1;      //DQ复位
    RainfallDelay(260);  //稍做延时
    RainfallIO = 0;      //单片机将DQ拉低
    RainfallDelay(750);  //精确延时 大于 480us 小于960us
    RainfallIO = 1;      //拉高总线
    RainfallInputInitial();//设置IO输入
    while((RainfallIO != 0)&&(Flag_1 == 1))//等待rainfall响应，具有防止超时功能
    {                                      //等待约60ms左右
        CONT_1++;
        RainfallDelay(10);
        if(CONT_1 > 8000)Flag_1 = 0;
        Status = RainfallIO;
    }
    RainfallOutputInitial();
    RainfallIO = 1;
    RainfallDelay(100);
    return Status;       //返回初始化状态
}

void RainfallWrite(unsigned char infor)
{
    unsigned int i;
    RainfallOutputInitial();
    for(i=0;i<8;i++)
    {
        if((infor & 0x01))
        {
            RainfallIO = 0;
            RainfallDelay(6);
            RainfallIO = 1;
            RainfallDelay(50);
        }
        else
        {
            RainfallIO = 0;
            RainfallDelay(50);
            RainfallIO = 1;
            RainfallDelay(6);
        }
        infor >>= 1;
    }
}

unsigned char RainfallRead(void)
{
    unsigned char Value = 0x00;
    unsigned int i;
    RainfallOutputInitial();
    RainfallIO = 1;
    RainfallDelay(10);
    for(i=0;i<8;i++)
    {
        Value >>= 1; 
        RainfallOutputInitial();
        RainfallIO = 0;// 给脉冲信号
        RainfallDelay(3);
        RainfallIO = 1;// 给脉冲信号
        RainfallDelay(3);
        RainfallInputInitial();
        if(RainfallIO == 1) Value |= 0x80;
        RainfallDelay(15);
    } 
    return Value;
}

//温度读取函数
unsigned char ReadRainfall(void) 
{
    unsigned char V1,V2;   //定义高低8位 缓冲
    unsigned char temp;    //定义温度缓冲寄存器
    
    RainfallInitial();
    RainfallWrite(0xcc);    // 跳过读序号列号的操作
    RainfallWrite(0x44);    // 启动温度转换
    
    RainfallInitial();
    RainfallWrite(0xcc);    //跳过读序号列号的操作 
    RainfallWrite(0xbe);    //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
    
    V1 = RainfallRead();    //低位
    V2 = RainfallRead();    //高位
    temp = ((V1 >> 4)+((V2 & 0x07)*16)); //转换数据 

    return temp;
}

//温度读取函数 带1位小数位
float floatReadRainfall(void) 
{
    unsigned char V1,V2;   //定义高低8位 缓冲
    unsigned int temp;     //定义温度缓冲寄存器
    float fValue;
    RainfallInitial();
    RainfallWrite(0xcc);    // 跳过读序号列号的操作
    RainfallWrite(0x44);    // 启动温度转换
    
    RainfallInitial();
    RainfallWrite(0xcc);    //跳过读序号列号的操作 
    RainfallWrite(0xbe);    //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
    
    V1 = RainfallRead();    //低位
    V2 = RainfallRead();    //高位
    //temp = ((V1 >> 4)+((V2 & 0x07)*16)); //转换数据 
    temp=V2*0xFF+V1;
    fValue = temp*0.0625;
    
    return fValue;
}

unsigned char fun(unsigned char *s)
{
	
  int i,t;             //t记录临时加的数 
	
  unsigned char sum =0;
	for(i=0;s[i];i++)
	
  {
		
    if(s[i]>='0'&&s[i]<='9')
		
      t=s[i]-'0';       //当字符是0~9时保持原数不变
		
    if(s[i]>='a'&&s[i]<='z')
		
      t=s[i]-'a'+10;
		
    if(s[i]>='A'&&s[i]<='Z')
		
      t=s[i]-'A'+10;
		
    sum=sum*16+t;
	
  }
	
  return sum;
 
} 
/*********************************************************************
读雨量浓度函数
*/
uint16 myApp_ReadRainLevel( void )
{
  uint16 reading = 0;
  
  /* Enable channel */
  ADCCFG |= 0x40;
  
  /* writing to this register starts the extra conversion */
  ADCCON3 = 0x86;// AVDD5 引脚  00： 64 抽取率(7 位ENOB)  0110： AIN6
  
  /* Wait for the conversion to be done */
  while (!(ADCCON1 & 0x80));
  
  /* Disable channel after done conversion */
  ADCCFG &= (0x40 ^ 0xFF); //按位异或。如1010^1111=0101（二进制）
  
  /* Read the result */
  reading = ADCL;
  reading |= (int16) (ADCH << 8); 
  
  reading >>= 8;
  
  return (reading);
}