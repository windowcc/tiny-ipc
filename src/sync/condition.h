#ifndef _IPC_SYNC_CONDITION_H_
#define _IPC_SYNC_CONDITION_H_

#include <cstdint>
#include <string.h>
#include "handle.h"
#include "ipc/def.h"

namespace ipc {
namespace detail {

class Mutex;

class Condition
{
public:
    Condition();
    ~Condition() = default;

    pthread_cond_t const *native() const noexcept;

    pthread_cond_t *native() noexcept;

    bool valid() const noexcept;

    bool init() noexcept;

    void close() noexcept;

    bool wait(
        Mutex &mtx,
        std::uint64_t tm = static_cast<uint64_t>(TimeOut::DEFAULT_TIMEOUT)) noexcept;

    bool notify(
        Mutex &) noexcept;

    bool broadcast(
        Mutex &) noexcept;
private:
    pthread_cond_t cond_;
};

} // namespace detail
} // namespace ipc

#endif // !_IPC_SYNC_CONDITION_H_