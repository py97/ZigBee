/*********************************************************************
* INCLUDES
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "SerialApp.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
#include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"


#include "rainfall.h"
//#include "DHT11.h"
#include "nwk_globals.h"
/*********************************************************************
* MACROS
*/
#define COORD_ADDR   0x00
#define ED_ADDR      0x01
#define UART0        0x00
#define MAX_NODE     0x04
#define UART_DEBUG   0x00        //调试宏,通过串口输出协调器和终端的IEEE、短地址
#define LAMP_PIN     P0_4        //定义P0.4口为继电器输入端
#define GAS_PIN      P0_5        //定义P0.5口为烟雾传感器的输入端  
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr)[0])

//---------------------------------------------------------------------
//标准版不同的终端需要修改此ID,用于识别协调器发过来的数据，ID相同则处理
//专业版自动从Flash获得地址，所有终端固件相同，适合量产
static uint16 EndDeviceID = 0x0002; //终端ID，重要
//---------------------------------------------------------------------


unsigned char tempRX;
//变量

unsigned char  Recdata[128];


/*********************************************************************/

#define STEP1 0x00

#define STEP2 0x01

#define STEP3 0x02

#define STEP4 0x03
static uint8 tasksteps = STEP1;
uint8 adr=0x00;
uint8 fc=0x00;


uint8 Data_Report[30]={0};
uint8 flag=0x00;

/*********************************************************************/
/* CONSTANTS
*/

#if !defined( SERIAL_APP_PORT )
#define SERIAL_APP_PORT  0
#endif

#if !defined( SERIAL_APP_BAUD )
//#define SERIAL_APP_BAUD  HAL_UART_BR_38400
#define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  6
#endif

// Loopback Rx bytes to Tx for throughput testing.
#if !defined( SERIAL_APP_LOOPBACK )
#define SERIAL_APP_LOOPBACK  FALSE
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
//#define SERIAL_APP_TX_MAX  30
#define SERIAL_APP_TX_MAX  40
#endif

#define SERIAL_APP_RSP_CNT  4

// This list should be filled with Application specific Cluster IDs.
const cId_t SerialApp_ClusterList[SERIALAPP_MAX_CLUSTERS] =
{
	SERIALAPP_CLUSTERID
};

const SimpleDescriptionFormat_t SerialApp_SimpleDesc =
{
	SERIALAPP_ENDPOINT,              //  int   Endpoint;
    SERIALAPP_PROFID,                //  uint16 AppProfId[2];
    SERIALAPP_DEVICEID,              //  uint16 AppDeviceId[2];
    SERIALAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
    SERIALAPP_FLAGS,                 //  int   AppFlags:4;
    SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
    (cId_t *)SerialApp_ClusterList,  //  byte *pAppInClusterList;
    SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
    (cId_t *)SerialApp_ClusterList   //  byte *pAppOutClusterList;
};

const endPointDesc_t SerialApp_epDesc =
{
	SERIALAPP_ENDPOINT,
    &SerialApp_TaskID,
    (SimpleDescriptionFormat_t *)&SerialApp_SimpleDesc,
    noLatencyReqs
};

/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

uint8 SerialApp_TaskID;    // Task ID for internal task/event processing.

/*********************************************************************
* EXTERNAL VARIABLES
*/

/*********************************************************************
* EXTERNAL FUNCTIONS
*/

/*********************************************************************
* LOCAL VARIABLES
*/
static bool SendFlag = 0;

static uint8 SerialApp_MsgID;

static afAddrType_t SerialApp_TxAddr;
static afAddrType_t Broadcast_DstAddr;

static uint8 SerialApp_TxSeq;
static uint8 SerialApp_TxBuf[SERIAL_APP_TX_MAX+1];
static uint8 SerialApp_TxLen;

static afAddrType_t SerialApp_RxAddr;
static uint8 SerialApp_RspBuf[SERIAL_APP_RSP_CNT];

static devStates_t SerialApp_NwkState;
static afAddrType_t SerialApp_TxAddr;
static uint8 SerialApp_MsgID;

uint8 NodeData[MAX_NODE][5];         //终端数据缓冲区 0=温度 1=湿度 2=气体 3=灯



/*********************************************************************
* LOCAL FUNCTIONS
*/

static void SerialApp_HandleKeys( uint8 shift, uint8 keys );
static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
static void SerialApp_Send(void);
static void SerialApp_Resp(void);
static void SerialApp_CallBack(uint8 port, uint8 event);

