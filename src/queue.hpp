#ifndef _IPC_QUEUE_H_
#define _IPC_QUEUE_H_

#include <tuple>
#include <cassert>
#include <type_traits>
#include "handle.h"
#include "resource.hpp"
#include "sync/waiter.h"

namespace ipc
{
namespace detail
{

class QueueConn
{
public:
    QueueConn() = default;
    QueueConn(
        const QueueConn &) = delete;
    QueueConn &operator=(
        const QueueConn &) = delete;

public:
    uint32_t connected_id() const noexcept
    {
        return connected_id_;
    }
    
    template <typename Segment>
    auto connect(
        Segment *segment,
        unsigned mode = RECEIVER) noexcept
        -> std::tuple<bool, bool, decltype(std::declval<Segment>().rd())>
    {
        if (segment == nullptr)
        {
            return {};
        }
        // if it's already connected, just return
        if (connected_id())
        {
            return {connected_id(), false, 0};
        }

        connected_id_ = segment->connect(mode);
        return {connected_id(), true, segment->rd()};
    }

    template <typename Segment>
    bool disconnect(
        Segment *segment) noexcept
    {
        if (segment == nullptr || !connected_id())
        {
            return false;
        }

        segment->disconnect(RECEIVER, std::exchange(connected_id_, 0));
        return true;
    }

protected:
    template <typename Segment>
    Segment *open(
        char const *name)
    {
        if (!is_valid_string(name))
        {
            return nullptr;
        }
        if (!handle_.acquire(name, sizeof(Segment)))
        {
            return nullptr;
        }
        auto segment = static_cast<Segment *>(handle_.get());
        if (segment == nullptr)
        {
            return nullptr;
        }
        segment->init();
        return segment;
    }

    void close()
    {
        handle_.release();
    }

protected:
    uint32_t connected_id_ = 0;
    Handle handle_;
};

template <typename Segment>
class QueueBase : public QueueConn
{
public:
    using segment_t = Segment;
    QueueBase() = default;

    explicit QueueBase(
        char const *name)
        : QueueBase{}
    {
        segment_ = QueueConn::template open<segment_t>(name);
    }

    virtual ~QueueBase()
    {

        if(handle_.ref() <= 1)
        {
            segment_->waiter().close();
        }
        QueueConn::close();
    }

public:
    bool open(
        char const *name) noexcept
    {
        QueueConn::close();
        segment_ = QueueConn::template open<segment_t>(name);
        return segment_ != nullptr;
    }

    segment_t *segment() noexcept
    { 
        return segment_;
    }

    bool connect(
        unsigned mode = RECEIVER) noexcept
    {
        auto tp = QueueConn::connect(segment_,mode);
        if (std::get<0>(tp) && std::get<1>(tp))
        {
            cursor_ = std::get<2>(tp);
            sender_flag_ = true;
            return true;
        }
        return std::get<0>(tp);
    }

    bool disconnect() noexcept
    {
        if (segment_ == nullptr || !connected_id())
        {
            return false;
        }

        segment_->disconnect(RECEIVER, std::exchange(connected_id_, 0));
        sender_flag_ = false;

        return true;
    }

    bool valid() const noexcept
    {
        return segment_ != nullptr;
    }

    bool empty() const noexcept
    {
        return !valid() || (cursor_ == segment_->wr());
    }

    template <typename Description, typename... P>
    bool push(
        P &&...params)
    {
        if (segment_ == nullptr || sender_flag_ == false)
        {
            return false;
        }
        return segment_->push(this, [&](void *p)
        {
            ::new (p) Description(std::forward<P>(params)...);
        });
    }

    template <typename D, typename F>
    bool pop(
        D &item,
        F &&out)
    {
        if (segment_ == nullptr || sender_flag_ == false)
        {
            return false;
        }
        return segment_->pop(this, cursor_, [&item](void *p)
        {
            ::new (&item) D(std::move(*static_cast<D *>(p)));
        }, std::forward<F>(out));
    }

    inline Waiter *waiter() noexcept
    {
        return &(segment_->waiter());
    }

private:
    // It is used to record the actual read subscript of the object currently being read.
    decltype(std::declval<segment_t>().rd()) cursor_ = 0;
    bool sender_flag_ = false;
    segment_t *segment_ = nullptr;
};

} // namespace detail

template <typename D, typename Choose>
class Queue final : 
    public detail::QueueBase<typename Choose::template segment_t<sizeof(D), alignof(D)>>
{
    using base_t = detail::QueueBase<typename Choose::template segment_t<sizeof(D), alignof(D)>>;

public:
    template <typename... P>
    bool push(
        P &&...params)
    {
        return base_t::template push<D>(std::forward<P>(params)...);
    }

    template <typename F>
    bool pop(
        D &item,
        F &&out)
    {
        return base_t::pop(item, std::forward<F>(out));
    }
};

} // namespace ipc


#endif // ! _IPC_QUEUE_H_