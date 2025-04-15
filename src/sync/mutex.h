#ifndef _IPC_SYNC_MUTEX_H_
#define _IPC_SYNC_MUTEX_H_

#include <cstdint>
#include <mutex>
#include <system_error>
#include "ipc/def.h"
#include "handle.h"

namespace ipc
{
namespace detail
{

class Mutex
{
public:
    Mutex();
    ~Mutex() = default;

    pthread_mutex_t const *native() const noexcept;

    pthread_mutex_t *native() noexcept;

    bool valid() const noexcept;

    bool init() noexcept;

    void close() noexcept;

    bool lock(
        uint64_t tm = static_cast<uint64_t>(TimeOut::INVALID_TIMEOUT)) noexcept;

    bool try_lock() noexcept(false);

    bool unlock() noexcept;
private:
    pthread_mutex_t mutex_;
};

} // namespace detail
} // namespace ipc

#endif // _IPC_SYNC_MUTEX_H_