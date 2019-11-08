#include "rainfall.h" 
#include "OnBoard.h"

#define RainfallIO P0_7       //�¶ȴ���������
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

void Delay_s(unsigned int k)
{
    while(k)
    {
        Delay_ms(1000);
        k--;
    }
}

void RainfallDelay(unsigned int k) //ʱ��Ƶ��Ϊ32M 
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

void RainfallInputInitial(void)//���ö˿�Ϊ����
{
    P0DIR &= 0x7f;
}

void RainfallOutputInitial(void)//���ö˿�Ϊ���
{
    P0DIR |= 0x80;
}

//rainfall��ʼ��  ��ʼ���ɹ�����0x00��ʧ�ܷ���0x01
unsigned char RainfallInitial(void)
{
    unsigned char Status = 0x00;
    unsigned int CONT_1 = 0;
    unsigned char Flag_1 = 1;
    RainfallOutputInitial();
    RainfallIO = 1;      //DQ��λ
    RainfallDelay(260);  //������ʱ
    RainfallIO = 0;      //��Ƭ����DQ����
    RainfallDelay(750);  //��ȷ��ʱ ���� 480us С��960us
    RainfallIO = 1;      //��������
    RainfallInputInitial();//����IO����
    while((RainfallIO != 0)&&(Flag_1 == 1))//�ȴ�rainfall��Ӧ�����з�ֹ��ʱ����
    {                                      //�ȴ�Լ60ms����
        CONT_1++;
        RainfallDelay(10);
        if(CONT_1 > 8000)Flag_1 = 0;
        Status = RainfallIO;
    }
    RainfallOutputInitial();
    RainfallIO = 1;
    RainfallDelay(100);
    return Status;       //���س�ʼ��״̬
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
        RainfallIO = 0;// �������ź�
        RainfallDelay(3);
        RainfallIO = 1;// �������ź�
        RainfallDelay(3);
        RainfallInputInitial();
        if(RainfallIO == 1) Value |= 0x80;
        RainfallDelay(15);
    } 
    return Value;
}

//�¶ȶ�ȡ����
unsigned char ReadRainfall(void) 
{
    unsigned char V1,V2;   //����ߵ�8λ ����
    unsigned char temp;    //�����¶Ȼ���Ĵ���
    
    RainfallInitial();
    RainfallWrite(0xcc);    // ����������кŵĲ���
    RainfallWrite(0x44);    // �����¶�ת��
    
    RainfallInitial();
    RainfallWrite(0xcc);    //����������кŵĲ��� 
    RainfallWrite(0xbe);    //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
    
    V1 = RainfallRead();    //��λ
    V2 = RainfallRead();    //��λ
    temp = ((V1 >> 4)+((V2 & 0x07)*16)); //ת������ 

    return temp;
}

//�¶ȶ�ȡ���� ��1λС��λ
float floatReadRainfall(void) 
{
    unsigned char V1,V2;   //����ߵ�8λ ����
    unsigned int temp;     //�����¶Ȼ���Ĵ���
    float fValue;
    RainfallInitial();
    RainfallWrite(0xcc);    // ����������кŵĲ���
    RainfallWrite(0x44);    // �����¶�ת��
    
    RainfallInitial();
    RainfallWrite(0xcc);    //����������кŵĲ��� 
    RainfallWrite(0xbe);    //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
    
    V1 = RainfallRead();    //��λ
    V2 = RainfallRead();    //��λ
    //temp = ((V1 >> 4)+((V2 & 0x07)*16)); //ת������ 
    temp=V2*0xFF+V1;
    fValue = temp*0.0625;
    
    return fValue;
}

unsigned char fun(unsigned char *s)
{
	
  int i,t;             //t��¼��ʱ�ӵ��� 
	
  unsigned char sum =0;
	for(i=0;s[i];i++)
	
  {
		
    if(s[i]>='0'&&s[i]<='9')
		
      t=s[i]-'0';       //���ַ���0~9ʱ����ԭ������
		
    if(s[i]>='a'&&s[i]<='z')
		
      t=s[i]-'a'+10;
		
    if(s[i]>='A'&&s[i]<='Z')
		
      t=s[i]-'A'+10;
		
    sum=sum*16+t;
	
  }
	
  return sum;
 
} 
/*********************************************************************
������Ũ�Ⱥ���
*/
uint16 myApp_ReadRainLevel( void )
{
  uint16 reading = 0;
  
  /* Enable channel */
  ADCCFG |= 0x40;
  
  /* writing to this register starts the extra conversion */
  ADCCON3 = 0x86;// AVDD5 ����  00�� 64 ��ȡ��(7 λENOB)  0110�� AIN6
  
  /* Wait for the conversion to be done */
  while (!(ADCCON1 & 0x80));
  
  /* Disable channel after done conversion */
  ADCCFG &= (0x40 ^ 0xFF); //��λ�����1010^1111=0101�������ƣ�
  
  /* Read the result */
  reading = ADCL;
  reading |= (int16) (ADCH << 8); 
  
  reading >>= 8;
  
  return (reading);
}