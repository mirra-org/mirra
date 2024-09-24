#include "CommunicationCommon.h"

using namespace mirra::comm;

MACAddress::MACAddress(const char* string)
{
    sscanf(string, "%02X:%02X:%02X:%02X:%02X:%02X", &address[0], &address[1], &address[2],
           &address[3], &address[4], &address[5]);
    ;
}

char* MACAddress::toString(char* string) const
{
    snprintf(string, MACAddress::stringLength, "%02X:%02X:%02X:%02X:%02X:%02X", this->address[0],
             this->address[1], this->address[2], this->address[3], this->address[4],
             this->address[5]);
    return string;
}

char* MACAddress::toStringRaw(char* string) const
{
    snprintf(string, MACAddress::rawStringLength, "%02X%02X%02X%02X%02X%02X", this->address[0],
             this->address[1], this->address[2], this->address[3], this->address[4],
             this->address[5]);
    return string;
}

char MACAddress::strBuffer[MACAddress::stringLength];

Address::Address(const char* string)
{
    sscanf(string, "%04X:%04X", &gateway, &node);
};

char* Address::toString(char* string) const
{
    snprintf(string, Address::stringLength, "%04X:%02X", gateway, node);
    return string;
}

char Address::strBuffer[Address::stringLength];