#include <Protocol.h>

using namespace mirra::communication;

Window::Window(size_t size)
{
    buffer.reserve(size);
    writeHead = buffer.begin().base();
    messages.fill(nullptr);
}

void Window::push(uint8_t* buffer, size_t size) { std::copy(buffer, buffer + size, prepush(size)); }

uint8_t* Window::prepush(size_t size)
{
    uint8_t* oldWriteHead = writeHead;
    messages[count] = writeHead;
    sizes[count] = size;
    count++;
    writeHead += size;
    return oldWriteHead;
}

void Window::depush()
{
    count--;
    writeHead -= sizes[count];
}

void Window::clear()
{
    writeHead = buffer.begin().base();
    messages.fill(nullptr);
    acks.reset();
    count = 0;
}

bool Protocol::send(size_t index)
{
    uint8_t* messageBytes = sendWind[index];
    MessageHeader* messageHeader = reinterpret_cast<MessageHeader*>(messageBytes);
    messageHeader->addr = this->addr;
    messageHeader->seq = sendWind.getCount() - 1;
    if (!lora.sendPacket(messageBytes, sendWind.getSize(index)))
        return false;
    return lora.wait();
}

bool Protocol::receive(size_t messageSize, MessageType type)
{
    for (size_t i = 0; i < sendWind.getCount(); i++)
    {
        if (!sendWind.getAcks()[i])
        {
            send(i);
        }
    }
    uint32_t timeoutMs = lora.getTimeOnAir(messageSize) * 2;
    bool receiving = true;
    while (receiving)
    {
        if (!lora.receivePacket(timeoutMs))
        {
            return false;
        }
        if (!lora.wait())
        {
        }
        timeBudgetMs -= lora.getTimeOnAir(lora.getPacketLength());
        uint8_t* recvWindowWriteHead = recvWind.prepush(lora.getPacketLength());
        if (!lora.readPacket(recvWindowWriteHead))
        {
            recvWind.depush();
            continue;
        }
        MessageHeader* header = reinterpret_cast<MessageHeader*>(recvWindowWriteHead);
        if (header->addr.gateway != this->addr.gateway) // message is either from another MIRRA set or another source entirely
        {
            recvWind.depush();
            continue;
        }
        if (header->isType(MessageType::ACK))
        {
            sendWind.getAcks() = reinterpret_cast<Message<ACK>*>(recvWindowWriteHead)->body.acks;
            recvWind.depush();
            return receive(messageSize, type);
        }
        if (!header->isType(type) && type != MessageType::ANY) // message does not have desired type
        {
            recvWind.depush();
            return false;
        }

        if (header->last)
        {
            receiving = false;
        }
    }
}

bool Protocol::close() {}
