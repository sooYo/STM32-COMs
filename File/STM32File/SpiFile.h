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

    void SetFileName(u8 *fileName);

    void SetReadWriteMode(OpenMode openMode);

    // @writeBuffer     the data to write
    // @bytesToWrite    length of the data
    /* we can't get the amount of data that has been written */
    void Write(u8 *writeBuffer , u16 bytesToWrite);

    // @recvBuffer    buffer for receiving data
    // @Retval        the actual length of this file
    u16 ReadNextFile(u8 *recvBuffer);

    // This function is for testing only , for reading pleas use ReadNextFile
    void Read(u8 *readBuffer , u16 bytesToRead , u32 addr);


    void Write(u8 *writeBuffer, u16 bytesToWrite , u32 addr);

protected:
    // Do configuration here
    virtual bool Init();

private:
    class Private;
    Private *d;
};


