#include "MIRRACommunicationCommon.h"

using namespace mirra::communication;

Address::Address(const char* string) { sscanf(string, "%04X:%04X", &gateway, &node); };

char* Address::toString(char* string) const
{
    snprintf(string, Address::stringLength, "%04X:%04X", gateway, node);
    return string;
}

char Address::strBuffer[Address::stringLength];