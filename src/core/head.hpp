#ifndef _IPC_CORE_ELEMDEF_H_
#define _IPC_CORE_ELEMDEF_H_

#include <cstddef>
#include <cstdint>
#include <bitset>
#include <mutex>
#include "ipc/def.h"
#include "sync/waiter.h"

namespace ipc
{
namespace detail
{

template<typename Content>
class Head
{
public:
    using base_t   = Head<Content>;
    using cursor_t = decltype(std::declval<Content>().rd());

public:
    Head() = default;
    Head(Head const &) = delete;
    Head &operator=(Head const &) = delete;

public:
    bool init()
    {
        if (!constructed_.load(std::memory_order_acquire))
        {
            auto guard = std::unique_lock(lc_);
            if (!constructed_.load(std::memory_order_relaxed))
            {
                ::new (this) Head;
                if(!waiter_.init())
                {
                    return false;
                }
                constructed_.store(true, std::memory_order_release);
            }
        }
        return true;
    }

    uint32_t connections() noexcept
    {
        return base_t::content_.connections();
    }

    uint32_t connect(
        const unsigned &mode = SENDER) noexcept
    {
        return base_t::content_.connect(mode);
    }

    uint32_t disconnect(
        const unsigned &mode = SENDER,
        uint32_t cc_id = 0) noexcept
    {
        return base_t::content_.disconnect(mode, cc_id);
    }

    uint32_t recv_count() noexcept
    {
        return base_t::content_.recv_count();
    }

    cursor_t rd() const noexcept
    {
        return base_t::content_.rd();
    }

    cursor_t wr() const noexcept
    {
        return base_t::content_.wr();
    }

    inline Waiter &waiter() noexcept
    {
        return waiter_;
    }

protected:
    Waiter waiter_;
    Content content_;

    // init
    SpinLock lc_;
    std::atomic<bool> constructed_{false};
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_CORE_ELEMDEF_H_