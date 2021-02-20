#include <Arduino.h>
#include "Debugging.h"

namespace debug{

  String hexDump(byte* data,int len) {
    String fin="";

    for (int i = 0; i < len; i++)
      fin+=String((char)data[i], HEX) + " ";

    return fin;
  }
  String hexDump(String packet) {
    String fin="";
    for (int i = 0; i < packet.length(); i++)
      fin+=String(packet[i], HEX) + " ";
    
    return fin;
  }

  void info(String data){
    Serial.println("[INFO] ("+String(millis())+") "+data);
  }
  void debug(String data){
    //Serial.println("\t\t[DEBUG] "+data);
  }
  void error(String data){
    Serial.println("[ERROR] "+data);
  }
  void warning(String data){
    Serial.println("[WARNING] "+data);
  }
}
