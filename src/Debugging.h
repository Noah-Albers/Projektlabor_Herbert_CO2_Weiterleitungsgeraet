#pragma once
#include <Arduino.h>

namespace debug{
    String hexDump(byte* data,int len);
    String hexDump(String packet);

    void info(String data);
    void debug(String data);
    void error(String data);
    void warning(String data);
}