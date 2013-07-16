/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
//#include "stm32_eth.h"
//#include "netconf.h"
#include "main.h"
//#include "helloworld.h"
//#include "httpd.h"
//#include "tftpserver.h"
#include <string.h>
#include "usart.h"
//#include "w25q16.h"
//#include "Bins.h"
#include "Protocol.h"
#include "TwinReader.h"
#include "keycrypto.h"
#include "zhuzhouCard.h"
#include "ShenzhenBikeDefine.h"

#include "LockCtrler.h"
#include "SpiFile.h"

#include "w25q16.h"

#include <vector>
#include <map>
#include <string>
#include <stddef.h>
#include <iostream>
using namespace std;

using namespace std;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;
TwinReader gReader;
//ICard::BLOCK_BASE  gHackBlock;

/* Private function prototypes -----------------------------------------------*/
void System_Periodic_Handle(void);

#define GPIOB_CRL   (*(volatile unsigned long *)0x40011400) //端口B配置低寄存器
#define GPIOB_BSRR   (*(volatile unsigned long *)0x40011410) //端口B位设置/复位寄存器
#define GPIOB_BRR    (*(volatile unsigned long *)0x40011414) //端口B位复位寄存
#define RCC_APB2ENR  (*((volatile unsigned int*)(0x40021018)))
int c;
void delayTest(void)
{
    volatile unsigned int i;
    for( i = 0; i < 5000000; ++i);
}

/*
 * STL Test Code
 */

//typedef std::vector<int>       IntVectorType;
//typedef IntVectorType::iterator IntVectorItr;
//typedef std::map<int, int>     IntIntMapType;
//typedef IntIntMapType::iterator IntIntMapItr;
////
//IntVectorType v;
////IntIntMapType m;
//std::string str;

extern unsigned char TI_POINT; //定义指针

//void stl_test (int &sum)  {
////  int   sum = 0;
//
//// vector test
//  sum = v.size();
//  v.push_back(1);
//  v.push_back(2);
//  v.push_back(3);
//  sum = v.size();
//
//  sum = 0;
//  for (IntVectorItr itr = v.begin(); itr != v.end(); itr++)  {
//    sum += *itr;
//  }
////  printf ("*** stl_test: vector sum = %d\n", sum);
//
////// map test
////  sum  = 0;
////  m[1] = 1;
////  m[2] = 2;
////  m[3] = 3;
////
////  for (IntIntMapItr itr = m.begin(); itr != m.end(); itr++)  {
////    sum += itr->first * itr->second;
////  }
////  printf ("*** stl_test: map sum = %d\n", sum);
////
////// Exception-test:
//////  Note: execptions need to be enabled in
//////   Options for Target - C/C++ - Misc controls: '--exceptions'
////  std::cout << "Throwing an exception:" << std::endl;
////  printf ("----------------------\n");
////
////  try  {
////    throw "Test exception";
////  }
////  catch (const char *szMessage)  {
////    str = szMessage;
////  }
//
////  if (str.length() > 0)  {
////    str += " - has been catched !";
////  }
////  std::cout << str << std::endl;
//}

//int receComdata(unsigned char* USART_RX_BUF){
//	TI_POINT = 0;
//	while(TI_POINT < 20){
//		while (!(USART2->SR & USART_FLAG_RXNE));
//	  	USART_RX_BUF[TI_POINT] = USART2->DR;
//		if(USART_RX_BUF[TI_POINT] == 'a'){
//			break;
//		}
//		TI_POINT++;
//		if( TI_POINT>USART_RX_LEN-1 ) TI_POINT = 0;
//	}
//
//}

inline unsigned char c2i(char a, int _Radix = 16){
    int v = a - 0x30;

    switch(_Radix){
        case 2:
            return (v == 1) ? 1 : 0;
            break;
        case 8:
            return (unsigned char)(v < 0) ? 0 : ((v > 7) ? 0 : v);
            break;
        case 10:
            return (unsigned char)(v < 0) ? 0 : ((v > 9) ? 0 : v);
            break;
        case 16:
            v = (a >= 'a') ? (a - 'a'+10)
              : ((a >= 'A') ? (a - 'A'+10)
              : ((a > '9') ? 0
              : ((a >= '0') ? (a - '0')
              : 0 )));
            return (unsigned char)v;
            break;
        default:
            return 0;
    }
}

