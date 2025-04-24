#ifndef _IPC_QUEUE_H_
#define _IPC_QUEUE_H_

#include <cassert>
#include <type_traits>
#include "handle.h"
#include "resource.hpp"
#include "sync/waiter.h"

namespace ipc
{
namespace detail
{

template <typename Segment>
class QueueBase
{
public:
    using segment_t = Segment;
    QueueBase() = default;

    explicit QueueBase(
        char const *name)
        : QueueBase{}
    {
    }

    virtual ~QueueBase()
    {

        if(handle_.ref() <= 1)
        {
            segment_->waiter().close();
        }
        close();
    }

public:

    bool open(
        char const *name) noexcept
    {
        close();

        if(!is_valid_string(name) || !handle_.acquire(name,sizeof(segment_t)))
        {
            return false;
        }

        segment_ = static_cast<segment_t*>(handle_.get());

        return segment_ && segment_->init();
    }

    void close()
    {
        handle_.release();
    }

    segment_t *segment() noexcept
    { 
        return segment_;
    }

    uint32_t connected_id() const noexcept
    {
        return connected_id_;
    }

    bool connect(
        unsigned mode = RECEIVER) noexcept
    {
        if(!segment_)
        {
            return false;
        }
        if(!connected_id_)
        {
            connected_id_ = segment_->connect(mode);
        }

        cursor_ = segment_->rd();
        sender_flag_ = true;

        return true;
    }

    bool disconnect() noexcept
    {
        waiter()->quit();
        if (segment_ == nullptr || !connected_id())
        {
            return false;
        }

        segment_->disconnect(RECEIVER, std::exchange(connected_id_, 0));
        sender_flag_ = false;

        return true;
    }

    inline Waiter *waiter() noexcept
    {
        return &(segment_->waiter());
    }

    bool valid() const noexcept
    {
        return segment_ != nullptr;
    }

    bool empty() const noexcept
    {
        return !valid() || (cursor_ == segment_->wr());
    }

    template <typename F>
    void wait_for(
        F &&pred,
        std::uint64_t tm)
    {
        waiter()->wait_for(std::forward<F>(pred), tm);
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

private:
    uint32_t connected_id_ = 0;
    Handle handle_;
    // It is used to record the actual read subscript of the object currently being read.
    decltype(std::declval<segment_t>().rd()) cursor_ = 0;
    bool sender_flag_ = false;
    segment_t *segment_ = nullptr;
};

} // namespace detail

template <typename D, typename Choose>
class Queue : 
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