static void PrintAddrInfo(uint16 shortAddr, uint8 *pIeeeAddr);
static void AfSendAddrInfo(void);
static void GetIeeeAddr(uint8 * pIeeeAddr, uint8 *pStr);
static void SerialApp_SendPeriodicMessage( void );
static uint8 GetDataLen(uint8 fc);
static uint8 GetLamp( void );
static uint8 GetGas( void );
static uint8 XorCheckSum(uint8 * pBuf, uint8 len);
uint8 SendData(uint8 addr, uint8 FC);
static uint16 myApp_ReadRainLevel( void );

/*********************************************************************
* @fn      
*
* @brief  
*
* @param   
*
* @return  
*************************************************************************/

void Init_Wifi(void)
{

  HalLcdWriteString( "initwifi", HAL_LCD_LINE_2 );      
 
  HalUARTWrite(UART0, "AT+CIPMUX=1\r\n", 12);
  Delay_ms(500);
  tasksteps=STEP2;

} 

//初始化串口0
void initUART0(void)
{
   //串口
      CLKCONCMD &= ~0x40;                         //设置系统时钟源为32MHZ晶振
      while(CLKCONSTA & 0x40);                    //等待晶振稳定
      CLKCONCMD &= ~0x47;                         //设置系统主时钟频率为32MHZ
  
      PERCFG = 0x00;	//位置1 P0 口
      P0SEL = 0x0c;	//P0_2,P0_3用作串口（外部设备功能）
      P2DIR &= ~0XC0; //P0优先作为UART0

      U0CSR |= 0x80;	//设置为UART方式
      
      U0GCR |= 11;
      U0BAUD |= 216;	//波特率设为115200 根据上面表中获得的数据
      UTX0IF = 1;	//UART0 TX 中断标志初始置位0

      U0CSR |= 0x40; //允许接收
      IEN0 |= 0x84; //开总中断允许接收中断  
  
  
}



/*********************************************************************
* @fn      SerialApp_Init
*
* @brief   This is called during OSAL tasks' initialization.
*
* @param   task_id - the Task ID assigned by OSAL.
*
* @return  none
*/
void SerialApp_Init( uint8 task_id )
{
	halUARTCfg_t uartConfig;
    
    P0SEL &= 0xEf;                  //设置P0.4口为普通IO
    P0DIR |= 0x10;                  //设置P0.4为输出
    LAMP_PIN = 0;                   //高电平继电器断开;低电平继电器吸合
    P0SEL &= ~0x20;                 //设置P0.5为普通IO口
    P0DIR &= ~0x20;                 //P0.5定义为输入口
    P0SEL &= 0x7f;                  //P0_7配置成通用io
	
	SerialApp_TaskID = task_id;
	//SerialApp_RxSeq = 0xC3;
	
	afRegister( (endPointDesc_t *)&SerialApp_epDesc );
	
	RegisterForKeys( task_id );
	
	uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
	uartConfig.baudRate             = SERIAL_APP_BAUD;
	uartConfig.flowControl          = FALSE;
	uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
	uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
	uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
	uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
	uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
	uartConfig.callBackFunc         = SerialApp_CallBack;
	HalUARTOpen (UART0, &uartConfig);
        ////cici
#if defined(ZDO_COORDINATOR)       
        
#else      
    initUART0();
#endif
    
#if defined ( LCD_SUPPORTED )
	HalLcdWriteString( "SerialApp", HAL_LCD_LINE_2 );
#endif
	//HalUARTWrite(UART0, "Init", 4);
        
	ZDO_RegisterForZDOMsg( SerialApp_TaskID, End_Device_Bind_rsp );
	ZDO_RegisterForZDOMsg( SerialApp_TaskID, Match_Desc_rsp );
 //       Init_ESP8266();
#if defined(ZDO_COORDINATOR)       
        Init_Wifi();
#else        
        osal_start_timerEx( SerialApp_TaskID,
                            SERIALAPP_SEND_PERIODIC_EVT,
                            SERIALAPP_SEND_PERIODIC_TIMEOUT ); 
#endif
}