inline BikeRFID get_rfid(){
    #ifdef STM32F10X_READER
        int confRetryRFID = 10;
    #endif
    BikeRFID r;
    int err;
    err=gReader.rfSelectEx(r);
    for(int i=confRetryRFID; i>0 && r.empty(); i--){
        err=gReader.rfSelectEx(r);
    }
    #ifdef STM32F10X_READER
    #else
        gBike = r;;
    #endif
    //r.m_bEmpty = false;
    return r;
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    USART_Configuration();
//
//	LockCtrler  lock;
//
//	printf("Init ok  , waitint bike ...");
//	while(true)
//	{
//	 	if(lock.BikeNear())
//		{
//			printf("Bike near               ");
//			break;
//		}
//	}
//	lock.TryToOpenLock();
//
//	Delay(1000);
//	if(lock.IsLockOpened())
//		printf("LOCK did open               ");
//	else
//		printf("Open wait time out           ");
//
// 	lock.TryToCloseLock();
//	Delay(1000);
//	if(lock.IsLockClosed())
//		printf("Lock did close              ");
//	else
//		printf("Close wait time out          ");

        cout << "Main begin ..." << endl;
//	   SPIFile file((u8*)"kkkk" , SPIFile::modeReadWrite);
//	   u8 content[10] = {'a' , 'b' , 'c' , 'd' , 'e'};
//	   memset(content , 'a' , 10);
//	   for(int i = 0 ; i < 10 ; ++ i)
//	   cout << content[i] << endl;
//	   file.Write(content , 10);
//
//
//		cout << "SPIFile Write finished !" << endl;
//	   u8 buffer[256] = {0};
//	 //  file.ReadNextFile(buffer);
//	 file.Read(buffer , 10 , 0);
//	   cout << "SPIFile Read finished !" << endl;
//	  for (int i = 0 ; i < 10 ;  ++ i)
//	  		printf("%d : %02X   " , i , buffer[i]);

     W25Q16_Init();

     u8 content[256];
     memset(content , 'a' , 256);


     SPIFile file((u8*)"20137102030returnbike"  , SPIFile::modeReadWrite);
     file.Write(content ,  252);


     file.SetFileName((u8*)"200000000retrunBike");
     memset(content , 'b' , 256);
     file.Write(content , 256);

    // u8 buffer[256];
    // memset(buffer , 0 , 256);

    // file.Read(buffer , 256 , 0);

     u8 buffer2[400];
     memset (buffer2 , 0 , 400);

     int len = file.ReadNextFile(buffer2);
      printf("Totally read : %d \r\n" , len);

      memset(buffer2 , 0 , 400);
      len = file.ReadNextFile(buffer2);
      printf("Totally read : %d \r\n" , len);
    return 0;
}


/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
//  timingdelay = LocalTime + nCount;
//
//  /* wait until the desired delay finish */
//  while(timingdelay > LocalTime)
//  {
//    	Time_Update();
//  }

  for(int i = 0 ; i < 30000 ; ++ i)
    for(int j = 0 ; j < 1000 ; ++ j);
  printf("Delay off          ");
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}

/**
  * @brief  Handles the periodic tasks of the system
  * @param  None
  * @retval None
  */
void System_Periodic_Handle(void)
{
//  /* Update the LCD display and the LEDs status */
//  /* Manage the IP address setting */
//  Display_Periodic_Handle(LocalTime);
//
//  /* LwIP periodic services are done here */
//  LwIP_Periodic_Handle(LocalTime);
}

void W25Q16_test(void)
{
//	uint8_t TX_Buffer[256];
//	uint8_t RX_Buffer[256];
//	uint16_t x, y;
//	uint8_t temp;
//	SPI_Flash_Erase_Chip();   /* 全部擦除 */
//
//	temp = SPI_Flash_ReadSR();					/* 读状态寄存器，看是否被写保护*/
//	printf("\r\n The temp is 0x%x \r\n", temp);
//
//	for(x=0; x<256; x++)
//	{
//		TX_Buffer[x] = x + 1;
//	}
//
//	SPI_Flash_Write_Page(TX_Buffer, 0, 256);	/* 从地址0写256Byte */
//
//    SPI_Flash_Read(RX_Buffer, 0, 256);   /*从地址0读256Byte*/
//
//	printf("\r\n 从地址0读256Byte start \r\n");
//	for(y=0; y<256; y++)
//	{
//		//printf("\r\n The RX_Buffer[%d] is 0x%x \r\n", y, RX_Buffer[y]);
//		printf("%X ",RX_Buffer[y]);
//	}
//	printf("\r\n 从地址0读256Byte end \r\n");
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
