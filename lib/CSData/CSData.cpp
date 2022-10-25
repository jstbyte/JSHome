#include "CSData.h"

CSData::CSData() : memory(false), headLength(0), dataLength(0), bufferLength(0){};

CSData::~CSData()
{
    this->free();
}

void CSData::free()
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

char *CSData::getHeader()
{
    return this->headLength ? (char *)this->buffer : NULL;
}

uint8_t CSData::getHeaderLength()
{
    return this->headLength;
}

byte *CSData::getData()
{
    return this->dataLength ? this->buffer + this->headLength : NULL;
}

uint8_t CSData::getDataLength()
{
    return this->dataLength;
}

byte *CSData::getBuffer()
{
    return this->bufferLength ? this->buffer : NULL;
}

uint16_t CSData::getBufferLength()
{
    return this->bufferLength;
}

bool CSData::serialize(char *header, byte *data, uint8_t length)
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

bool CSData::deserialize(byte *ctrl, byte *data)
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

bool CSData::deserialize(HardwareSerial *serial)
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
