#include <Protocol.h>

using namespace mirra::comm;

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

void Window::push(size_t size)
{
    return push(size, std::find_if_not(static_cast<size_t>(0), maxWindowSize, hasMessage));
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
    auto [timedOut, timeAsleep] = lora.wait();
    if (timedOut)
        return false;
    timeBudgetMs -= timeAsleep;
}

bool Protocol::send()
{
    for (size_t i = 0; i < sendWind.maxWindowSize; i++)
    {
        if (!sendWind.getAcks()[i])
        {
            MessageHeader& messageHeader = sendWind.getMessageHeader(i);
            size_t messageSize = sendWind.getSize(i);
            messageHeader.addr = this->addr;
            messageHeader.seq = i;

            if (std::none_of(i + 1, Window::maxWindowSize,
                             [this](size_t i) { return sendWind.hasMessage(i); }))
                messageHeader.last = true;
            if (!lora.sendPacket(sendWind[i], messageSize,
                                 2 * lora.getTimeOnAir(messageSize) / 1000))
                return false;
            auto [timedOut, timeAsleep] = lora.wait();
            if (timedOut)
                return false;
            timeBudgetMs -= timeAsleep;
        }
    }
    return true;
}

bool Protocol::receive(size_t messageSize, MessageType type)
{
    recvWind.clear();
    send();
    uint32_t timeoutMs = std::min(1000 + (2 * lora.getTimeOnAir(messageSize) / 1000), timeBudgetMs);
    while (true)
    {
        if (!lora.receivePacket(timeoutMs))
        {
            return false;
        }
        auto [timedOut, timeAsleep] = lora.wait();
        if (timeBudgetMs <= timeAsleep)
        {
            return false;
        }
        timeBudgetMs -= timeAsleep;
        if (timedOut || !lora.readPacket(recvWind.getWriteHead()))
        {
            sendAcks();
            continue;
        }
        MessageHeader* header =
            std::launder(reinterpret_cast<MessageHeader*>(recvWind.getWriteHead()));
        if (this->addr.gateway != 0 && header->addr.gateway != this->addr.gateway)
        {
            // message is either from another MIRRA set or another source entirely
            continue;
        }
        if (header->isType(MessageType::ACK))
        {
            sendWind.getAcks() = Message<ACK>::fromData(recvWind.getWriteHead()).body.acks;
            send();
            continue;
        }
        if (!header->isType(type) && type != MessageType::ANY) // message does not have desired type
        {
            return false;
        }
        recvWind.push(lora.getPacketLength(), header->seq);
        recvWind.getAcks()[header->seq] = true;
        if (header->last)
        {
            sendWind.clear();
            return true;
        }
    }
}

bool Protocol::close()
{
    receive(sizeof(Message<MessageType::ACK>), MessageType::ACK);
}
