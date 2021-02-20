#pragma once

class ScannerData{
    private:
        bool validData = false;
    public:
        int co2 = 0; // In ppm
        float humidity = 0; // In percentage %
        float temperature = 0; // In degrees
        float light=0; // In percentage

        void update(int* co2,float* humidity,float* temperature,float* light);
        bool hasDataArrived();
};