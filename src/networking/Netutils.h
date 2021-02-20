#pragma once
#include <Arduino.h>

// Converter for four-byte data-types
typedef union {
  float floatValue;
  byte byteValue[4];
  int intValue;
} Bitv4;

namespace netutils{

  const int PACKET_SCANNER_DATA = 1;
  const int PACKET_REQUEST_DATA = 0;

  struct PacketInfo{
    byte id;
    int length;
  };

  // Holds all infos about all registered packets
  extern PacketInfo registeredPackets[];

  // General converter
  extern Bitv4 converter;

  int getPacketLengthById(byte id);

  Bitv4 getFromPacket(byte* packet,int position);

  void appendFloat(byte* packet,int position,float value);

  void appendInt(byte* packet,int position, int value);

  bool getNextPacket(byte* packet,int len);

  void sendPacket(byte data[],byte packetId,int receiverID);

  // Checks if a received packet is valid
  bool isPacketValid(byte packet[], int len);

  // Hashes the given array of bytes (Packet) with the sha256 and places those 32 bytes before the array
  void hashPacket(byte input[],byte result[],int inputLength);
}