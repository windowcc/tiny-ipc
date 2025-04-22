#ifndef _IPC_DEF_H_
#define _IPC_DEF_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <utility>

namespace ipc {

// constants

enum : unsigned
{
    SENDER = 1,
    RECEIVER
};

enum class ErrorCode : uint8_t
{
	IPC_ERR_SUCCESS = 0,
    IPC_ERR_NOINIT,
	IPC_ERR_NOMEM,
	IPC_ERR_INVAL,
	IPC_ERR_NO_CONN,
	IPC_ERR_CONN_REFUSED,
	IPC_ERR_NOT_FOUND,
	IPC_ERR_CONN_LOST,
	IPC_ERR_NOT_SUPPORTED,
	IPC_ERR_UNKNOWN,
};

enum class TimeOut : std::uint64_t
{
    DEFAULT_TIMEOUT = 100, // ms
    INVALID_TIMEOUT = (std::numeric_limits<std::uint64_t>::max)(),
};

enum class Transmission : uint32_t
{
    UNICAST,
    BROADCAST
};

#define ALIGN_64 (64)

// producer-consumer policy flag
template <Transmission Ts>
struct Wr
{
    constexpr static bool is_broadcast = (Ts == Transmission::BROADCAST);
};

} // namespace ipc

#endif // ! _IPC_DEF_H_