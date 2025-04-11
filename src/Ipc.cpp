#include <ipc/Ipc.h>
#include <shared_mutex>
#include <Handle.h>
#include <MessageQueue.hpp>
#include <Choose.hpp>
#include <ipc/Callback.h>
#include <core/Cache.hpp>
#include <core/Segment.hpp>

using namespace ipc::detail;

namespace ipc
{

namespace
{
    #define HANDLE         (impl_->handle)
    #define FRAGMENT       (impl_->fragment)
    #define MODE           (impl_->mode)
    #define CONNECTED      (impl_->connected)
    #define CALLBACK       (impl_->callback)
} // internal-linkage



template<typename Wr>
struct Ipc<Wr>::IpcImpl
{
    std::shared_ptr<MessageQueue<Choose<Segment>> > handle { nullptr };
    std::shared_ptr<void> fragment {nullptr};
    unsigned mode { SENDER };
    std::atomic_bool connected { false };
    CallbackPtr callback {nullptr};
};

template <typename Wr>
Ipc<Wr>::Ipc(char const * name, const unsigned &mode)
    : impl_ {std::make_unique<Ipc<Wr>::IpcImpl>()}
{
    connect(name, mode);
}

template <typename Wr>
Ipc<Wr>::Ipc(Ipc&& rhs) noexcept
        : Ipc{}
{
    impl_.swap(rhs.impl_);
}

template <typename Wr>
Ipc<Wr>::~Ipc()
{
    disconnect();
}

template <typename Wr>
Ipc<Wr>& Ipc<Wr>::operator=(Ipc<Wr> rhs) noexcept
{
    impl_.swap(rhs.impl_);
    return *this;
}

template <typename Wr>
char const * Ipc<Wr>::name() const noexcept
{
    return (HANDLE == nullptr) ? nullptr : HANDLE->name().c_str();
}

template <typename Wr>
bool Ipc<Wr>::valid() const noexcept
{
    return (HANDLE != nullptr);
}

template <typename Wr>
unsigned Ipc<Wr>::mode() const noexcept
{
    return MODE;
}

template <typename Wr>
bool Ipc<Wr>::connect(char const * name, const unsigned &mode)
{
    if (name == nullptr || name[0] == '\0')
    {
        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_NOINIT);
        }
        return false;
    }

    switch (mode)
    {
    case static_cast<unsigned>(SENDER):
        FRAGMENT = std::make_shared<CaCheManager<CacheSender>>();
        break;
    case static_cast<unsigned>(RECEIVER):
        FRAGMENT = std::make_shared<CaCheManager<CacheReceiver>>();
        break;
    default:
        break;
    }

    disconnect();
    if(!valid())
    {
        HANDLE = std::make_shared<MessageQueue<Choose<Segment>>>(nullptr,name);
        if(!HANDLE->init())
        {
            if(CALLBACK)
            {
                CALLBACK->connected(ErrorCode::IPC_ERR_NOINIT);
            }
            return false;
        }
    }
    MODE = mode;
    return CONNECTED = reconnect(mode);
}

template <typename Wr>
bool Ipc<Wr>::reconnect(unsigned mode)
{
    if (!valid())
    {
        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_INVAL);
        }
        return false;
    }
    if (CONNECTED && (MODE == mode))
    {
        if(CALLBACK)
        {
            CALLBACK->connected();
        }
        return true;
    }

    auto que = HANDLE->queue();
    if (que == nullptr)
    {
        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_NOMEM);
        }
        return false;
    }

    if(!(HANDLE->init()))
    {
        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_NOINIT);
        }
        return false;
    }

    if (mode & RECEIVER)
    {
        que->disconnect();
        if (que->connect(mode))
        {
            if(CALLBACK)
            {
                CALLBACK->connected();
            }
            return true;
        }

        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_NOINIT);
        }
        return false;
    }

    if (que->connected_id())
    {
        HANDLE->disconnect();
    }

    if(CALLBACK)
    {
        CALLBACK->connected();
    }

    return que->connect(mode);
}

