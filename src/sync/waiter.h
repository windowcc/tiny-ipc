#ifndef _IPC_SYNC_WAITER_H_
#define _IPC_SYNC_WAITER_H_

#include <utility>
#include <string>
#include <atomic>
#include "sync/mutex.h"
#include "sync/condition.h"

namespace ipc
{
namespace detail
{

class Waiter
{
public:
    Waiter();
    ~Waiter();
public:
    bool valid() const noexcept;

    bool init() noexcept;

    void close() noexcept;

    bool notify() noexcept;

    bool broadcast() noexcept;

    bool quit();

    template <typename F>
    void wait_for(
        F &&pred,
        std::uint64_t tm = static_cast<uint64_t>(TimeOut::DEFAULT_TIMEOUT)) noexcept
    {
        std::lock_guard<Mutex> guard{ mutex_ };
        if (tm && !cond_.wait(mutex_, tm))
        {
            // std::cerr << "Condition wait has a error." << std::endl;
            return;
        }

        std::forward<F>(pred)(); 
    }

private:
    Mutex mutex_;
    Condition cond_;
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_SYNC_WAITER_H_