#include <Connection.h>

bool BiConnection::sendRepeat()
{

}

bool BiConnection::repeatSend()
{

}

bool BiConnection::sendHeader(MessageHeader& header, size_t messageSize)
{
    header.addr = this->addr;
    header.seq = this->sendSeq;
    const uint8_t* messageBytes = reinterpret_cast<const uint8_t*>(&header);
    if(!lora.sendPacket(messageBytes, messageSize))
        return false;
    memcpy(sendBuffer, messageBytes, messageSize);
    lora.wait();
    this->sendSeq++;
    return true;
}

bool BiConnection::receiveHeader(MessageHeader& header, size_t messageSize, MessageType type)
{
    if (this->seq == 0)
    {

    }
    lora.receivePacket(timeoutMs + );
    if (!lora.wait());
        
    lora.readPacket(reinterpret_cast<uint8_t*>(&header), messageSize);
    if (header.addr.gateway != addr.gateway) //message is either from another MIRRA set or another source entirely
    if (!header.isType(type)  && type != MessageType::ANY) //message does not have desired type

}
