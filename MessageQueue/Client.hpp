#ifndef READERHPP
#define READERHPP

#include <list>
#include "MessageQueueItem.hpp"

template <typename TMessage>
struct Client
{
    std::size_t clientId;
    typename std::list<MessageQueueItem<TMessage>>::const_reverse_iterator rit;
    Client(std::size_t clientId, typename std::list<MessageQueueItem<TMessage>>::const_reverse_iterator rit) : clientId(clientId), rit(rit){};
};

#endif