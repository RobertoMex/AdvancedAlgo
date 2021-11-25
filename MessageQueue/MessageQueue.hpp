#ifndef MESSAGEQUEUEHPP
#define MESSAGEQUEUEHPP

#include <list>
#include <shared_mutex>
#include <mutex>
#include "MessageQueueItem.hpp"

template <typename TMessage>
class Client;

template <typename TMessage>
class MessageQueue
{
private:
    std::list<MessageQueueItem<TMessage>> list{};
    std::shared_mutex mutex;
    std::size_t nextClientId = 0;

public:
    MessageQueue()
    {
        list.emplace_back(TMessage(), -1);
    }

    Client<TMessage> getClient()
    {
        std::unique_lock lock(mutex);
        Client<TMessage> client(nextClientId, list.begin());
        ++nextClientId;
        return client;
    };

    void enqueue(const TMessage content, const Client<TMessage> &client)
    {
        std::unique_lock lock(mutex);
        list.emplace_back(content, client.clientId);
    };

    bool hasNext(Client<TMessage> &client)
    {
        std::shared_lock lock(mutex);
        if (list.empty())
        {
            return false;
        }
        return client.hasNext(list.begin(), list.end());
    };

    const TMessage next(Client<TMessage> &client)
    {
        std::shared_lock lock(mutex);
        if (list.empty())
        {
            return TMessage();
        }
        return client.consume(list.begin(), list.end());
    };
};

template <typename TMessage>
class Client
{
    friend class MessageQueue<TMessage>;

private:
    std::size_t clientId;
    typename std::list<MessageQueueItem<TMessage>>::iterator it;
    bool consumed = true;

    void advance(typename std::list<MessageQueueItem<TMessage>>::iterator beg, typename std::list<MessageQueueItem<TMessage>>::iterator end)
    {
        while (consumed && std::next(it) != end)
        {
            ++it;
            consumed = (it->clientId == clientId);
        }
    };

    const TMessage consume(typename std::list<MessageQueueItem<TMessage>>::iterator beg, typename std::list<MessageQueueItem<TMessage>>::iterator end)
    {
        advance(beg, end);
        consumed = true;
        return it->content;
    };

    bool hasNext(typename std::list<MessageQueueItem<TMessage>>::iterator beg, typename std::list<MessageQueueItem<TMessage>>::iterator end)
    {
        advance(beg, end);
        return !consumed;
    };

public:
    Client(std::size_t clientId, typename std::list<MessageQueueItem<TMessage>>::iterator it) : clientId(clientId), it(it){};
};

#endif
