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
    CSData() : memory(false), headLength(0), dataLength(0), bufferLength(0){};
    ~CSData()
    {
        this->free();
    };
    void free()
    {
        if (this->memory)
        {
            delete[] this->buffer;
            this->memory = false;
        }

        this->headLength = 0;
        this->dataLength = 0;
        this->bufferLength = 0;
    };

    // Getters & Setters;
    char *getHeader()
    {
        return this->headLength ? (char *)this->buffer : NULL;
    }
    uint8_t getHeaderLength()
    {
        return this->headLength;
    }
    byte *getData()
    {
        return this->dataLength ? this->buffer + this->headLength : NULL;
    }
    uint8_t getDataLength()
    {
        return this->dataLength;
    }
    byte *getBuffer()
    {
        return this->bufferLength ? this->buffer : NULL;
    }
    uint16_t getBufferLength()
    {
        return this->bufferLength;
    }

    // Oparation Functions;
    bool serialize(char *header, byte *data, uint8_t length)
    {
        this->free();

        uint8_t headerLength = strlen(header) + 1;

        this->bufferLength = 4 + headerLength + length;
        this->buffer = new byte[this->bufferLength];
        this->memory = true;

        this->buffer[0] = 0x01;
        this->buffer[1] = headerLength;
        this->buffer[2] = length;
        this->buffer[3] = 0x16;

        strcpy((char *)(this->buffer + 4), header);
        memcpy((this->buffer + 4 + headerLength), data, length);

        return true;
    }
    bool deserialize(byte *ctrl, byte *data)
    {
        this->free();

        if (ctrl[0] != 0x01 || ctrl[1] == 0 || ctrl[3] != 0x16)
        {
            return false;
        }

        if (data == NULL)
        {
            this->buffer = new byte[ctrl[1] + ctrl[2]];
            this->memory = true;
        }
        else
        {
            this->buffer = data;
        }

        this->headLength = ctrl[1];
        this->dataLength = ctrl[2];

        return true;
    }

    // Additional Functions;
    bool deserialize(HardwareSerial *serial)
    {
        if (!this->headLength && serial->available() > 5)
        {
            byte headerInfo[4] = {0x00, 0x00, 0x00, 0x00};
            serial->readBytes(headerInfo, 1);

            while (serial->available())
            {

                if (headerInfo[0] == 0x01)
                { // 1st Check Pass;

                    serial->readBytes(headerInfo + 1, 1);
                    if (headerInfo[1] > 1 && serial->available() > 1)
                    { // 2nd Check Pass;

                        serial->readBytes(headerInfo + 2, 2);
                        if (headerInfo[3] == 0x16)
                        { // 3rd & Finale Check Pass;
                            this->free();
                            this->memory = true;
                            this->headLength = headerInfo[1];
                            this->dataLength = headerInfo[2];
                            this->buffer = new byte[headerInfo[1] + headerInfo[2]];
                            break;
                        }

                        headerInfo[0] = headerInfo[2];
                        headerInfo[1] = headerInfo[3];
                        continue;
                    }
                    else if (headerInfo[1] == 0x01)
                    {
                        headerInfo[0] = headerInfo[1];
                        continue;
                    }
                }

                return false;
            }
        }

        uint16_t requiredBuffer = (this->headLength + this->dataLength) - this->bufferLength;
        if (requiredBuffer == 0)
        {
            return true;
        }

        uint16_t availableBuffer = serial->available();
        if (availableBuffer >= requiredBuffer)
        {
            serial->readBytes(this->buffer + this->bufferLength, requiredBuffer);
            this->bufferLength += requiredBuffer;
            return true;
        }

        if (availableBuffer > 0)
        {
            serial->readBytes(this->buffer + this->bufferLength, availableBuffer);
            this->bufferLength += availableBuffer;
        }

        return false;
    }
};
