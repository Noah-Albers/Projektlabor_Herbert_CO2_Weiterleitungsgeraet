#include "State.h"
#include <Arduino.h>
#include <WebServer.h>

State::~State(){}
void State::onTick(){}
void State::onInit(){}
bool State::isLoRaNeeded(){return false;}
void State::onLoRaPacketReceive(byte id,int sender,byte* data){}