/*********************************************************************
* @fn      SerialApp_ProcessEvent
*
* @brief   Generic Application Task event processor.
*
* @param   task_id  - The OSAL assigned task ID.
* @param   events   - Bit map of events to process.
*
* @return  Event flags of all unprocessed events.
*/
UINT16 SerialApp_ProcessEvent( uint8 task_id, UINT16 events )
{
	(void)task_id;  // Intentionally unreferenced parameter
	
	if ( events & SYS_EVENT_MSG )
	{
		afIncomingMSGPacket_t *MSGpkt;
		
		while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SerialApp_TaskID )) )
		{
			switch ( MSGpkt->hdr.event )
			{
			case ZDO_CB_MSG:
				//SerialApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
				break;
				
			case KEY_CHANGE:
				SerialApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
				break;
				
			case AF_INCOMING_MSG_CMD:
				SerialApp_ProcessMSGCmd( MSGpkt );
				break;
                
            case ZDO_STATE_CHANGE:
              SerialApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
              if ( (SerialApp_NwkState == DEV_ZB_COORD)
                  || (SerialApp_NwkState == DEV_ROUTER)
                  || (SerialApp_NwkState == DEV_END_DEVICE) )
              {
                #if defined(ZDO_COORDINATOR) //协调器通过串口输出自身短地址、IEEE  
                    Broadcast_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
                    Broadcast_DstAddr.endPoint = SERIALAPP_ENDPOINT;
                    Broadcast_DstAddr.addr.shortAddr = 0xFFFF;
                    #if UART_DEBUG           
                    PrintAddrInfo( NLME_GetShortAddr(), aExtendedAddress + Z_EXTADDR_LEN - 1);
                    #endif 
                    //初始化灯的状态，1为熄灭状态，0为点亮
                    NodeData[0][3] = 1;
                    NodeData[1][3] = 1;
                    NodeData[2][3] = 1;
                    NodeData[3][3] = 1;
                #else                        //终端无线发送短地址、IEEE   
                    AfSendAddrInfo();
                #endif
                
              }
              break;				
			default:
				break;
			}
			
			osal_msg_deallocate( (uint8 *)MSGpkt );
		}
		
		return ( events ^ SYS_EVENT_MSG );
	}
    
    //在此事件中可以定时向协调器发送节点传感器参数信息
    if ( events & SERIALAPP_SEND_PERIODIC_EVT )
    {
        SerialApp_SendPeriodicMessage();
        
        osal_start_timerEx( SerialApp_TaskID, SERIALAPP_SEND_PERIODIC_EVT,
            (SERIALAPP_SEND_PERIODIC_TIMEOUT + (osal_rand() & 0x00FF)) );
        
        return (events ^ SERIALAPP_SEND_PERIODIC_EVT);
    }
    
	if ( events & SERIALAPP_SEND_EVT )
	{
		SerialApp_Send();
		return ( events ^ SERIALAPP_SEND_EVT );
	}
	
	if ( events & SERIALAPP_RESP_EVT )
	{
		SerialApp_Resp();
		return ( events ^ SERIALAPP_RESP_EVT );
	}
	
	return ( 0 ); 
}

/*********************************************************************
* @fn      SerialApp_HandleKeys
*
* @brief   Handles all key events for this device.
*
* @param   shift - true if in shift/alt.
* @param   keys  - bit field for key events.
*
* @return  none
*/
void SerialApp_HandleKeys( uint8 shift, uint8 keys )
{
	zAddrType_t txAddr;
	
    if ( keys & HAL_KEY_SW_6 ) //按S1键启动或停止终端定时上报数据 
    {
      if(SendFlag == 0)
        {
        SendFlag = 1;
        HalLedSet ( HAL_LED_1, HAL_LED_MODE_ON );
        osal_start_timerEx( SerialApp_TaskID,
                            SERIALAPP_SEND_PERIODIC_EVT,
                            SERIALAPP_SEND_PERIODIC_TIMEOUT );
        }
        else
        {      
            SendFlag = 0;
            HalLedSet ( HAL_LED_1, HAL_LED_MODE_OFF );
            osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_SEND_PERIODIC_EVT);
        }
    }
    
    if ( keys & HAL_KEY_SW_1 ) //按S2
    {
        LAMP_PIN = ~LAMP_PIN;
    }
    
    if ( keys & HAL_KEY_SW_2 )
    {
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
        
        // Initiate an End Device Bind Request for the mandatory endpoint
        txAddr.addrMode = Addr16Bit;
        txAddr.addr.shortAddr = 0x0000; // Coordinator
        ZDP_EndDeviceBindReq( &txAddr, NLME_GetShortAddr(), 
            SerialApp_epDesc.endPoint,
            SERIALAPP_PROFID,
            SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
            SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
            FALSE );
    }
    
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    
    if ( keys & HAL_KEY_SW_4 )
    {
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
        
        // Initiate a Match Description Request (Service Discovery)
        txAddr.addrMode = AddrBroadcast;
        txAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
        ZDP_MatchDescReq( &txAddr, NWK_BROADCAST_SHORTADDR,
            SERIALAPP_PROFID,
            SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
            SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
            FALSE );
    }

}