template <typename Wr>
void Ipc<Wr>::disconnect()
{
    if (!valid())
    {
        if(CALLBACK)
        {
            CALLBACK->connection_lost(ErrorCode::IPC_ERR_NOMEM);
        }
        return;
    }
    auto que = HANDLE->queue();
    if (que == nullptr)
    {
        if(CALLBACK)
        {
            CALLBACK->connection_lost(ErrorCode::IPC_ERR_NOMEM);
        }
        return;
    }
    CONNECTED = false;
    que->disconnect();
    assert((HANDLE) != nullptr);
    HANDLE->disconnect();

    if(CALLBACK)
    {
        CALLBACK->connection_lost();
    }
}

template <typename Wr>
void Ipc<Wr>::set_callback(CallbackPtr callback)
{
    if(!CALLBACK)
    {
        CALLBACK = callback;
    }
}

template <typename Wr>
bool Ipc<Wr>::is_connected() const noexcept
{
    return CONNECTED;
}

template <typename Wr>
bool Ipc<Wr>::write(void const *data, std::size_t size)
{
    if (!valid() || data == nullptr || size == 0)
    {
        if(CALLBACK)
        {
            CALLBACK->delivery_complete(ErrorCode::IPC_ERR_NOINIT);
        }
        return false;
    }
    auto que = HANDLE->queue();
    if (que == nullptr || que->segment() == nullptr || !que->connect() ||
            !(que->segment()->connections()))
    {
        if(CALLBACK)
        {
            CALLBACK->delivery_complete(ErrorCode::IPC_ERR_NOMEM);
        }
        return false;
    }

    Description desc = std::static_pointer_cast<CaCheManager<CacheSender>>(FRAGMENT)->write(data,size,que->segment()->recv_count());
    if(!desc.length() || !que->push(desc))
    {
        if(CALLBACK)
        {
            CALLBACK->delivery_complete(ErrorCode::IPC_ERR_INVAL);
        }
        return false;
    }
    
    auto ret = Wr::is_broadcast ? 
        HANDLE->waiter()->broadcast() : HANDLE->waiter()->notify();

    if(!ret || CALLBACK)
    {
        CALLBACK->delivery_complete();
    }

    return true;
}

template <typename Wr>
bool Ipc<Wr>::write(Buffer const & buff)
{
    return this->write(buff.data(), buff.size());
}

template <typename Wr>
bool Ipc<Wr>::write(std::string const & str)
{
    return this->write(str.c_str(), str.size());
}

static std::string thread_id_to_string(const uint32_t &id)
{
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

template <typename Wr>
void Ipc<Wr>::read(std::uint64_t tm)
{
    if(!valid())
    {
        if(CALLBACK)
        {
            CALLBACK->message_arrived(ErrorCode::IPC_ERR_NOMEM);
        }
        return ;
    }
    auto que = HANDLE->queue();
    if (que == nullptr)
    {
        if(CALLBACK)
        {
            CALLBACK->message_arrived(ErrorCode::IPC_ERR_NOMEM);
        }
        return ;
    }
    if (!que->connected_id())
    {
        if(CALLBACK)
        {
            CALLBACK->message_arrived(ErrorCode::IPC_ERR_NO_CONN);
        }
        return ;
    }
    for (;;)
    {
        if(!is_connected())
        {
            break;
        }

        HANDLE->wait_for([&]
        {
            Description desc{};
            while(!que->empty())
            {
                if(!que->pop(desc,[&](bool) -> bool
                {
                    return std::static_pointer_cast<CaCheManager<CacheReceiver>>(FRAGMENT)->read(desc,[&](const Buffer *buf) -> void
                    {
                        CALLBACK->message_arrived(buf);
                    });
                }))
                {
                    return;
                }
            }
        }, tm);
    }
}

// UNICAST 一个通道对应一个Read
template struct Ipc<Wr<Transmission::UNICAST>>;

// BROADCAST 一个通道对应多个Read
template struct Ipc<Wr<Transmission::BROADCAST>>;


} // namespace ipc
