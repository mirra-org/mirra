#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <CommunicationCommon.h>
#include <LoRaModule.h>

#include <array>
#include <optional>

#include "config.h"

namespace mirra::comm
{

class Window
{
public:
    static constexpr size_t maxWindowSize{8};
    static constexpr size_t bufferSize{maxWindowSize * Message<ANY>::maxSize};

    using RawMessagesArray = std::array<uint8_t*, maxWindowSize>;
    template <MessageType T> using MessagesArray = std::array<Message<T>*, maxWindowSize>;

private:
    std::unique_ptr<std::array<uint8_t, bufferSize>> buffer;
    AckSet acks{};
    RawMessagesArray messages{0};
    std::array<size_t, maxWindowSize> sizes{0};
    uint8_t* writeHead;

public:
    Window();

    template <MessageType T, class... Args> void emplace(Args&&... args);

    void push(uint8_t* buffer, size_t size, size_t index);
    void push(size_t size, size_t index);
    void push(size_t size);

    void clear();

    bool hasMessage(size_t index) const { return sizes[index] != 0; }
    size_t getSize(size_t index) const { return sizes[index]; }

    uint8_t* operator[](size_t index) const { return messages[index]; }
    MessageHeader& getMessageHeader(size_t index) const
    {
        return *reinterpret_cast<MessageHeader*>(messages[index]);
    };
    template <MessageType T> const MessagesArray<T>& getMessagesArray() const
    {
        return *reinterpret_cast<MessagesArray<T>>(messages);
    }

    AckSet& getAcks() { return acks; }
    uint8_t* getWriteHead() const { return writeHead; }
};

class Protocol
{

private:
    Address addr;
    LoRaModule& lora;

    Window sendWind{maxWindowSize * Message<ANY>::maxSize};
    Window recvWind{maxWindowSize * Message<ANY>::maxSize};

    size_t timeBudgetMs;

    Protocol(Address addr, uint8_t spreadingFactor, size_t timeBudgetMs)
        : addr{addr}, lora{lora}, timeBudgetMs{timeBudgetMs} {};

    bool send();
    bool sendAcks();
    bool receive(size_t messageSize, MessageType type);

public:
    template <MessageType T, class... Args> void sendMessage(Args&&... args);
    template <MessageType T> void sendMessage(Message<T>&& message);
    /// @return The received message. Disengaged if no valid message could be received.
    template <MessageType T> const Window::MessagesArray<T>& receiveMessages() {}
    bool close();

    friend class LoRaModule;
};

#include <Protocol.tpp>

};

#endif