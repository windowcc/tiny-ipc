#include "waiter.h"
#include "mutex.h"

namespace ipc
{
namespace detail
{

Waiter::Waiter()
    : cond_()
    , mutex_()
{
}

Waiter::~Waiter()
{
    close();
}

bool Waiter::valid() const noexcept
{
    return cond_.valid() && mutex_.valid();
}

bool Waiter::init() noexcept
{
    return cond_.init() && mutex_.init();
}

void Waiter::close() noexcept
{
    cond_.close();
    mutex_.close();
}

bool Waiter::notify() noexcept
{
    return cond_.notify(mutex_);
}

bool Waiter::broadcast() noexcept
{
    return cond_.broadcast(mutex_);
}

bool Waiter::quit()
{
    return broadcast();
}

} // namespace detail
} // namespace ipc