void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )
{
    uint16 shortAddr;
    uint8 *pIeeeAddr; 
    uint8 delay;
    uint8 afRxData[30]={0};

    
    
	//查询单个终端上所有传感器的数据 3A 00 01 02 39 23  响应：3A 00 01 02 00 00 00 00 xor 23
	switch ( pkt->clusterId )
	{
	// A message with a serial data block to be transmitted on the serial port.
	case SERIALAPP_CLUSTERID:
        osal_memcpy(afRxData, pkt->cmd.Data, pkt->cmd.DataLength);
        
        #if defined(ZDO_COORDINATOR)
        #else 
            HalUARTWrite (UART0, afRxData, pkt->cmd.DataLength); //调试时通过串口输出
        #endif    
		switch(afRxData[0]) //简单协议命令字解析
		{
#if defined(ZDO_COORDINATOR)
		case 0x3B:  //收到终端无线发过来的短地址和IEEE地址,通过串口输出显示      
			shortAddr=(afRxData[1]<<8)|afRxData[2];
			pIeeeAddr = &afRxData[3];
            #if UART_DEBUG
			PrintAddrInfo(shortAddr, pIeeeAddr + Z_EXTADDR_LEN - 1);
            #endif   
			break;
		case 0x3A:	
            if(afRxData[3] == 0x02) //收到终端传过来的传感器数据并保存
            {  
                NodeData[afRxData[2]-1][0] = afRxData[4];
                NodeData[afRxData[2]-1][1] = afRxData[5];
                NodeData[afRxData[2]-1][2] = afRxData[6];
                NodeData[afRxData[2]-1][3] = afRxData[7];
                NodeData[afRxData[2]-1][4] = 0x00;
                
 /////////////////////////////cici////*******************************////////////////               
                //协调器采集的数据 待发送
              osal_memset(Data_Report,0x00,30);
              osal_memcpy(Data_Report, afRxData, 30); 

              osal_memset(afRxData,0x00,30);
//////////////////**************************************/////////////////////////////////
              }
           ///////////////////////////////////////////////   
            
        #if UART_DEBUG
            HalUARTWrite (UART0, NodeData[afRxData[3]-1], 4); //调试时通过串口输出
            HalUARTWrite (UART0, "\n", 1);
        #endif            
           break;
#else  
		case 0x3A:  //开关灯设备          
        if(afRxData[3] == 0x0A || afRxData[3] == 0x0B || afRxData[3] == 0x0C) //控制终端          
        {  
			if(EndDeviceID == afRxData[2] || afRxData[2]==0xFF)
			{
				if(afRxData[4] == 0)
                {
                    LAMP_PIN = 1;
				//	HalLedSet ( HAL_LED_2, HAL_LED_MODE_OFF );
                }
				else
                {
                    LAMP_PIN = 0;
				//	HalLedSet ( HAL_LED_2, HAL_LED_MODE_ON );
                }
			}
			break;
        }		
#endif
        default :
            break;
        }
        break;
		// A response to a received serial data block.
		case SERIALAPP_CLUSTERID2:
			if ((pkt->cmd.Data[1] == SerialApp_TxSeq) &&
				((pkt->cmd.Data[0] == OTA_SUCCESS) || (pkt->cmd.Data[0] == OTA_DUP_MSG)))
			{
				SerialApp_TxLen = 0;
				osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_SEND_EVT);
			}
			else
			{
				// Re-start timeout according to delay sent from other device.
				delay = BUILD_UINT16( pkt->cmd.Data[2], pkt->cmd.Data[3] );
				osal_start_timerEx( SerialApp_TaskID, SERIALAPP_SEND_EVT, delay );
			}
			break;
			
		default:
			break;
	}
}

uint8 TxBuffer[128];

