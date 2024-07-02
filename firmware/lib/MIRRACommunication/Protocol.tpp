#ifndef __PROTOCOL_T__
#define __PROTOCOL_T__

template <MessageType T, class... Args> void Window::emplace(Args&&... args)
{
    Message<T>* writeHeadMessage = reinterpret_cast<Message<T>*>(writeHead);
    *writeHeadMessage = Message<T>(args...);
    prepush(writeHeadMessage->getSize());
}

template <MessageType T, class... Args> void Protocol::sendMessage(Args&&... args) { sendWind.emplace<T>(args...); }

template <MessageType T> void Protocol::sendMessage(Message<T>&& message) { sendWind.push(message.toData(), message.getSize(), sendWind.getCount()); }

#endif