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
    static constexpr size_t maxWindowSize{8};
    std::vector<uint8_t> buffer;
    AckSet acks{};
    std::array<uint8_t*, maxWindowSize> messages;
    std::array<size_t, maxWindowSize> sizes{0};
    uint8_t* writeHead;
    uint8_t count{0};

public:
    Window(size_t size);

    template <MessageType T, class... Args> void emplace(Args&&... args);
    uint8_t* push(size_t size);
    void push(uint8_t* buffer, size_t size);

    void pop();
    void clear();

    uint8_t* operator[](uint8_t index) { return messages[index]; };
    size_t getSize(uint8_t index) { return sizes[index]; }

    uint8_t* getWriteHead() { return writeHead; }
    AckSet& getAcks() { return acks; }
    const uint8_t getCount() const { return count; }
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