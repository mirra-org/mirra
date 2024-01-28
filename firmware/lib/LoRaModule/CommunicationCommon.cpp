#include "CommunicationCommon.h"
#include <algorithm>

char* MACAddress::toString(char* string) const
{
    snprintf(string, MACAddress::stringLength, "%02X:%02X:%02X:%02X:%02X:%02X", this->address[0], this->address[1], this->address[2], this->address[3],
             this->address[4], this->address[5]);
    return string;
}

MACAddress MACAddress::fromString(char* string)
{
    MACAddress mac{};
    uint8_t* address = mac.getAddress();
    sscanf(string, "%02X:%02X:%02X:%02X:%02X:%02X", &address[0], &address[1], &address[2], &address[3], &address[4], &address[5]);
    return mac;
}
const MACAddress MACAddress::broadcast{};
char MACAddress::strBuffer[MACAddress::stringLength];
