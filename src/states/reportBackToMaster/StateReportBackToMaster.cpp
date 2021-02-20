#include "StateReportBackToMaster.h"
#include "main.h"
#include "states/requestData/StateRequestData.h"
#include <base64.h>
#include <cJSON.h>
#include "Debugging.h"
#include <HTTPClient.h>
#include "UserConfig.h"
#include "Config.h"

StateReportBackToMaster::StateReportBackToMaster(ScannerData* receivedData){

    // The json objects
    cJSON* root;

    // Creates the root
    root = cJSON_CreateObject();

    Serial.println("\n\n\n");
    for(int i=0;i<KNOWN_DEVICES_AMOUNT;i++){
        // Gets the reference to the scannerdata
        ScannerData* sd = &receivedData[i];
        
        // Gets the device name
        String sName = String(KNOWN_DEVICES[i]);
        char deviceName[sName.length()];
        sName.toCharArray(deviceName, sName.length()+1);

        // Checks if data got received for that device
        if(sd->hasDataArrived()){
            // Creates the json object
            cJSON* object = cJSON_CreateObject();

            // Append the object to the root
            cJSON_AddItemToObject(root,deviceName,object);

            // Appends the data to the object
            cJSON_AddNumberToObject(object,"co2",sd->co2);
            cJSON_AddNumberToObject(object,"humidity",sd->humidity);
            cJSON_AddNumberToObject(object,"light",sd->light);
            cJSON_AddNumberToObject(object,"temperature",sd->temperature);
        }else
            // Appends false to the object
            cJSON_AddItemToObject(root,deviceName,cJSON_CreateFalse());
    }

    // Removes the received data array
    delete[] receivedData;

    // Holds the json raw string as char pointer
    char* rawJsonPtr = cJSON_Print(root);
    // Gets the json string as a arduino string
    String rawJson = String(rawJsonPtr);
    // Frees the raw json ptr
    free(rawJsonPtr);

    // Encodes the string as b64
    String b64Json = base64::encode(rawJson);

    // Outputs the received data
    debug::debug("Received the following information: ");
    debug::debug(rawJson);
    debug::debug("Base64 string: ");
    debug::debug(b64Json);

    // Starts a http connection to the master server
    HTTPClient con;
    con.begin("http://"+String(MASTER_SERVER)+"/post/"+b64Json);

    // Posts the data
    int httpCode = con.GET();

    // Checks the code 
    if (httpCode == 200 && con.getString().equals("post.success"))
        // Debug info
        debug::debug("Connection was successful.");
    else
        debug::debug("Error making http request to "+String(MASTER_SERVER)+" status code "+String(httpCode));
 
    con.end(); //Free the resources
}

void StateReportBackToMaster::onTick(){

    // Waits until the given minimum time between request-rounds has been reached
    while(millis()-requestStartTime < REQUEST_ROUND_DELAY)
        delay(2000);

    // Gets a random delay to wait
    // Used to desync the middle nodes
    long delayTime = random(MIN_RANDOM_DELAY,MAX_RANDOM_DELAY);

    debug::info("Waiting: "+String(delayTime)+"ms.");

    // Waits the time
    delay(delayTime);
    // Opens the next state
    openState(new StateRequestData());
}