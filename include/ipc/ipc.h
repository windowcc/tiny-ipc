#ifndef _IPC_IPC_H_
#define _IPC_IPC_H_

#include <string>
#include <memory>
#include <functional>
#include "ipc/export.h"
#include "ipc/def.h"
#include "ipc/buffer.h"
#include "ipc/callback.h"

namespace ipc
{

template <typename Wr>
class Ipc
{
public:
    Ipc() noexcept = default;
    explicit Ipc(
        char const * name,
        const unsigned &mode = SENDER);
    Ipc(
        Ipc&& rhs) noexcept;

    ~Ipc();

    Ipc& operator=(
        Ipc rhs) noexcept;
public:
    char const * name() const noexcept;

    bool valid() const noexcept;

    unsigned mode() const noexcept;

    bool connect(
        char const * name,
        const unsigned &mode = SENDER);

    bool reconnect(
        unsigned mode);

    void disconnect();

    void set_callback(
        CallbackPtr callback_ptr);

    bool is_connected() const noexcept;

    bool write(
        void const * data,
        std::size_t size);

    bool write(
        Buffer const & buff);

    bool write(
        std::string const & str);

    void read(
        std::uint64_t tm = static_cast<uint64_t>(TimeOut::INVALID_TIMEOUT));

private:
    struct IpcImpl;
    std::unique_ptr<IpcImpl> impl_;
};

/**
 * @brief channel
 * 
 * @note You could use multi writers for sending messages to a route,
 *       then all the readers which are receiving with this route, would receive your sent messages.
 *       A route could be used in N to N .
 */
using Route = Ipc<Wr<Transmission::BROADCAST>>;


/**
 * @brief channel
 * 
 * @note You could use multi writers for sending messages to a channel,
 *       then only reader which are receiving with this channel, would receive your sent messages.
 *       A channel could only be used in N to 1 .
 */
using Channel = Ipc<Wr<Transmission::UNICAST>>;

} // namespace ipc


#endif // ! _IPC_IPC_H_