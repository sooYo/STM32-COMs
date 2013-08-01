#pragma once

//#include "stm32f10x_conf.h"
#include "w25q16.h"

/*  W25Q256 flash memery space feature  */
/*  There are totally 32M storage
    256 Bytes as 1 page , totally 131072 pages
    16 pages as 1 sector , totally 8192 secotrs , and 4k space for each sector
    256 pages as 1 block , 16 sectors as 1 block , totally 512 blocks ,and 64k space for each block
*/


typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

class SPIFile
{
public:
    typedef enum
    {
        modeReadOnly = 1,
        modeReadWrite
    }OpenMode;

    SPIFile(u8 *fileName = NULL , OpenMode openMode = modeReadOnly);

    ~SPIFile();

    void SetFileName(u8 *fileName);

    void SetReadWriteMode(OpenMode openMode);

    /* The two functions right below is now on testing , please don't use it in any formal function */
    void Write(u8 *writeBuffer , u32 bytesToWrite);
    void ReadNextFile(u8 *readBuffer);
   /* */

    // This function is used to read the data on spi flash
    // @readBuffer      buffer for recving data
    // @addr            the begin address of the data on spi flash
    // @bytesToRead     how long do you want to read
    void Read(u8 *readBuffer , u32 addr , u32 bytesToRead);

    // This function is used to write data to spi flash
    // @writeBuffe      data buffer that you want to write to the spi falsh
    // @addr            the begin address on spi flash you're going to write
    // @bytesToWrite    the lenght of the data buffer
    void Write(u8 *writeBuffer,  u32 addr , u32 bytesToWrite);

protected:
    // Do configuration here
    virtual bool Init();

private:
    class Private;
    Private *d;
};