uint8 SendData(uint8 addr, uint8 FC)
{
	uint8 ret, i, index=4;

        osal_memset(TxBuffer,0x00,128);
        
	TxBuffer[0] = 0x3A;
	TxBuffer[1] = 0x00;
	TxBuffer[2] = addr;
	TxBuffer[3] = FC;

	switch(FC)
	{
	case 0x01: //查询所有终端传感器的数据
		for (i=0; i<MAX_NODE; i++)
		{
			osal_memcpy(&TxBuffer[index], NodeData[i], 4);
			index += 4;
		}
		TxBuffer[index] = XorCheckSum(TxBuffer, index);
		TxBuffer[index+1] = 0x23; 
		
		HalUARTWrite(UART0, TxBuffer, index+2);
        ret = 1;
		break;
	case 0x02: //查询单个终端上所有传感器的数据
		osal_memcpy(&TxBuffer[index], NodeData[addr-1], 4);
		index += 4;
		TxBuffer[index] = XorCheckSum(TxBuffer, index);
		TxBuffer[index+1] = 0x23; 
	
		HalUARTWrite(UART0, TxBuffer, index+2);		
        ret = 1;
		break;   
	default:
        ret = 0;
		break;
	}

    return ret;
}

/*********************************************************************
* @fn      SerialApp_Send
*
* @brief   Send data OTA.
*
* @param   none
*
* @return  none
*/
static void SerialApp_Send(void)
{
    uint8  addr, FC;
//    uint8 checksum=0;
    int len=0;
    int i;
    
    uint8 length[3]={0};
    uint8 tmpbuf[256]={0};
    uint8 cmd[256]={0};
 //  char *p=NULL;
	
#if SERIAL_APP_LOOPBACK
	if (SerialApp_TxLen < SERIAL_APP_TX_MAX)
	{
		SerialApp_TxLen += HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf+SerialApp_TxLen+1,
			SERIAL_APP_TX_MAX-SerialApp_TxLen);
	}
	
	if (SerialApp_TxLen)
	{
		(void)SerialApp_TxAddr;
		if (HalUARTWrite(SERIAL_APP_PORT, SerialApp_TxBuf+1, SerialApp_TxLen))
		{
			SerialApp_TxLen = 0;
		}
		else
		{
			osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
		}
	}
#else
	if (!SerialApp_TxLen && 
		(SerialApp_TxLen = HalUARTRead(UART0, SerialApp_TxBuf, SERIAL_APP_TX_MAX)))
	
        {
        if (SerialApp_TxLen)
        {
            
//////////////////////////***********************/////////////////////////////////////////////
             if(strstr((char *)SerialApp_TxBuf,"+IPD")!=NULL) //接收的指令
             {
               sscanf(SerialApp_TxBuf,"%*[^,]%*c%*[^,]%*c%[^:]%*c%s]",length,tmpbuf);            
               len=atoi(length);
               
               for(i=0;i<SerialApp_TxLen;i++)
               {
                 if(SerialApp_TxBuf[i]==':')          
                   break;
               }
               
               osal_memcpy(cmd,&SerialApp_TxBuf[i+1],len);
               adr = cmd[2];
               fc=cmd[3];
               
               HalLcdWriteString( "cmd ready....", HAL_LCD_LINE_3 );
               
               if(cmd[3]==0x0A) //控制指令 广播出去
	      {   
               //将收到的命令通过无线的形式发送出去
                if (afStatus_SUCCESS == AF_DataRequest(&Broadcast_DstAddr,
                                                        (endPointDesc_t *)&SerialApp_epDesc,
                                                         SERIALAPP_CLUSTERID,
                                                         len, cmd,
                                                         &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS)) 

                  {
                    Delay_ms(500);
                    HalLcdWriteString( "cmd send[ok]....", HAL_LCD_LINE_3 );
                
                  }
                }
              else if(cmd[3]==0x02)//查询指令 上报数据
               {
                    if(flag==0x00)
                    {
                      HalUARTWrite (UART0,"AT+CIPSEND=0,10\r\n",16);
                      flag=0x01;
                    }
             //       Delay_ms(500);
               }
               else if(cmd[3]==0x01)//查询指令 上报数据
               {
                    if(flag==0x00)
                    {
                      HalUARTWrite (UART0,"AT+CIPSEND=0,22\r\n",16);
                      flag=0x01;
                    }
             //       Delay_ms(500);
               }              
               
                                              
               HalLcdWriteString( "wait cmd....", HAL_LCD_LINE_3 );
               
               
             }        
              //    sprintf(Recdata,"%s",SerialApp_TxBuf); ////????
             //     HalLcdWriteString( Recdata, HAL_LCD_LINE_2 );
             else if((strstr(&SerialApp_TxBuf[1],"OK")!=NULL)||(strstr(&SerialApp_TxBuf[1],"no change")!=NULL))
             {
                   if(tasksteps == STEP2)
                   {                     
                     HalLcdWriteString( "CIPMUX...[ok].", HAL_LCD_LINE_2 );
                     HalUARTWrite(UART0, "AT+CIPSERVER=1,33333\r\n", 21);
                     Delay_ms(500);
               
                     tasksteps = STEP3;
                   }
                   else if(tasksteps == STEP3)
                   {
                      HalLcdWriteString( "Init Server.[ok]", HAL_LCD_LINE_2 );
                      
                       tasksteps = STEP4;                                 
                   }
              /*     else if((strstr(SerialApp_TxBuf,"Link")!=NULL))
                   {
                      HalLcdWriteString( "Link [ok]..", HAL_LCD_LINE_3 );
                   } */
                   
              }
              else if(strstr(&SerialApp_TxBuf[1],">")!=NULL)
              {
                osal_memset(SerialApp_TxBuf,0x00,31);
                if(flag==0x01)
                {
  /*               if(cmd[3]==0x02) //查询单个终端
                 {
                  HalUARTWrite(UART0, Data_Report, 10);//发送采集的数据
                  Delay_ms(500);
                  flag=0x00;
                 }
                 else if(cmd[3]==0x01) //查询所有终端
                 {
                   
                 }
                 
  */
                
                  
                   SendData(adr, fc);
                   flag=0x00;
                   
                }
                
              }
              else if((strstr(SerialApp_TxBuf,"Link")!=NULL))
              {
                   HalLcdWriteString( "Link [ok]..", HAL_LCD_LINE_3 );
              } 
              else if((strstr(SerialApp_TxBuf,"UnLink")!=NULL))
              {
                   HalLcdWriteString( "UnLink ..", HAL_LCD_LINE_3 );
               } 
             
               osal_memset(tmpbuf,0x00,31);
              
               osal_memset(SerialApp_TxBuf,0x00,31);            
             
               len=0;
               SerialApp_TxLen = 0;  
 //////////////////////////***********************/////////////////////////////////////////////           
        }
    }
