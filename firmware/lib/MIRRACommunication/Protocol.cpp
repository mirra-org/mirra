#include <Protocol.h>

using namespace mirra::communication;

Window::Window(size_t size)
{
    buffer.reserve(size);
    writeHead = buffer.begin().base();
    messages.fill(nullptr);
}

void Window::push(uint8_t* buffer, size_t size) { std::copy(buffer, buffer + size, push(size)); }

uint8_t* Window::push(size_t size)
{
    uint8_t* oldWriteHead = writeHead;
    messages[count] = writeHead;
    sizes[count] = size;
    count++;
    writeHead += size;
    return oldWriteHead;
}

void Window::pop()
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
    size_t messageSize = sendWind.getSize(index);
    MessageHeader* messageHeader = reinterpret_cast<MessageHeader*>(messageBytes);
    messageHeader->addr = this->addr;
    messageHeader->seq = index;
    if (sendWind.getCount() != index)
        messageHeader->last = true;
    if (!lora.sendPacket(messageBytes, messageSize))
        return false;
    if (!lora.wait())
        return false;
    timeBudgetMs -= lora.getTimeOnAir(messageSize); // todo: use timer to accurately reduce budget
    return true;
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
    while (true)
    {
        if (!lora.receivePacket(timeoutMs))
        {
            return false;
        }
        if (!lora.wait())
        {
        }
        timeBudgetMs -= lora.getTimeOnAir(lora.getPacketLength()); // todo: use timer to accurately reduce budget
        uint8_t* recvWindowWriteHead = recvWind.push(lora.getPacketLength());
        if (!lora.readPacket(recvWindowWriteHead))
        {
            recvWind.pop();
            continue;
        }
        MessageHeader* header = reinterpret_cast<MessageHeader*>(recvWindowWriteHead);
        if (header->addr.gateway != this->addr.gateway) // message is either from another MIRRA set or another source entirely
        {
            recvWind.pop();
            continue;
        }
        if (header->isType(MessageType::ACK))
        {
            sendWind.getAcks() = reinterpret_cast<Message<ACK>*>(header)->body.acks;
            recvWind.pop();
            return receive(messageSize, type);
        }
        if (!header->isType(type) && type != MessageType::ANY) // message does not have desired type
        {
            recvWind.pop();
            return false;
        }

        recvWind.getAcks()[header->seq] = true;
        if (header->last)
        {
            return true;
        }
    }
}

bool Protocol::close() {}
