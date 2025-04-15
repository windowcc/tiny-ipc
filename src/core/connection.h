#ifndef _IPC_CORE_CONNECTION_H_
#define _IPC_CORE_CONNECTION_H_

#include <config.h>
#include <bitset>
#include <mutex>
#include "ipc/def.h"
#include "sync/spinlock.h"

namespace ipc
{
namespace detail
{

class Connection
{
public:
    Connection();

    ~Connection();
public:
    uint32_t connect(
        const unsigned &mode = SENDER) noexcept;

    uint32_t disconnect(
        const unsigned &mode = SENDER,
        uint32_t cc_id = 0) noexcept;

    uint32_t connections() noexcept;

    uint32_t recv_count() noexcept;

protected:
    SpinLock lcc_;
    std::bitset<MAX_CONNECTIONS> cc_;
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_CORE_CONNECTION_H_