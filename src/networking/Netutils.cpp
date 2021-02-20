#include <Arduino.h>
#include "mbedtls/md.h"
#include "Netutils.h"
#include "Debugging.h"
#include <LoRa.h>
#include "UserConfig.h"


namespace netutils{

  PacketInfo registeredPackets[] = {
    {
      // Request-data packet
      PACKET_REQUEST_DATA,
      0  // Holds no extern data
    },{
      // Sensor-data packet
      PACKET_SCANNER_DATA,
      4*4 // Holds 4 4-byte values
    }
  };

  // General converter
  Bitv4 converter;

  Bitv4 getFromPacket(byte* packet,int position) {
    for(int i=0;i<4;i++)
      converter.byteValue[i]=packet[position+i];
    return converter;
  }

  int getPacketLengthById(byte id){
    // Searches the packet infors for that id
    for(int i=0;i<sizeof(registeredPackets)/sizeof(PacketInfo);i++){
      // Gets the packet info
      PacketInfo* p = &registeredPackets[i];

      // Checks if the id matches
      if(p->id==id)
        return p->length;
    }

    return -1;
  }

  void appendFloat(byte* packet,int position,float value){
    converter.floatValue=value;

    for(int i=0;i<4;i++)
      packet[position+i]=converter.byteValue[i];
  }
  
  void appendInt(byte* packet,int position, int value){
    converter.intValue=value;

    for(int i=0;i<4;i++)
      packet[position+i]=converter.byteValue[i];
  }

bool getNextPacket(byte* packet,int len) {
  // Reads in the packet
  for (int i = 0; i < len; i++)
    packet[i] = LoRa.read();

  // Waits
  delay(10);

  debug::debug("Hex-Dump:");
  debug::debug(debug::hexDump(packet,len));

  // Checks if the length is invalid
  // Checksum + Packetid + Senderid + receiverid
  if (len < 32 + 1 + 4 + 4) {
    debug::error("Received invalid Packet (Length is to short): "+String(len)+" bytes of length");
    return false;
  }

  // Gets the receiverId
  int receiverId = netutils::getFromPacket(packet,32+1).intValue;

  // Checks if the ids don't match up
  if (receiverId != DEVICE_ID) {
    debug::debug("Received packet for different device " + String(receiverId) + " ,but my device id is: " + String(DEVICE_ID));
    return false;
  }

  // Checks if the packet is valid
  if (!netutils::isPacketValid(packet, len)) {
    debug::error("Received invalid Packet (Sha256 was invalid).");
    return false;
  }

  debug::debug("Received packet is valid.");

  // Gets the packet-info
  int pktLength = getPacketLengthById(packet[32]);

  // Checks if no packet-info for that packet couln't be found
  if(pktLength == -1){
    debug::error("Could not find length for packet id: "+String(packet[32]));
    return false;
  }

  debug::debug("Packet found for id "+String(packet[32]));

  // Checks if the length matches
  if(len-32-1-4-4==pktLength){
    debug::debug("Packet is valid from length: "+String(pktLength));
    // Valid packet received
    return true;
  }else{
    debug::error("Packet length is invalid, received "+String(len-32-1-4-4)+" != length of "+String(pktLength));
    // Invalid packet
    return false;
  }
}

  void sendPacket(byte data[],byte packetId,int receiverID){
    // Gets the packet length
    int pktLen = getPacketLengthById(packetId);

    // Creates the packet array
    byte packet[1+4+4+pktLen];
    // Appends the id
    packet[0] = packetId;
    // Appends receiver
    appendInt(packet,1,receiverID);
    // Appends the sender
    appendInt(packet,5,DEVICE_ID);

    // Copies the data
    for(int i=0;i<pktLen;i++)
      packet[9+i]=data[i];
    
    // Final packet
    byte finalPacket[sizeof(packet)+32];

    // Hashes the packet
    hashPacket(packet,finalPacket,sizeof(packet));

    debug::debug("Hex dump of sending packet:");
    debug::debug(debug::hexDump(finalPacket,sizeof(finalPacket)));

    // Sends the packet
    LoRa.beginPacket();
    LoRa.setTxPower(14,0x80);
    for(int i=0;i<sizeof(finalPacket);i++)
      LoRa.write(finalPacket[i]);
    LoRa.endPacket();
  }

  // Checks if a received packet is valid
  bool isPacketValid(byte packet[], int len) {
    // Seperates the data from the sha-hash of the packet
    byte data[len - 32];
    for (int i = 0; i < len - 32; i++)
      data[i] = packet[i + 32];

    // Holds the hash-result as bytes
    byte result[32];

    // Hash-Magic
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *) data, len - 32);
    mbedtls_md_finish(&ctx, result);
    mbedtls_md_free(&ctx);

    // Iterates over the hash and the result
    for (int i = 0; i < 32; i++) {
      // Checks if the generates hash and received hash match up
      if (result[i] != packet[i])
        return false;
    }

    // Both hashes match up
    return true;
  }

  void hashPacket(byte input[],byte result[],int inputLength){
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *) input, inputLength);
    mbedtls_md_finish(&ctx, result);
    mbedtls_md_free(&ctx);

    // Appends the input-data
    for(int i=0;i<inputLength;i++)
      result[i+32]=input[i];

  }

}