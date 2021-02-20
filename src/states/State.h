#pragma once
#include <Arduino.h>

class State{
  public:
    virtual ~State();
    virtual void onInit();
    virtual void onTick();
    virtual bool isLoRaNeeded();
    virtual void onLoRaPacketReceive(byte id,int sender,byte* data);
};