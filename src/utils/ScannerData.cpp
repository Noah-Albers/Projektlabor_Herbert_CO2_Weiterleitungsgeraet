#include "ScannerData.h"
#include "Debugging.h"
#include <Arduino.h>

void ScannerData::update(int* co2,float* humidity,float* temperature,float* light){
    this->co2=*co2;
    this->humidity=*humidity;
    this->temperature=*temperature;
    this->light=*light;
    this->validData=true;
};

bool ScannerData::hasDataArrived(){
    return this->validData;
}