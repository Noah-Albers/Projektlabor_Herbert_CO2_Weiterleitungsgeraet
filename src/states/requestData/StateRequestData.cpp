#include "StateRequestData.h"
#include <Arduino.h>
#include "networking/Netutils.h"
#include "main.h"
#include "states/reportBackToMaster/StateReportBackToMaster.h"
#include "Debugging.h"
#include "UserConfig.h"
#include "Config.h"

void StateRequestData::startWithNextDevice(){
    // Updates to the next device index
    this->currentDeviceIndex++;

    // Checks if any more devices must be requested
    if(this->currentDeviceIndex>=KNOWN_DEVICES_AMOUNT){
        debug::info("All devices have been requested... opening next state");

        // Opens the state to report all data back to the master server
        openState(new StateReportBackToMaster(this->receivedData));
        return;
    }

    debug::info("[New Request] to: "+String(KNOWN_DEVICES[this->currentDeviceIndex]));

    // Sends the next reqeust
    this->sendRequestToCurrentDevice();
    // Resets the time
    this->lastRequestTime=millis();
    // Resets the packet counter
    this->sendPacketCount=1;
}

void StateRequestData::onInit(){
    // Resets the global timer
    requestStartTime = millis();

    // Creates the array with the data and fill it with empty data
    this->receivedData = new ScannerData[KNOWN_DEVICES_AMOUNT];
    
    for(int i=0;i<KNOWN_DEVICES_AMOUNT;i++)
        this->receivedData[i]=ScannerData();

    // Starts the requesting
    this->startWithNextDevice();
}

void StateRequestData::sendRequestToCurrentDevice(){
    debug::info("\tLora send request ");
    // Sends the request
    byte pkt[0];
    netutils::sendPacket(pkt,netutils::PACKET_REQUEST_DATA,KNOWN_DEVICES[this->currentDeviceIndex]);
}

void StateRequestData::onTick(){
    // Checks if the packet should be resend
    if(millis()-this->lastRequestTime > DELAY_BETWEEN_PACKETS){
        // Increases the packet counter
        this->sendPacketCount++;

        // Checks if the packet counter is to high
        if(this->sendPacketCount>MAX_REQUEST_PACKET){
            this->startWithNextDevice();
            return;
        }

        // Resends the request packet
        debug::info("\tSending request number "+String(this->sendPacketCount)+" to device with "+String(KNOWN_DEVICES[this->currentDeviceIndex]));
        this->sendRequestToCurrentDevice();
        this->lastRequestTime=millis();
        return;
    }
}

void StateRequestData::onLoRaPacketReceive(byte id,int sender,byte* data){
    // Checks if the packet is not a sensor-data packet
    if(id!=netutils::PACKET_SCANNER_DATA){
        debug::debug("Waiting for sensor-data packets ("+String(netutils::PACKET_SCANNER_DATA)+") but received packet with id "+String(id)+" from "+String(sender)+".");
        return;
    }

    // Holds the index to the device
    int kdIndex = -1;

    // Searches the index of the sending device
    for(int i=0;i<KNOWN_DEVICES_AMOUNT;i++)
        if(KNOWN_DEVICES[i]==sender){
            kdIndex=i;
            break;
        }

    // Checks if the sending device is not one of the known onces
    if(kdIndex==-1){
        debug::debug("The sending device with id "+String(sender)+" is not one of my devices.");
        return;
    }

    debug::info("\nWell known device "+String(KNOWN_DEVICES[kdIndex])+" has send the following data:");

    int ppm = netutils::getFromPacket(data,0).intValue;
    float hum =netutils::getFromPacket(data,4).floatValue;
    float tmp =netutils::getFromPacket(data,8).floatValue;
    float lig = netutils::getFromPacket(data,12).floatValue;

    debug::info("\t"+String(ppm)+" ppm / "+String(hum)+"% Humidity / "+String(tmp)+"° Temperature / "+String(lig)+"% Light\n");

    // Updates the received data
    this->receivedData[kdIndex].update(&ppm,&hum,&tmp,&lig);

    // Continues with the next device
    this->startWithNextDevice();
}

bool StateRequestData::isLoRaNeeded(){
    return true;
}