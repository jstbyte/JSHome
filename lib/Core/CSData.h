#pragma once
#include <Arduino.h>

/* JSerialize & JDeSerialize Data */
class CSData
{
    /*:::::::::::::::::::::::::: RULE ::::::::::::::::::::::::::::
        Fist 4 Bytes Are Control Bytes;
        Message = [0x01][header_length][data_length][0x16][...data];
        Header Never Be Empty! require least 2 char including '\0';
        Whole Buffer length Maybe More The 256 Bytes -
        But Only Data Length Should Be Less Then 256 Bytes;
        RECOMEND: try to keep whole payload under 256 Bytes;
   */

private:
    bool memory;           // is memory allocated;
    byte *buffer;          // memory buffer;
    uint8_t headLength;    // in Byte;
    uint8_t dataLength;    // in Byte;
    uint16_t bufferLength; // in Byte;

public:
    CSData();
    ~CSData();
    void free();
    byte *getData();
    char *getHeader();
    byte *getBuffer();
    uint8_t getDataLength();
    uint8_t getHeaderLength();
    uint16_t getBufferLength();
    bool deserialize(byte *ctrl, byte *data);
    bool deserialize(HardwareSerial *serial);
    bool serialize(char *header, byte *data, uint8_t length);
};
