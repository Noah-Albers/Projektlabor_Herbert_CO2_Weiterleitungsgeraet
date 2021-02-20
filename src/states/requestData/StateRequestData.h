#pragma once
#include "states/State.h"
#include "utils/ScannerData.h"

class StateRequestData : public State{
    private:
        // Holds the array with the scanner-data for all devices
        ScannerData* receivedData;

        // Current decive index (Starts with -1 to be able to increase at start)
        int currentDeviceIndex=-1;

        // Last reqeust time
        long lastRequestTime=0;

        // Amount of requests have been send for a single device this for
        int sendPacketCount=0;

        // Sends the request to the device
        void sendRequestToCurrentDevice();
        void startWithNextDevice();
    public:
        void onInit();
        void onTick();
        bool isLoRaNeeded();
        void onLoRaPacketReceive(byte id,int sender,byte* data);
};