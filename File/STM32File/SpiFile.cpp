#include <string.h>

#include "SpiFile.h"
#include "stm32f10x_conf.h"

//check developer branch


// ID
#define STANDARD_ID 0XEF14

// Comand
#define WriteReg1            0x01  // Write status register
#define WriteReg2            0x31
#define WriteReg3            0x11
#define WritePage            0x02  // Request to write the flash
#define ReadData             0x03  // Request to read the flash
#define DisableWrite         0x04  // Write protect
#define ReadReg1             0x05  // Read status register
#define ReadReg2             0x35
#define ReadReg3             0x15
#define EnableWrite          0x06  // Allow write flash
#define EraseSector          0x20  // Erase data of one sector
#define ReadID               0x90  // Get the manufact device ID
#define Enter4ByteAddrMode   0xB7  // 4Byte mode allows 32M address
#define Exit4ByteAddrMode    0xE9  // 3Byte mode only allows 24M address


// Configuration
#define FILE_PORT      GPIOA
#define SCK_PIN        GPIO_Pin_5
#define MISO_PIN	   GPIO_Pin_6
#define MOSI_PIN	   GPIO_Pin_7
#define FILE_SPI	   SPI1

#define CS_PORT    GPIOD         // These two defines is used to
#define CS_PIN     GPIO_Pin_2    // select the target device

// Other defines
#define LOWEST_ADDRESS       4    // 4 byte to store the writeCursor pos
#define MAX_FILE_NAME_LEN    50
#define PAGE_SIZE            256
#define SECTOR_SIZE          4096
#define MAX_FLASH_ADDRESS    16 * 1024 * 1024   // 16M

class SPIFile::Private
{
public:
    Private(SPIFile *parent = 0);

    ~Private();

    int GetWritePos();

    void SetWritePos();

    void SetPermanent4ByteMode();

    void SelectDevice();

    void UnselectDevice();

    void WaitForDeviceRelease();

    void SPI_Configuration();

    void WriteEnable();

    void WriteDisable();

    void Exit4ByteMode();

    void Enter4ByteMode();

    void SafeWrite(u8 *dataBuffer , u16 bytesToWrite , bool addEndTag = true); // this function should throw some exceptions

    u16 ReadDeviceID();

    u8 ReadWriteByte(u8 data);


    // Data members
    u8 fileName[MAX_FILE_NAME_LEN];

    bool isReadWriteMode;

    u32 writeCursor;

    u32 readCursor;

private:
    uint8_t ReadRegStatus(int regNO);

    void WriteRegStatus(u8 data , int regNO);

    void ReadBuffer(u8 *dataBuffer , u16 addr , u16 bytesToRead);

    void WriteData(u8 *dataBuffer , u16 bytesToWrite);

    void WritePageByPage(u8 *dataBuffer , u16 bytesToWrite , bool mainBody = true); // This function can't avoid data recover when the bytesToWrite is larger than 256

    void SectorErasing(u16 sectorNO);

    SPIFile *q;
};

SPIFile::Private::Private(SPIFile *parent) : q(parent)
{
    isReadWriteMode = false;
    writeCursor = GetWritePos();
    readCursor = LOWEST_ADDRESS;
}

SPIFile::Private::~Private()
{

}

void SPIFile::Private::SPI_Configuration()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

    /*-----------------GPIO Settings-------------------*/
    /* SPI1_SCK SPI1_MISO SPI1_MOSI*/
    GPIO_InitStructure.GPIO_Pin = SCK_PIN | MISO_PIN | MOSI_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(FILE_PORT, &GPIO_InitStructure);

    /* FLASH_CS */
    GPIO_InitStructure.GPIO_Pin = CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(CS_PORT , &GPIO_InitStructure);
    GPIO_SetBits(CS_PORT , CS_PIN);	  /* Disable the device at first */

    /*-----------------SPI Settings--------------------*/
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7; /*the polynomial used for the CRC calculation*/
    SPI_Init(FILE_SPI, &SPI_InitStructure);

    /* Enable SPI */
    SPI_Cmd(FILE_SPI, ENABLE);
}

