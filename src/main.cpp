#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include "networking/Netutils.h"
#include "Debugging.h"
#include <WiFi.h>
#include "main.h"
#include "UserConfig.h"
#include "Config.h"
#include "states/State.h"
#include "states/requestData/StateRequestData.h"
#include <U8x8lib.h>

// Display on the esp
U8X8_SSD1306_128X64_NONAME_SW_I2C display(15, 4, 16);

// Currently open program state (subprogram)
State* currentState;

// Holds the time when a new request-round starts
long requestStartTime;

void ensureWifiConnection() {
  // Checks if the wifi is connected
  if(WiFi.status() == WL_CONNECTED)
    return;

  // Waits until the wifi status is connected
  while (WiFi.status() != WL_CONNECTED) {
    display.clearLine(3);
    display.setCursor(1,3);
    display.print("Wlan...."+WiFi.status());

    debug::debug("WiFi not connected with stats "+String(WiFi.status()));

    // Checks if static config should be used and if the configuration failed
    if(!IP_USE_DHCP && !WiFi.config(IP_STATIC_IP,IP_STATIC_GATEWAY,IP_STATIC_SUBNETMASK,IP_STATIC_DNS)){
        debug::warning("WIFI-Config failed.");
        continue;
    }

    // Tries to connect to the wifi
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
    // Waits to establish a connection
    delay(WIFI_CONNECTION_DELAY);
  }
  
  display.clearLine(3);
  display.setCursor(1,3);
  display.print(WiFi.localIP().toString());
  debug::info("(Re)connected wifi with ip: "+WiFi.localIP().toString());
}

void openState(State* newState){
  // Ensures an open wifi connection
  ensureWifiConnection();

  // Checks if the old state exists
  if(currentState)
    // Kills the old state
    delete currentState;
  
  // Opens the new state
  currentState=newState;

  // Informs the state, that all preperations have been done
  currentState->onInit();
}

void sendPacketToState(byte* packet,int pktLen){
  // Gets all infos about the packet
  byte id = packet[31+1];
  int sender = netutils::getFromPacket(packet,32+1+4).intValue;

  // Seperates the data
  byte data[pktLen-32-4-4-1];

  // Copies the data
  for(int i=0;i<pktLen-32-4-4-1; i++)
    data[i]=packet[i+32+4+4+1];

  // Sends the packet to the current state
  currentState->onLoRaPacketReceive(id,sender,data);
}

void onFailedStartup(){
  // Goes into an infinity loop (Waits until the user restarts the device)
  while(1);
}

void setup() {

  // Starts the serial connection, the spi and lora
  Serial.begin(115200);  
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(18, 14, 16);

  // Starts the display
  display.begin();
  display.setFont(u8x8_font_chroma48medium8_r);
  display.clearDisplay();
  display.setCursor(1,1);
  
  // Checks that lora can start
  if(!LoRa.begin(LORA_FREQUENCY)){
    // Displays the error to the user
    display.print("Lora star failed.");
    display.setCursor(1,2);
    display.print("Please restart.");

    debug::error("Failed to start lora. Please try again.");
    onFailedStartup();
  }

  // Connects to the wifi
  ensureWifiConnection();

  // Starts the beginning state
  openState(new StateRequestData());

  display.setCursor(1,1);
  // Displays that everything went correct
  display.print("Middle node started.");
  display.setCursor(1,2);
  display.print("ID: "+String(DEVICE_ID));
}

void loop() {
  // Ensures an open wifi connection
  ensureWifiConnection();

  // Checks if lora is used by the state
  if(currentState->isLoRaNeeded()){
    // Gets the next lora packet
    int len = LoRa.parsePacket();

    // Checks if a packet is available
    if(len > 0){
      // The packet
      byte packet[len];

      // Processes the packet
      if(netutils::getNextPacket(packet,len)){
        sendPacketToState(packet,len);
        return;
      }
    }
  }

  // Runs the tick
  currentState->onTick();
}
