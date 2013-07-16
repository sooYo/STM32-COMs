#pragma once

#define LOCK_PORT      GPIOE
#define LOCK           GPIO_Pin_2
#define UNLOCK         GPIO_Pin_3
#define BIKE_OK        GPIO_Pin_4
#define LOCK_OK        GPIO_Pin_5
#define UNLOCK_OK      GPIO_Pin_6

class LockCtrler
{
public:
    LockCtrler();

    ~LockCtrler();

    // a bike is coming in
    bool BikeNear();

    // use the function right next to it to check if it really work
    void TryToOpenLock();

    bool IsLockOpened();

    // use the function right next to it to check if it really work
    void TryToCloseLock();

    bool IsLockClosed();

protected:
    void Init();

private:

};

