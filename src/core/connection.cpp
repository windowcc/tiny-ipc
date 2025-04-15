#include "connection.h"
#include <assert.h>


namespace ipc
{
namespace detail
{

// hex receiver
// low sender

Connection::Connection()
    :cc_{}
{
    assert(!(MAX_CONNECTIONS % 2) && "MAX_CONNECTIONS is must be an even number.");
}

Connection::~Connection()
{
    std::unique_lock<SpinLock> guard(lcc_);
    cc_.reset();
}

uint32_t Connection::connect(
    const unsigned &mode) noexcept
{
    auto guard = std::unique_lock<SpinLock>(lcc_);

    uint32_t start_pos = (mode == SENDER) ? 0 : (MAX_CONNECTIONS / 2);
    uint32_t end_pos = (mode == SENDER) ?  (MAX_CONNECTIONS / 2) : MAX_CONNECTIONS;
    uint32_t cur_pos = start_pos;
    for (; cur_pos < end_pos; cur_pos++)
    {
        if(!cc_.test(cur_pos))
        {
            cc_.set(cur_pos);
            break;
        }
    }
    if(cur_pos == end_pos)
    {
        assert("The maximum subscript is exceeded.");
    }
    return cur_pos + 1;
}

uint32_t Connection::disconnect(
    const unsigned &mode,
    uint32_t cc_id) noexcept
{
    std::unique_lock<SpinLock> guard(lcc_);
    cc_.reset(cc_id - 1);
    return cc_id;
}

uint32_t Connection::Connection::connections() noexcept
{
    std::unique_lock<SpinLock> guard(lcc_);
    return cc_.count();
}

uint32_t Connection::recv_count() noexcept
{
    std::bitset<MAX_CONNECTIONS> bitset(
        std::string(MAX_CONNECTIONS / 2, '1') + std::string(MAX_CONNECTIONS / 2, '0'));
    std::unique_lock<SpinLock> guard(lcc_);
    return (bitset & cc_).count();
}

} // namespace detail
} // namespace ipc