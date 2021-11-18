#ifndef MESSAGEQUEUEHPP
#define MESSAGEQUEUEHPP

#include <list>
#include <shared_mutex>
#include <mutex>
#include "Client.hpp"
#include "MessageQueueItem.hpp"

template <typename TMessage>
class MessageQueue
{
private:
    std::list<MessageQueueItem<TMessage>> list{};
    mutable std::shared_mutex mutex;
    mutable std::size_t nextClientId = 0;

    void advance(const Client<TMessage> &client)
    {
        auto crbeg = list.crbegin();
        for (auto &rit = client.rit; rit != crbeg; ++rit)
        {
            MessageQueueItem<TMessage> message = *rit;
            if (!message.consumed && message.clientId != client.clientId)
            {
                break;
            }
        }
    };

public:
    const Client<TMessage> getClient() const
    {
        std::unique_lock lock(mutex);
        const Client<TMessage> client(nextClientId, list.crend());
        ++nextClientId;
        return client;
    };

    void enqueue(const TMessage content, const Client<TMessage> &client)
    {
        std::unique_lock lock(mutex);
        list.push_front(MessageQueueItem(content, client.clientId));
    };

    bool hasNext(Client<TMessage> &client) const
    {
        std::shared_lock lock(mutex);
        advance(client);
        if (client.rit == list.crend())
        {
            return false;
        }
        MessageQueueItem<TMessage> message = *client.rit;
        return (!message.consumed && message.clientId != client.clientId);
    };

    const TMessage next(Client<TMessage> &client) const
    {
        std::shared_lock lock(mutex);
        advance(client);
        if (client.rit == list.crend())
        {
            return TMessage();
        }
        MessageQueueItem<TMessage> message = *client.rit;
        message.consumed = true;
        return message.content;
    };
};

#endif
