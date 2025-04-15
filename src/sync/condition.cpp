#include "condition.h"
#include "guard.h"
#include "mutex.h"

namespace ipc
{
namespace detail
{

Condition::Condition()
    : cond_()
{
}

pthread_cond_t const *Condition::native() const noexcept
{
    return &cond_;
}

pthread_cond_t *Condition::native() noexcept
{
    return &cond_;
}

bool Condition::valid() const noexcept
{
    static const char tmp[sizeof(pthread_cond_t)]{};
    return (memcmp(tmp, &cond_, sizeof(pthread_cond_t)) != 0);
}

bool Condition::init() noexcept
{
    close();
    ::pthread_cond_destroy(&cond_);
    auto finally = Guard([this]
    {
        close(); 
    }); // close when failed

    // init Condition
    int eno;
    pthread_condattr_t cond_attr;
    if ((eno = ::pthread_condattr_init(&cond_attr)) != 0)
    {
        return false;
    }
    auto guard_cond_attr = Guard([&cond_attr]
    {
        ::pthread_condattr_destroy(&cond_attr);
    });

    if ((eno = ::pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED)) != 0)
    {
        return false;
    }
    cond_ = PTHREAD_COND_INITIALIZER;
    if ((eno = ::pthread_cond_init(&cond_, &cond_attr)) != 0)
    {
        return false;
    }
    finally.dismiss();
    return valid();
}

void Condition::close() noexcept
{
    int eno;
    if ((eno = ::pthread_cond_destroy(&cond_)) != 0)
    {
    }
}

bool Condition::wait(
    Mutex &mtx,
    std::uint64_t tm) noexcept
{
    if (!valid())
        return false;
    switch (tm)
    {
    case static_cast<long unsigned int>(TimeOut::INVALID_TIMEOUT):
    {
        int eno;
        if ((eno = ::pthread_cond_wait(&cond_, static_cast<pthread_mutex_t *>(mtx.native()))) != 0)
        {
            return false;
        }
    }
    break;
    default:
    {
        auto ts = make_timespec(tm);
        int eno;
        if ((eno = ::pthread_cond_timedwait(&cond_, static_cast<pthread_mutex_t *>(mtx.native()), &ts)) != 0)
        {
            if (eno != ETIMEDOUT)
            {
            }
            return false;
        }
    }
    break;
    }
    return true;
}

bool Condition::notify(
    Mutex &) noexcept
{
    if (!valid())
        return false;
    int eno;
    if ((eno = ::pthread_cond_signal(&cond_)) != 0)
    {
        return false;
    }
    return true;
}

bool Condition::broadcast(
    Mutex &) noexcept
{
    if (!valid())
        return false;
    int eno;
    if ((eno = ::pthread_cond_broadcast(&cond_)) != 0)
    {
        return false;
    }
    return true;
}

} // namespace detail
} // namespace ipc
