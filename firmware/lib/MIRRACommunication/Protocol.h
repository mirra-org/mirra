#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <LoRaModule.h>
#include <MIRRACommunicationCommon.h>

#include <array>
#include <optional>

#define DEFAULT_TIMEOUT (1500)
#define DEFAULT_REPEATS (2)

namespace mirra::communication
{
class Window
{
public:
    static constexpr size_t maxWindowSize{8};
    static constexpr size_t bufferSize{maxWindowSize * Message<ANY>::maxSize};

private:
    std::unique_ptr<std::array<uint8_t, bufferSize>> buffer;
    AckSet acks{};
    std::array<uint8_t*, maxWindowSize> messages{0};
    std::array<size_t, maxWindowSize> sizes{0};
    uint8_t* writeHead;

public:
    Window();

    template <MessageType T, class... Args> void emplace(Args&&... args);

    void push(size_t size, size_t index);
    void push(uint8_t* buffer, size_t size, size_t index);

    void clear();

    bool hasMessage(size_t index) const { return sizes[index] != 0; }
    uint8_t* operator[](size_t index) const { return messages[index]; }
    MessageHeader& getMessageHeader(size_t index) { return *reinterpret_cast<MessageHeader*>(messages[index]); };
    size_t getSize(size_t index) const { return sizes[index]; }

    uint8_t* getWriteHead() const { return writeHead; }
    AckSet& getAcks() { return acks; }
};

class Protocol
{

private:
    Address addr;
    LoRaModule& lora;

    Window sendWind{maxWindowSize * Message<ANY>::maxSize};
    Window recvWind{maxWindowSize * Message<ANY>::maxSize};

    size_t timeBudgetMs;
    uint8_t repeats;

    Protocol(Address addr, LoRaModule& lora, size_t timeBudgetMs = DEFAULT_TIMEOUT, size_t repeats = DEFAULT_REPEATS)
        : addr{addr}, lora{lora}, timeBudgetMs{timeBudgetMs}, repeats{repeats} {};

    bool send(size_t index);
    bool receive(size_t messageSize, MessageType type);

public:
    template <MessageType T, class... Args> void sendMessage(Args&&... args);
    template <MessageType T> void sendMessage(Message<T>&& message);
    /// @return The received message. Disengaged if no valid message could be received.
    template <MessageType T> std::vector<std::optional<Message<T>*>> receiveMessages(size_t expected) {}
    bool close();

    friend class LoRaModule;
};

#include <Protocol.tpp>

};

#endif