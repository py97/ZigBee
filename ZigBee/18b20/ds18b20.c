#include "iocc2530.h"
#include "ds18b20.h" 
#include "OnBoard.h"

#define Ds18b20IO P0_7       //�¶ȴ���������

void Ds18b20Delay(unsigned int k);
void Ds18b20InputInitial(void);
void Ds18b20OutputInitial(void);
unsigned char Ds18b20Initial(void);
void Ds18b20Write(unsigned char infor);
unsigned char Ds18b20Read(void);


unsigned char temp; //�����¶���Ϣ
uint16 Flag;
float fValue;

//��ʱ����
void Delay_us(void) //1 us��ʱ
{
    MicroWait(1);   
}

void Delay_10us(void) //10 us��ʱ
{
   MicroWait(10);
}

void Delay_ms(unsigned int Time)//n ms��ʱ
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
  MicroWait(k); //����Э��ջ�ڵ�1us����
}


void Ds18b20InputInitial(void)//���ö˿�Ϊ����
{
  P0DIR &= 0x7f;
}


void Ds18b20OutputInitial(void)//���ö˿�Ϊ���
{
  P0DIR |= 0x80;
}


//ds18b20��ʼ�� ��ʼ���ɹ�����0x00��ʧ�ܷ���0x01
unsigned char Ds18b20Initial(void)
{
  unsigned char Status = 0x00;
  unsigned int CONT_1 = 0;
  unsigned char Flag_1 = 1;

  Ds18b20OutputInitial();

  Ds18b20IO = 1;      //DQ��λ
  Ds18b20Delay(260);    //������ʱ��260us
  Ds18b20IO = 0;    //��Ƭ����DQ����
  Ds18b20Delay(750);    //��ȷ��ʱ ���� 480us С��960us
  Ds18b20IO = 1;   //��������
  Ds18b20InputInitial();//����IO����
  while((Ds18b20IO != 0)&&(Flag_1 == 1))//�ȴ�ds18b20��Ӧ�����з�ֹ��ʱ����
  {//�ȴ�Լ60ms����
    CONT_1++;
    Ds18b20Delay(10);
    if(CONT_1 > 8000)Flag_1 = 0;
    Status = Ds18b20IO;
  }
  Ds18b20OutputInitial();
  Ds18b20IO = 1;
  Ds18b20Delay(100);
  return Status; //���س�ʼ��״̬

}


void Ds18b20Write(unsigned char infor)
{
  unsigned int i;
  Ds18b20OutputInitial();

  for(i=0;i<8;i++)
  {
    if((infor & 0x01))//��д
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

    infor >>= 1;//��λ�����λ
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

    Ds18b20IO = 0;// �������ź�
    Ds18b20Delay(6);


    Ds18b20IO = 1;// �������ź�
    Ds18b20Delay(4);


    Ds18b20InputInitial();


    if(Ds18b20IO == 1) Value |= 0x80;

    Ds18b20Delay(30);

  }

  return Value;

}


//�¶ȶ�ȡ����
unsigned char ReadDs18B20(void)
{

  unsigned char V1,V2;  //����ߵ�8λ ����
  unsigned char Temp;  //�����¶Ȼ���Ĵ���
  
  Ds18b20Initial();

  Ds18b20Write(0xcc);   // ����������кŵĲ���
  Ds18b20Write(0x44);   // �����¶�ת��
  
  Ds18b20Initial();


  Ds18b20Write(0xcc);   //����������кŵĲ���
  Ds18b20Write(0xbe); //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
  
  V1 = Ds18b20Read(); //��λ
  V2 = Ds18b20Read(); //��λ 
  Temp = ((V1 >> 4)+((V2 & 0x07)*16)); //ת������
  
  return Temp;


}

//�¶ȶ�ȡ���� ��1λС��λ
void floatReadDs18B20(void)
{
  unsigned char V1,V2; //����ߵ�8λ ���� 
  unsigned int Temp; //�����¶Ȼ���Ĵ��� 
  
  Ds18b20Initial();


  Ds18b20Write(0xcc);   // ����������кŵĲ��� 
  Ds18b20Write(0x44);  // �����¶�ת�� 
  
  
  Ds18b20Initial();


  Ds18b20Write(0xcc);   //����������кŵĲ���
  Ds18b20Write(0xbe);   //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
  
  V1 = Ds18b20Read();    //��λ
  V2 = Ds18b20Read();    //��λ 
  //Temp = ((V1 >> 4)+((V2 & 0x07)*16));    //ת������ 
  //Temp=V2*0xFF+V1;    

//��������λ
  //fValue = Temp*0.0625;

  
  if(V2&0xF0) //�жϷ���λΪ��
  {
    Flag = 1; //������־��һ
    Temp = ((V2<<8)|V1);  //�߰�λ�Ͱ�λ��������
    Temp = ((~Temp)+1);   //�󷴣���1
    fValue = Temp*0.0625;
  //���ʮ����
  }
  else
  {
    Flag = 0;
    fValue = ((V2<<8)|V1)*0.0625;
  }
  P0DIR |= 0x80; //IO����Ҫ�������� 
  //return fValue;


}