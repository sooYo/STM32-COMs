#ifndef	__W25Q16_H_
#define __W25Q16_H_
///#include "stm32f10x_conf.h"
/* ����ֻ�궨���˶˿� ���� ģ�飬���ʱ�������
void SPI_Configuration(void)ʹ��*/
#define W25Q16_PORT				GPIOA
#define W25Q16_SCK_PIN			GPIO_Pin_5
#define W25Q16_MISO_PIN			GPIO_Pin_6
#define W25Q16_MOSI_PIN			GPIO_Pin_7
#define W25Q16_SPI	            SPI1

/* W25Q16Ƭѡ */
#define W25Q16_CS_LOW()       GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define W25Q16_CS_HIGH()      GPIO_SetBits(GPIOB, GPIO_Pin_9)

/* W25X16ϵ��ID��0XEF14 */
#define W25Q16_ID  0XEF14

/* W25Q16ָ���	*/
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

/* Private Function*/
uint8_t W25Q16_Init(void);
/* ��״̬�Ĵ��� */
uint8_t SPI_Flash_ReadSR(void);
uint8_t SPI_Flash_WriteSR(uint16_t);
   
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);   
void SPI_Flash_Erase_Chip(void);   
#endif

