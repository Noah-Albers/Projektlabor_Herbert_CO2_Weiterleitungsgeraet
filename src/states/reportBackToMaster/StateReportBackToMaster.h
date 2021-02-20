#pragma once
#include "states/State.h"
#include "utils/ScannerData.h"

class StateReportBackToMaster : public State{
    public:
        StateReportBackToMaster(ScannerData* receivedData);
        void onTick();
};