u8  SPIFile::Private::ReadWriteByte(u8 TxData)
{
    uint8_t RxData = 0;
    while(SPI_I2S_GetFlagStatus(FILE_SPI , SPI_I2S_FLAG_TXE) == RESET);	/* Wait until the send buffer empty */
    SPI_I2S_SendData(FILE_SPI , TxData);	               /* Send the command */

    while(SPI_I2S_GetFlagStatus(FILE_SPI , SPI_I2S_FLAG_RXNE) == RESET);	 /* Wait until the recv buffer is not empty */
    RxData = SPI_I2S_ReceiveData(FILE_SPI);	        /* read the recv buffer */

    return (uint8_t)RxData;
}

int SPIFile::Private::GetWritePos()
{
    SelectDevice();

    // Send ReadData command
    ReadWriteByte(ReadData);
    // Send address
    u32 addr = 0;
    ReadWriteByte((u8)(addr >> 16));
    ReadWriteByte((u8)(addr >> 8));
    ReadWriteByte((u8)(addr));

    int Pos = 0;
    Pos  = ReadWriteByte(0xFF) << 24;
    Pos |= ReadWriteByte(0xFF) << 16;
    Pos |= ReadWriteByte(0xFF) << 8;
    Pos |= ReadWriteByte(0xFF);

    UnselectDevice();

    return Pos;
}

void SPIFile::Private::SetWritePos()
{
    SelectDevice();

    // Send WriteData command
    ReadWriteByte(WritePage);
    // Send address
    u32 addr = 0;
    ReadWriteByte((u8)(addr >> 16));
    ReadWriteByte((u8)(addr >> 8));
    ReadWriteByte((u8)(addr));

    // Write data
    ReadWriteByte((u8)writeCursor >> 24);
    ReadWriteByte((u8)writeCursor >> 16);
    ReadWriteByte((u8)writeCursor >> 8);
    ReadWriteByte((u8)writeCursor);

    UnselectDevice();
    WaitForDeviceRelease();
}


void SPIFile::Private::SelectDevice()
{
    GPIO_ResetBits(CS_PORT , CS_PIN);
}

void SPIFile::Private::UnselectDevice()
{
    GPIO_SetBits(CS_PORT , CS_PIN);
}

uint8_t SPIFile::Private::ReadRegStatus(int regNO)
{
    SelectDevice();
    u8 comandID;
    switch(regNO)
    {
    case 1:
        comandID = ReadReg1;
        break;
    case 2:
        comandID = ReadReg2;
        break;
    case 3:
        comandID = ReadReg3;
        break;
    default:
        comandID = 0x00;
        break;
    }

    ReadWriteByte(comandID);  // Send ReadReg command
    u8 byte = ReadWriteByte(0xFF);  // Read one byte
    UnselectDevice();
    return byte;
}

void SPIFile::Private::WriteRegStatus(u8 data , int regNO)
{
    SelectDevice();
    u8 comandID;
    switch(regNO)
    {
    case 1:
        comandID = WriteReg1;
        break;
    case 2:
        comandID = WriteReg2;
        break;
    case 3:
        comandID = WriteReg3;
        break;
    default:
        comandID = 0x00;
        break;
    }
    ReadWriteByte(comandID); // Send command
    ReadWriteByte(data);
    UnselectDevice();
}

void SPIFile::Private::SetPermanent4ByteMode()
{
    u8 oldReg3Data = ReadRegStatus(3);
    u8 newReg3Data = oldReg3Data | 2;
    WriteRegStatus(newReg3Data , 3);
}

void SPIFile::Private::ReadBuffer(u8 *dataBuffer, u16 addr, u16 bytesToRead)
{
    SelectDevice();

    // Send ReadData command
    ReadWriteByte(ReadData);

    // Send address to read
    ReadWriteByte((u8)(addr >> 24));
    ReadWriteByte((u8)(addr >> 16));
    ReadWriteByte((u8)(addr >> 8));
    ReadWriteByte((u8)(addr));

    for(int i = 0 ; i < bytesToRead ; ++ i)
        dataBuffer[i] = ReadWriteByte(0xFF); // Read data
    UnselectDevice();
}

void SPIFile::Private::WaitForDeviceRelease()
{
    while((ReadRegStatus(1) & 0x01) == 0x01);
}

u16 SPIFile::Private::ReadDeviceID()
{
    u16 temp = 0;
    SelectDevice();
    ReadWriteByte(ReadID);
    ReadWriteByte(0x00);
    ReadWriteByte(0x00);
    ReadWriteByte(0x00);
    temp |= ReadWriteByte(0xFF) << 8;
    temp |= ReadWriteByte(0xFF);
    UnselectDevice();

    return temp;
}

