#include <Protocol.h>

using namespace mirra::communication;

Window::Window() : buffer{new std::array<uint8_t, bufferSize>}, writeHead{buffer->begin()} {}

void Window::push(uint8_t* buffer, size_t size, size_t index)
{
    std::copy(buffer, buffer + size, writeHead);
    push(size, index);
}

void Window::push(size_t size, size_t index)
{
    messages[index] = writeHead;
    sizes[index] = size;
    writeHead += size;
}

void Window::clear()
{
    writeHead = buffer->begin();
    messages.fill(nullptr);
    sizes.fill(0);
    acks.reset();
}

bool Protocol::sendAcks()
{
    Message<ACK> ackMessage(recvWind.getAcks());
    ackMessage.header.addr = this->addr;
    if (!lora.sendPacket(ackMessage.toData(), ackMessage.getSize()))
        return false;
    if (!lora.wait())
        return false;
    timeBudgetMs -= lora.getTimeOnAir(ackMessage.getSize()) / 1000; // todo: use timer to accurately reduce budget
}

bool Protocol::send(size_t index)
{
    MessageHeader& messageHeader = sendWind.getMessageHeader(index);
    size_t messageSize = sendWind.getSize(index);
    messageHeader.addr = this->addr;
    messageHeader.seq = index;

    if (std::none_of(index, Window::maxWindowSize, [this](size_t i) { return sendWind.hasMessage(i); }))
        messageHeader.last = true;
    if (!lora.sendPacket(sendWind[index], messageSize))
        return false;
    if (!lora.wait())
        return false;
    timeBudgetMs -= lora.getTimeOnAir(messageSize) / 1000; // todo: use timer to accurately reduce budget
    return true;
}

bool Protocol::receive(size_t messageSize, MessageType type)
{
    for (size_t i = 0; i < sendWind.maxWindowSize; i++)
    {
        if (!sendWind.getAcks()[i])
        {
            send(i);
        }
    }
    uint32_t timeoutMs = 1000 + (2 * lora.getTimeOnAir(messageSize) / 1000);
    while (true)
    {
        if (!lora.receivePacket(timeoutMs))
        {
            return false;
        }
        bool timedOut = !lora.wait();
        timeBudgetMs -= lora.getTimeOnAir(lora.getPacketLength()) / 1000; // todo: use timer to accurately reduce budget
        if (timedOut || !lora.readPacket(recvWind.getWriteHead()))
        {
            sendAcks();
            continue;
        }
        MessageHeader* header = reinterpret_cast<MessageHeader*>(recvWind.getWriteHead());
        if (header->addr.gateway != this->addr.gateway) // message is either from another MIRRA set or another source entirely
        {
            continue;
        }
        if (header->isType(MessageType::ACK))
        {
            sendWind.getAcks() = reinterpret_cast<Message<ACK>*>(header)->body.acks;
            return receive(messageSize, type);
        }
        if (!header->isType(type) && type != MessageType::ANY) // message does not have desired type
        {
            return false;
        }
        recvWind.push(lora.getPacketLength(), header->seq);
        recvWind.getAcks()[header->seq] = true;
        if (header->last)
        {
            return true;
        }
    }
}

bool Protocol::close() {}
