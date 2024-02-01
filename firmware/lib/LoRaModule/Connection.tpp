#ifndef __CONNECTION_T__
#define __CONNECTION_T__

template <class T> bool BiConnection::sendMessage(T&& message)
{
    return sendHeader(std::forward<T>(message).header, std::forward<T>(message).getSize())
}

template <MessageType T> std::optional<Message<T>>  BiConnection::receiveMessage()
{
    Message<T> m;
    if (!receiveHeader(m.getHeader(), sizeof(m)))
        return std::nullopt;
    return std::make_optional(m);
}

#endif