void SPIFile::Private::WriteEnable()
{
    SelectDevice();
    ReadWriteByte(EnableWrite);
    UnselectDevice();
}

void SPIFile::Private::WriteDisable()
{
    SelectDevice();
    ReadWriteByte(DisableWrite);
    UnselectDevice();
}

// For a permanent 4byte mode , please use SetPermanent4ByteMode
void SPIFile::Private::Enter4ByteMode()
{
    SelectDevice();
    ReadWriteByte(Enter4ByteAddrMode);
    UnselectDevice();
}

void SPIFile::Private::Exit4ByteMode()
{
    SelectDevice();
    ReadWriteByte(Exit4ByteAddrMode);
    UnselectDevice();
}

// Param sectorNO is the number of the sector you wanna erase , not the address
void SPIFile::Private::SectorErasing(u16 sectorNO)
{
    u32 addr = sectorNO * SECTOR_SIZE;

    WriteEnable(); // This is a must
    WaitForDeviceRelease();  // Make sure flash is ready
    SelectDevice(); // CS open

    // Send EraseSector command
    ReadWriteByte(EraseSector);

    // Send Erase address
    ReadWriteByte((u8)(addr >> 24));
    ReadWriteByte((u8)(addr >> 16));
    ReadWriteByte((u8)(addr >> 8));
    ReadWriteByte((u8)addr);

    UnselectDevice(); // CS close
    WaitForDeviceRelease(); // wait operation ends
}

/* While using WritePage(0x02) instruction , mostly 256 bytes data can be handled at a time */
void SPIFile::Private::WriteData(u8 *dataBuffer, u16 bytesToWrite)
{
    if(!isReadWriteMode)
        return;

    WriteEnable();  // We have to do this , or writing would fail
    SelectDevice();

    ReadWriteByte(WritePage); // Send WritePage command

    // Send data address
    ReadWriteByte((u8)(writeCursor >> 24));
    ReadWriteByte((u8)(writeCursor >> 16));
    ReadWriteByte((u8)(writeCursor >> 8));
    ReadWriteByte((u8)(writeCursor));

    for(int i = 0 ; i < bytesToWrite ; ++ i)
        ReadWriteByte(dataBuffer[i]);  // Write file content

    UnselectDevice();
    WaitForDeviceRelease();   // Wait for writing progress ends
}

/* Avoid data recovering when the bytesToWrite is larger than 256 bytes */
void SPIFile::Private::WritePageByPage(u8 *dataBuffer, u16 bytesToWrite, bool mainBody)
{
    u16 pageOffset = 0;
    u8 hasEndTag = mainBody ? 1 : 0;

    while(bytesToWrite > 0)
    {
        pageOffset = writeCursor % PAGE_SIZE;
        if(bytesToWrite <= pageOffset || (pageOffset == 0 && bytesToWrite + hasEndTag <= PAGE_SIZE))
        {
            WriteData(dataBuffer , bytesToWrite);
            writeCursor += bytesToWrite;

            if(mainBody)
            {
                u8 eof = 0xFF;
                WriteData(&eof , 1); // Add an EOF
                ++ writeCursor;
            }
            bytesToWrite = 0;
        }
        else
        {
            if(pageOffset == 0)
                pageOffset = PAGE_SIZE;
            WriteData(dataBuffer , pageOffset);
            dataBuffer += pageOffset;
            writeCursor += pageOffset;
            bytesToWrite -= pageOffset;
        }
    }
}

