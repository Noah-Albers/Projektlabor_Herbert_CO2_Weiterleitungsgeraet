#pragma once
#include "states/State.h"
#include <U8x8lib.h>

extern long requestStartTime;

extern State* currentState;
extern U8X8_SSD1306_128X64_NONAME_SW_I2C display;

void ensureWifiConnection();
void openState(State* newState);
void sendPacketToState(byte* packet,int pktLen);
void onFailedStartup();