#include "CommunicationCommon.h"

MIRRAAddress::MIRRAAddress(const char* string)
{
    sscanf(string, "%04X:%04X", &gateway, &node);
};

char* MIRRAAddress::toString(char* string) const
{
    snprintf(string, MIRRAAddress::stringLength, "%04X:%04X", gateway, node);
    return string;
}

char MIRRAAddress::strBuffer[MIRRAAddress::stringLength];

static_assert(sizeof(MIRRAAddress) == MIRRAAddress::size);
static_assert(sizeof(MessageHeader) == MessageHeader::size);