#include "stm32f10x.h"
#include "LockCtrler.h"
#include "stdio.h"

LockCtrler::LockCtrler()
{
    Init();
}

LockCtrler::~LockCtrler()
{

}

void LockCtrler::Init()
{
    GPIO_InitTypeDef  initStruct;

    // init PE2 & PE3 , Lock & Unlock , Mode out
    initStruct.GPIO_Pin = LOCK | UNLOCK;
    initStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LOCK_PORT , &initStruct);

    // init PE4 & PE5 & PE6 , Bike_Ok & Lock_Ok & Unlock_Ok , Mode in
    initStruct.GPIO_Pin = BIKE_OK | LOCK_OK | UNLOCK_OK;
    initStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LOCK_PORT , &initStruct);

    // Set lock & unlock to low level , coz they were initialized with up level
    GPIO_ResetBits(LOCK_PORT , LOCK);
    GPIO_ResetBits(LOCK_PORT , UNLOCK);
}

bool LockCtrler::BikeNear()
{
    // When there is a bike coming in , BIKE_OK's IDR would be Bit_RESET
    if(GPIO_ReadInputDataBit(LOCK_PORT , BIKE_OK) == Bit_RESET)
        return true;
    return false;
}

void LockCtrler::TryToOpenLock()
{
    GPIO_SetBits(LOCK_PORT , UNLOCK);
}

void LockCtrler::TryToCloseLock()
{
    GPIO_SetBits(LOCK_PORT , LOCK);
}

bool LockCtrler::IsLockOpened()
{
    // The IDR should be Bit_RESET when it is OK
    u8 ret =  GPIO_ReadOutputDataBit(LOCK_PORT , UNLOCK_OK);
    GPIO_ResetBits(LOCK_PORT , UNLOCK);
    return ret == Bit_RESET;
}

bool LockCtrler::IsLockClosed()
{
    // The IDR should be Bit_RESET when it is OK
    u8 ret = GPIO_ReadOutputDataBit(LOCK_PORT , LOCK_OK);
    GPIO_ResetBits(LOCK_PORT , LOCK);
    return ret == Bit_RESET;
}





