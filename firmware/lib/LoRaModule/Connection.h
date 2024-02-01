#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <CommunicationCommon.h>
#include <LoRaModule.h>


#include <array>
#include <optional>

#define DEFAULT_TIMEOUT (1500)
#define DEFAULT_REPEATS (2)

class BiConnection 
{
    MIRRAAddress addr;
    LoRaModule& lora;

    ackSet sendAcked;
    uint8_t sendSize{0};

    size_t seqSend{0};

    ackSet recvAcked;
    size_t recvSize{0};

    size_t timeoutMs;
    size_t repeats;

    BiConnection(MIRRAAddress addr, LoRaModule& lora, size_t timeoutMs = DEFAULT_TIMEOUT, size_t repeats = DEFAULT_REPEATS) : addr{addr}, lora{lora}, timeoutMs{timeoutMs}, repeats{repeats} {};

    bool sendRepeat();
    bool repeatSend();

    bool sendHeader(MessageHeader& header, size_t messageSize);
    bool receiveHeader(MessageHeader& header, size_t messageSize);

    friend LoRaModule;

public:
    template <class T> bool sendMessage(T&& message);
    /// @return The received message. Disengaged if no message was received or no valid message could be received.
    template <MessageType T> std::vector<std::optional<Message<T>>> receiveMessages(size_t expected)
    {

    }
};

#include <Connection.tpp>

#endif