#endif
}

/*********************************************************************
* @fn      SerialApp_Resp
*
* @brief   Send data OTA.
*
* @param   none
*
* @return  none
*/
static void SerialApp_Resp(void)
{
	if (afStatus_SUCCESS != AF_DataRequest(&SerialApp_RxAddr,
		(endPointDesc_t *)&SerialApp_epDesc,
		SERIALAPP_CLUSTERID2,
		SERIAL_APP_RSP_CNT, SerialApp_RspBuf,
		&SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
	{
		osal_set_event(SerialApp_TaskID, SERIALAPP_RESP_EVT);
	}
}

/*********************************************************************
* @fn      SerialApp_CallBack
*
* @brief   Send data OTA.
*
* @param   port - UART port.
* @param   event - the UART port event flag.
*
* @return  none
*/
static void SerialApp_CallBack(uint8 port, uint8 event)
{
	(void)port;
	
	if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) &&
#if SERIAL_APP_LOOPBACK
		(SerialApp_TxLen < SERIAL_APP_TX_MAX))
#else
		!SerialApp_TxLen)
#endif
	{
          	
	SerialApp_Send();
                  
        }      
          
          
	
}

int exchange(unsigned char s)
{
  int n=0;
  if(s>='0'&&s<='9')n=n*16+s-'0';
  if(s>='a'&&s<='f')n=n*16+s-'a'+10;
  if(s>='A'&&s<='F')n=n*16+s-'A'+10;
  return(n);
}

