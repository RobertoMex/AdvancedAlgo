#ifndef MESSAGEQUEUEITEMHPP
#define MESSAGEQUEUEITEMHPP

template <typename TMessage>
struct MessageQueueItem
{
    const TMessage content;
    const std::size_t clientId;
    MessageQueueItem(const TMessage content, std::size_t clientId) : content(content), clientId(clientId){};
};

#endif
