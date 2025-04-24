#include <ipc/ipc.h>
#include <shared_mutex>
#include <handle.h>
#include "message.hpp"
#include "choose.hpp"
#include "ipc/callback.h"
#include "core/cache.hpp"
#include "core/segment.hpp"

using namespace ipc::detail;

namespace ipc
{

namespace
{
    #define MESSAGE          (impl_->message)
    #define CACHE           (impl_->cache)
    #define MODE            (impl_->mode)
    #define CONNECTED       (impl_->connected)
    #define CALLBACK        (impl_->callback)
} // internal-linkage



template<typename Wr>
struct Ipc<Wr>::IpcImpl
{
    std::unique_ptr<Message<Choose<Segment>>> message { nullptr };
    std::shared_ptr<void> cache {nullptr};
    unsigned mode { SENDER };
    std::atomic_bool connected { false };
    CallbackPtr callback {nullptr};
};

template <typename Wr>
Ipc<Wr>::Ipc(
    char const *name,
    const unsigned &mode)
    : impl_ { std::make_unique<Ipc<Wr>::IpcImpl>() }
{
    connect(name, mode);
}

template <typename Wr>
Ipc<Wr>::Ipc(
    Ipc&& rhs) noexcept
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
Ipc<Wr>& Ipc<Wr>::operator=(
    Ipc<Wr> rhs) noexcept
{
    impl_.swap(rhs.impl_);
    return *this;
}

template <typename Wr>
char const * Ipc<Wr>::name() const noexcept
{
    return (MESSAGE == nullptr) ? nullptr : MESSAGE->name().c_str();
}

template <typename Wr>
bool Ipc<Wr>::valid() const noexcept
{
    return (MESSAGE != nullptr);
}

template <typename Wr>
unsigned Ipc<Wr>::mode() const noexcept
{
    return MODE;
}

template <typename Wr>
bool Ipc<Wr>::connect(
    char const * name,
    const unsigned &mode)
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
        CACHE = std::make_shared<Cache<Sender>>();
        break;
    case static_cast<unsigned>(RECEIVER):
        CACHE = std::make_shared<Cache<Receiver>>();
        break;
    default:
        break;
    }

    disconnect();
    if(!valid())
    {
        MESSAGE = std::make_unique<Message<Choose<Segment>>>(nullptr,name);
    }
    MODE = mode;
    return CONNECTED = reconnect(mode);
}

template <typename Wr>
bool Ipc<Wr>::reconnect(
    unsigned mode)
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

    if (MESSAGE == nullptr)
    {
        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_NOMEM);
        }
        return false;
    }

    if(!(MESSAGE->init()))
    {
        if(CALLBACK)
        {
            CALLBACK->connected(ErrorCode::IPC_ERR_NOINIT);
        }
        return false;
    }

    if (mode & RECEIVER)
    {
        MESSAGE->disconnect();
        if (MESSAGE->connect(mode))
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

    if (MESSAGE->connected_id())
    {
        MESSAGE->disconnect();
    }

    if(CALLBACK)
    {
        CALLBACK->connected();
    }

    return MESSAGE->connect(mode);
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
    if (MESSAGE == nullptr)
    {
        if(CALLBACK)
        {
            CALLBACK->connection_lost(ErrorCode::IPC_ERR_NOMEM);
        }
        return;
    }
    CONNECTED = false;
    MESSAGE->disconnect();
    assert((MESSAGE) != nullptr);
    MESSAGE->disconnect();

    if(CALLBACK)
    {
        CALLBACK->connection_lost();
    }
}

template <typename Wr>
void Ipc<Wr>::set_callback(
    CallbackPtr callback)
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
bool Ipc<Wr>::write(
    void const *data,
    std::size_t size)
{
    if (!valid() || data == nullptr || size == 0)
    {
        if(CALLBACK)
        {
            CALLBACK->delivery_complete(ErrorCode::IPC_ERR_NOINIT);
        }
        return false;
    }
    if (MESSAGE == nullptr || MESSAGE->segment() == nullptr || !MESSAGE->connect() ||
            !(MESSAGE->segment()->connections()))
    {
        if(CALLBACK)
        {
            CALLBACK->delivery_complete(ErrorCode::IPC_ERR_NOMEM);
        }
        return false;
    }

    auto desc = std::static_pointer_cast<Description>( 
        std::static_pointer_cast<Cache<Sender>>(CACHE)->write(data,size,MESSAGE->segment()->recv_count())
    );
    if(!desc->length() || !MESSAGE->push(*desc))
    {
        if(CALLBACK)
        {
            CALLBACK->delivery_complete(ErrorCode::IPC_ERR_INVAL);
        }
        return false;
    }
    
    auto ret = Wr::is_broadcast ? 
        MESSAGE->waiter()->broadcast() : MESSAGE->waiter()->notify();

    if(!ret || CALLBACK)
    {
        CALLBACK->delivery_complete();
    }

    return true;
}

template <typename Wr>
bool Ipc<Wr>::write(
    Buffer const & buff)
{
    return this->write(buff.data(), buff.size());
}

template <typename Wr>
bool Ipc<Wr>::write(
    std::string const & str)
{
    return this->write(str.c_str(), str.size());
}

static std::string thread_id_to_string(
    const uint32_t &id)
{
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

template <typename Wr>
void Ipc<Wr>::read(
    std::uint64_t tm)
{
    if(!valid())
    {
        if(CALLBACK)
        {
            CALLBACK->message_arrived(nullptr, ErrorCode::IPC_ERR_NOMEM);
        }
        return ;
    }
    if (MESSAGE == nullptr)
    {
        if(CALLBACK)
        {
            CALLBACK->message_arrived(nullptr, ErrorCode::IPC_ERR_NOMEM);
        }
        return ;
    }
    if (!MESSAGE->connected_id())
    {
        if(CALLBACK)
        {
            CALLBACK->message_arrived(nullptr, ErrorCode::IPC_ERR_NO_CONN);
        }
        return ;
    }
    for (;;)
    {
        if(!is_connected())
        {
            break;
        }

        MESSAGE->wait_for([&]
        {
            Description desc{};
            while(!MESSAGE->empty())
            {
                if(!MESSAGE->pop(desc,[&](bool) -> bool
                {
                    return std::static_pointer_cast<Cache<Receiver>>(CACHE)->read(desc,[&](const Buffer *buf) -> void
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