//------------------------------------------------------------------------------------------------------------------------------------------
//查询单个终端上所有传感器的数据 3A 00 01 02 XX 23  响应：3A 00 01 02 00 00 00 00 xor 23
void SerialApp_SendPeriodicMessage( void )
{
    uint8 SendBuf[11]={0}; 
    unsigned char temp,x,y,z;
    unsigned short RainLevel;
    
    SendBuf[0] = 0x3A;                          
    SendBuf[1] = HI_UINT16( EndDeviceID );
    SendBuf[2] = LO_UINT16( EndDeviceID );
    SendBuf[3] = 0x02;                       //FC
    
    Delay_ms(500);
    RainLevel = myApp_ReadRainLevel();
    Delay_ms(500);

    x = RainLevel / 100 ;
    y = RainLevel / 10%10 ;
    z = RainLevel % 10;
    temp = x*100+y*10+z;
    
    SendBuf[4] = temp + '0';  
    SendBuf[5] = GetGas(); 
    SendBuf[6] = GetGas();  //获取气体传感器的状态  
    SendBuf[7] = GetLamp(); //获得灯的状态
    SendBuf[8] = XorCheckSum(SendBuf, 9);
    SendBuf[9] = 0x23;
  
    SerialApp_TxAddr.addrMode = (afAddrMode_t)Addr16Bit;
    SerialApp_TxAddr.endPoint = SERIALAPP_ENDPOINT;
    SerialApp_TxAddr.addr.shortAddr = 0x00;  
    if ( AF_DataRequest( &SerialApp_TxAddr, (endPointDesc_t *)&SerialApp_epDesc,
               SERIALAPP_CLUSTERID,
               10,
               SendBuf,
               &SerialApp_MsgID, 
               0, 
               AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
    // Successfully requested to be sent.
    }
    else
    {
    // Error occurred in request to send.
    }
}



//通过串口输出短地址 IEEE
void PrintAddrInfo(uint16 shortAddr, uint8 *pIeeeAddr)
{
    uint8 strIeeeAddr[17] = {0};
    char  buff[30] = {0};    
    
    //获得短地址   
    sprintf(buff, "shortAddr:%04X   IEEE:", shortAddr);  
 
    //获得IEEE地址
    GetIeeeAddr(pIeeeAddr, strIeeeAddr);
//cici
    #if UART_DEBUG
    HalUARTWrite (UART0, (uint8 *)buff, strlen(buff));
    Delay_ms(10);
    HalUARTWrite (UART0, strIeeeAddr, 16); 
    HalUARTWrite (UART0, "\n", 1);
#endif  
}

void AfSendAddrInfo(void)
{
    uint16 shortAddr;
    uint8 strBuf[11]={0};  
    
    SerialApp_TxAddr.addrMode = (afAddrMode_t)Addr16Bit;
    SerialApp_TxAddr.endPoint = SERIALAPP_ENDPOINT;
    SerialApp_TxAddr.addr.shortAddr = 0x00;   
    
    shortAddr=NLME_GetShortAddr();
    
    strBuf[0] = 0x3B;                          //发送地址给协调器 可用于点播
    strBuf[1] = HI_UINT16( shortAddr );        //存放短地址高8位
    strBuf[2] = LO_UINT16( shortAddr );        //存放短地址低8位
    
    osal_memcpy(&strBuf[3], NLME_GetExtAddr(), 8);
        
   if ( AF_DataRequest( &SerialApp_TxAddr, (endPointDesc_t *)&SerialApp_epDesc,
                       SERIALAPP_CLUSTERID,
                       11,
                       strBuf,
                       &SerialApp_MsgID, 
                       0, 
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }   
}

void GetIeeeAddr(uint8 * pIeeeAddr, uint8 *pStr)
{
  uint8 i;
  uint8 *xad = pIeeeAddr;

  for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
  {
    uint8 ch;
    ch = (*xad >> 4) & 0x0F;
    *pStr++ = ch + (( ch < 10 ) ? '0' : '7');
    i++;
    ch = *xad & 0x0F;
    *pStr++ = ch + (( ch < 10 ) ? '0' : '7');
    i++;
  }
}

uint8 XorCheckSum(uint8 * pBuf, uint8 len)
{
	uint8 i;
	uint8 byRet=0;

	if(len == 0)
		return byRet;
	else
		byRet = pBuf[0];

	for(i = 1; i < len; i ++)
		byRet = byRet ^ pBuf[i];

	return byRet;
}

uint8 GetDataLen(uint8 fc)
{
    uint8 len=0;
    switch(fc)
    {
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
      len = 1;
      break;
    }
    
    return len;
}


//获得P0_4 继电器引脚的电平
uint8 GetLamp( void )
{
  uint8 ret;
  
  if(LAMP_PIN == 0)
    ret = 1;
  else
    ret = 0;
  
  return ret;
}

//获得P0_5 MQ-2气体传感器的数据
uint8 GetGas( void )
{
  uint8 ret;
  
  if(GAS_PIN == 0)
    ret = 0;
  else
    ret = 1;
  
  return ret;
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