/* This function can handle more situation */
void SPIFile::Private::SafeWrite(u8 *dataBuffer, u16 bytesToWrite, bool addEndTag)
{
    if(!isReadWriteMode)
        return;

    while(bytesToWrite > 0)
    {
        /* The file cursor reaches the end , move it to the begin */
        if(writeCursor == MAX_FLASH_ADDRESS)
            writeCursor = LOWEST_ADDRESS;

        u8 sectorNO = writeCursor / SECTOR_SIZE;
        u16 sectorOffSet = writeCursor % SECTOR_SIZE;
        u16 sectorRemain = SECTOR_SIZE - sectorOffSet;

        if(bytesToWrite <= sectorRemain || sectorOffSet == 0)
            sectorRemain = bytesToWrite;

        u8 sectorBuffer[SECTOR_SIZE]; // For reading the whole sector
        ReadBuffer(sectorBuffer , sectorNO * SECTOR_SIZE , SECTOR_SIZE);

        // Data checking
        u16 i = 0;
        for( ; i < sectorRemain ; ++ i)
        {
            if(sectorBuffer[sectorOffSet + i] != 0xFF)
                break;
        }
        if(i < sectorRemain) // Needs erasing
        {
            SectorErasing(sectorNO);
            for(i = 0 ; i < sectorRemain ; ++ i)
                sectorBuffer[sectorOffSet + i] = dataBuffer[i];
            writeCursor = sectorNO * SECTOR_SIZE;  // Set the SEEK_SET to the beginnig of this sector
        }

        WritePageByPage(dataBuffer , sectorRemain , addEndTag);
        bytesToWrite -= sectorRemain;
        dataBuffer += sectorRemain;
    }
    SetWritePos();
}

SPIFile::SPIFile(u8 *fileName, OpenMode openMode)
{
    d = new Private(this);

    Init();
    if(fileName != NULL)
        strcpy((char *)d->fileName , (char *)fileName);
    if(openMode == modeReadWrite)
        d->isReadWriteMode = true;
   // d->SetPermanent4ByteMode(); // Enter 4 Byte Address Mode , take care of all your address
    d->Enter4ByteMode();
}

bool SPIFile::Init()
{
    d->SPI_Configuration();
    return d->ReadDeviceID() == STANDARD_ID;
}

void SPIFile::Write(u8 *writeBuffer, u16 bytesToWrite)
{
    d->SafeWrite(d->fileName , strlen((char *)d->fileName) , false);
    d->SafeWrite(writeBuffer , bytesToWrite);
}

void SPIFile::Read(u8 *readBuffer, u16 readLen , u32 addr)
{
    d->SelectDevice();
    d->ReadWriteByte(ReadData);

    d->ReadWriteByte((u8)(addr >> 24));
    d->ReadWriteByte((u8)(addr >> 16));
    d->ReadWriteByte((u8)(addr >> 8));
    d->ReadWriteByte((u8)(addr));

    for(int i = 0 ; i < readLen ; ++ i)
    {
        readBuffer[i] = d->ReadWriteByte(0xFF);
    }

    d->UnselectDevice();
}

//
u16 SPIFile::ReadNextFile(u8 *recvBuffer)
{
    d->SelectDevice();

    // Send ReadData command
    d->ReadWriteByte(ReadData);
    // Send readCursor pos
    d->ReadWriteByte((u8)(d->readCursor >> 24));
    d->ReadWriteByte((u8)(d->readCursor >> 16));
    d->ReadWriteByte((u8)(d->readCursor >> 8));
    d->ReadWriteByte((u8)(d->readCursor));

    u8 recvByte;
    u16 fileLen = 0;
    bool singleFF = false;

    do
    {
        recvByte = d->ReadWriteByte(0xFF);
        if(recvByte == 0xFF)
            singleFF = true;
        if(++ d->readCursor > MAX_FLASH_ADDRESS)
            d->readCursor = LOWEST_ADDRESS;
        if(recvByte == 0xFE)
        {
            recvByte = d->ReadWriteByte(0xFF);
            if(++ d->readCursor > MAX_FLASH_ADDRESS)
                d->readCursor = LOWEST_ADDRESS;
            if(recvByte == 0x01)
                recvByte = 0xFF;
            if(recvByte == 0x00)
                recvByte = 0xFE;
        }
        if(!singleFF)
        {
            recvBuffer[fileLen] = recvByte;
            ++ fileLen;
        }
    }while(!singleFF);

    d->UnselectDevice();
    return fileLen;
}


void SPIFile::SetFileName(u8 *fileName)
{
    strcpy((char *)d->fileName , "0");
    strcpy((char *)d->fileName , (char *)fileName);
}


void SPIFile::SetReadWriteMode(OpenMode openMode)
{
    if(openMode == SPIFile::modeReadWrite)
        d->isReadWriteMode = true;
    if(openMode == SPIFile::modeReadOnly)
        d->isReadWriteMode = false;
}
