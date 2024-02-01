#ifndef __COMM_COMM_H__
#define __COMM_COMM_H__

#include <bitset>
#include <array>
#include <utility>
#include <algorithm>

#include "Sensor.h"

struct MIRRAAddress
{
    using GatewayID = uint16_t;
    using NodeID = uint16_t;

    GatewayID gateway{0};
    NodeID node{0};

    constexpr MIRRAAddress() = default;
    /// @brief Constructs a MIRRA address from a gateway and node ID combo.
    /// @param gateway The gateway ID.
    /// @param node The node ID.
    constexpr MIRRAAddress(GatewayID gateway, NodeID node) : gateway{gateway}, node{node} {}
    /// @brief Constructs a MIRRA address from a string.
    /// @param string The hex string from which to construct the MIRRA address in the "0000:0000" format.
    constexpr MIRRAAddress(const char* string);


    bool operator==(const MIRRAAddress& other) const {return this->gateway == other.gateway && this->node == other.node;}
    bool operator!=(const MIRRAAddress& other) const {return !(*this == other);}

    /// @brief Gives a hex string representation of this MAC address.
    /// @param string Buffer to write the resulting string to. By default, this uses a static buffer.
    /// @return The buffer to which the string was written.
    char* toString(char* string = strBuffer) const;

    /// @brief Size of MIRRA address in bytes.
    static constexpr size_t size = sizeof(GatewayID) + sizeof(NodeID);
    /// @brief Length of the string representation of a MAC address in bytes, including terminator.
    static constexpr size_t stringLength{4 + 1 + 4 + 1};

private:
    /// @brief Internal static string buffer used by the toString method.
    static char strBuffer[stringLength];

}__attribute__((packed));

/// @brief Enum used to indicate the type of a message. Maximum of 128 available types.
enum MessageType : uint8_t
{
    HELLO = 0,
    CONFIG = 1,
    DATA = 2,
    ACK = 3,
    ANY = 7,
};

/// @brief Base class providing a common interface between all message types and the header portion of the message.
struct MessageHeader
{
    private:
    /// @brief Type of the message
    MessageType type : 3;
    public:
    /// @brief Last flag
    bool last : 1;
    /// @brief Sequence number
    uint8_t seq : 4;
    /// @brief The source MAC address of the message.
    MIRRAAddress addr;

    constexpr MessageHeader(MessageType type) : type{type}, last{false}, seq{0}, addr{} {};
    constexpr bool isType(MessageType type) const { return this->type == type; }
    /// @brief Forcibly sets the type of this message.
    /// @param type The type to force-set this message to.
    void setType(MessageType type) { this->type = type; }

    /// @brief The size of the header in bytes.
    static constexpr size_t size{ 1 + MIRRAAddress::size};
} __attribute__((packed));


/// @brief Message body class providing per-type data and functionality.
/// @tparam T The message type of the message.
template <MessageType T> struct MessageBody
{
    /// @return The message body size in bytes.
    constexpr size_t getSize() {return 0;}
};

template <MessageType T> struct Message
{
    MessageHeader header{T};
    MessageBody<T> body;
public:
    template <class... Ts, class = decltype(MessageBody(std::declval<Ts>()...))> constexpr Message(Ts&&... args) : body{std::forward<Ts>(args)...} {};

    constexpr const MessageHeader& getHeader() {return header;}
    /// @return Whether the message's type flag matches the desired type.
    constexpr bool isValid() const { return header.isType(T); }
    /// @return The message size in bytes.
    constexpr size_t getSize() const {return header.size + body.getSize();}

    /// @brief Converts this message in-place to a byte buffer.
    /// @return The pointer to the resulting byte buffer.
    constexpr const uint8_t* toData() const { return reinterpret_cast<const uint8_t*>(this); }
    /// @brief Converts a byte buffer in-place to this message type, without any runtime checking.
    /// @param data The byte buffer to interpret a message from.
    /// @return The resulting message object.
    static constexpr Message<T>& fromData(uint8_t* data) { return *reinterpret_cast<Message<T>*>(data); }


    /// @brief The maximum size of a message in bytes.
    static constexpr size_t maxSize{255};
} __attribute__((packed));

template <> struct MessageBody<CONFIG>
{
    uint32_t curTime, sampleInterval, sampleRounding, sampleOffset, commInterval, commTime, maxMessages;

    constexpr MessageBody(uint32_t curTime, uint32_t sampleInterval, uint32_t sampleRounding, uint32_t sampleOffset,
            uint32_t commInterval, uint32_t commTime, uint32_t maxMessages)
        : curTime{curTime}, sampleInterval{sampleInterval}, sampleRounding{sampleRounding}, sampleOffset{sampleOffset},
          commInterval{commInterval}, commTime{commTime}, maxMessages{maxMessages} {};

    /// @return The message body size in bytes.
    constexpr size_t getSize() const { return sizeof(*this); };
} __attribute__((packed));


template <> struct MessageBody<DATA>
{
    /// @brief The timestamp associated with the held values (UNIX epoch, seconds).
    uint32_t time;
    /// @brief The amount of values held in the messages' values array.
    uint8_t nValues;
    /// @brief The maximum amount of sensor values that can be held in a single sensor data message.
    static constexpr size_t maxNValues{(Message<ANY>::maxSize - MessageHeader::size - sizeof(time) - sizeof(nValues)) / sizeof(SensorValue)};

    std::array<SensorValue, maxNValues> values;

    constexpr MessageBody(uint32_t time, uint8_t nValues)
        : time{time}, nValues{std::min(nValues, static_cast<uint8_t>(maxNValues))}, values{} {};

    /// @return The message body size in bytes.
    constexpr size_t getSize() const {return sizeof(time) + sizeof(nValues) + nValues * sizeof(SensorValue); };
} __attribute__((packed));

using ackSet = std::bitset<16>;

template <> struct MessageBody<ACK>
{
    /// @brief Bitset containing ACKS(1)/NACKS(0) for every received message according to sequence number
    ackSet acks;

    constexpr MessageBody(ackSet acks) : acks{acks}{}

    /// @return The message body size in bytes.
    constexpr size_t getSize() const {return sizeof(*this);}
} __attribute__((packed));

#endif
