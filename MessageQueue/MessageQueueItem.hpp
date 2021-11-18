#ifndef MESSAGEQUEUEITEMHPP
#define MESSAGEQUEUEITEMHPP

template <typename TMessage>
struct MessageQueueItem
{
    TMessage content;
    std::size_t clientId;
    bool consumed = false;
    MessageQueueItem(TMessage content, std::size_t clientId) : content(content), clientId(clientId){};
};

#endif
