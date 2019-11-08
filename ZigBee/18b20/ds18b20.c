#include "iocc2530.h"
#include "ds18b20.h" 
#include "OnBoard.h"

#define Ds18b20IO P0_7       //温度传感器引脚

void Ds18b20Delay(unsigned int k);
void Ds18b20InputInitial(void);
void Ds18b20OutputInitial(void);
unsigned char Ds18b20Initial(void);
void Ds18b20Write(unsigned char infor);
unsigned char Ds18b20Read(void);


unsigned char temp; //储存温度信息
uint16 Flag;
float fValue;

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


void Ds18b20Delay(unsigned int k)
{
  MicroWait(k); //这是协议栈内的1us函数
}


void Ds18b20InputInitial(void)//设置端口为输入
{
  P0DIR &= 0x7f;
}


void Ds18b20OutputInitial(void)//设置端口为输出
{
  P0DIR |= 0x80;
}


//ds18b20初始化 初始化成功返回0x00，失败返回0x01
unsigned char Ds18b20Initial(void)
{
  unsigned char Status = 0x00;
  unsigned int CONT_1 = 0;
  unsigned char Flag_1 = 1;

  Ds18b20OutputInitial();

  Ds18b20IO = 1;      //DQ复位
  Ds18b20Delay(260);    //粗略延时：260us
  Ds18b20IO = 0;    //单片机将DQ拉低
  Ds18b20Delay(750);    //精确延时 大于 480us 小于960us
  Ds18b20IO = 1;   //拉高总线
  Ds18b20InputInitial();//设置IO输入
  while((Ds18b20IO != 0)&&(Flag_1 == 1))//等待ds18b20响应，具有防止超时功能
  {//等待约60ms左右
    CONT_1++;
    Ds18b20Delay(10);
    if(CONT_1 > 8000)Flag_1 = 0;
    Status = Ds18b20IO;
  }
  Ds18b20OutputInitial();
  Ds18b20IO = 1;
  Ds18b20Delay(100);
  return Status; //返回初始化状态

}


void Ds18b20Write(unsigned char infor)
{
  unsigned int i;
  Ds18b20OutputInitial();

  for(i=0;i<8;i++)
  {
    if((infor & 0x01))//若写
    {
      Ds18b20IO = 0;
      Ds18b20Delay(15);//6
      Ds18b20IO = 1;
      Ds18b20Delay(45);//50 
    }


    else
    {
      Ds18b20IO = 0;
      Ds18b20Delay(45);
      Ds18b20IO = 1;
      Ds18b20Delay(15);
    }

    infor >>= 1;//移位至最低位
    }

}


unsigned char Ds18b20Read(void)
{
  unsigned char Value = 0x00;
  unsigned int i;

  Ds18b20OutputInitial();

  Ds18b20IO = 1;


  Ds18b20Delay(2);


  for(i=0;i<8;i++)
  {

    Value >>= 1;


    Ds18b20OutputInitial();

    Ds18b20IO = 0;// 给脉冲信号
    Ds18b20Delay(6);


    Ds18b20IO = 1;// 给脉冲信号
    Ds18b20Delay(4);


    Ds18b20InputInitial();


    if(Ds18b20IO == 1) Value |= 0x80;

    Ds18b20Delay(30);

  }

  return Value;

}


//温度读取函数
unsigned char ReadDs18B20(void)
{

  unsigned char V1,V2;  //定义高低8位 缓冲
  unsigned char Temp;  //定义温度缓冲寄存器
  
  Ds18b20Initial();

  Ds18b20Write(0xcc);   // 跳过读序号列号的操作
  Ds18b20Write(0x44);   // 启动温度转换
  
  Ds18b20Initial();


  Ds18b20Write(0xcc);   //跳过读序号列号的操作
  Ds18b20Write(0xbe); //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
  
  V1 = Ds18b20Read(); //低位
  V2 = Ds18b20Read(); //高位 
  Temp = ((V1 >> 4)+((V2 & 0x07)*16)); //转换数据
  
  return Temp;


}

//温度读取函数 带1位小数位
void floatReadDs18B20(void)
{
  unsigned char V1,V2; //定义高低8位 缓冲 
  unsigned int Temp; //定义温度缓冲寄存器 
  
  Ds18b20Initial();


  Ds18b20Write(0xcc);   // 跳过读序号列号的操作 
  Ds18b20Write(0x44);  // 启动温度转换 
  
  
  Ds18b20Initial();


  Ds18b20Write(0xcc);   //跳过读序号列号的操作
  Ds18b20Write(0xbe);   //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
  
  V1 = Ds18b20Read();    //低位
  V2 = Ds18b20Read();    //高位 
  //Temp = ((V1 >> 4)+((V2 & 0x07)*16));    //转换数据 
  //Temp=V2*0xFF+V1;    

//不含负数位
  //fValue = Temp*0.0625;

  
  if(V2&0xF0) //判断符号位为负
  {
    Flag = 1; //负数标志置一
    Temp = ((V2<<8)|V1);  //高八位低八位数据整合
    Temp = ((~Temp)+1);   //求反，补1
    fValue = Temp*0.0625;
  //求出十进制
  }
  else
  {
    Flag = 0;
    fValue = ((V2<<8)|V1)*0.0625;
  }
  P0DIR |= 0x80; //IO口需要重新配置 
  //return fValue;


}