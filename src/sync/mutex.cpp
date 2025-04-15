#include "mutex.h"
#include <cstring>
#include <cassert>
#include <cstdint>
#include <system_error>
#include <atomic>
#include "guard.h"

namespace ipc
{
namespace detail
{

Mutex::Mutex()
    : mutex_()
{
}

pthread_mutex_t const *Mutex::native() const noexcept
{
    return &mutex_;
}

pthread_mutex_t *Mutex::native() noexcept
{
    return &mutex_;
}

bool Mutex::valid() const noexcept
{
    static const char tmp[sizeof(pthread_mutex_t)]{};
    return (std::memcmp(tmp, &mutex_, sizeof(pthread_mutex_t)) != 0);
}

bool Mutex::init() noexcept
{
    close();
    ::pthread_mutex_destroy(&mutex_);
    auto finally = Guard([this]
    {
        close();
    }); // close when failed
    // init Mutex
    int eno;
    pthread_mutexattr_t mutex_attr;
    if ((eno = ::pthread_mutexattr_init(&mutex_attr)) != 0)
    {
        return false;
    }
    auto guard_mutex_attr = Guard([&mutex_attr]
    {
        ::pthread_mutexattr_destroy(&mutex_attr); 
    });

    if ((eno = ::pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED)) != 0)
    {
        return false;
    }
    if ((eno = ::pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST)) != 0)
    {
        return false;
    }
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
    if ((eno = ::pthread_mutex_init(&mutex_, &mutex_attr)) != 0)
    {
        return false;
    }
    finally.dismiss();
    return valid();
}

void Mutex::close() noexcept
{
    int eno;
    if ((eno = ::pthread_mutex_destroy(&mutex_)) != 0)
    {
    }
}

bool Mutex::lock(
    std::uint64_t tm) noexcept
{
    if (!valid())
        return false;
    for (;;)
    {
        auto ts = make_timespec(tm);
        int eno = (tm == static_cast<uint64_t>(TimeOut::INVALID_TIMEOUT))
                        ? ::pthread_mutex_lock(&mutex_)
                        : ::pthread_mutex_timedlock(&mutex_, &ts);
        switch (eno)
        {
        case 0:
            return true;
        case ETIMEDOUT:
            return false;
        case EOWNERDEAD:
        {
            int eno2 = ::pthread_mutex_consistent(&mutex_);
            if (eno2 != 0)
            {
                return false;
            }
            int eno3 = ::pthread_mutex_unlock(&mutex_);
            if (eno3 != 0)
            {
                return false;
            }
        }
        break; // loop again
        default:
            return false;
        }
    }
}

bool Mutex::try_lock() noexcept(false)
{
    if (!valid())
        return false;
    auto ts = make_timespec(0);
    int eno = ::pthread_mutex_timedlock(&mutex_, &ts);
    switch (eno)
    {
    case 0:
        return true;
    case ETIMEDOUT:
        return false;
    case EOWNERDEAD:
    {
        int eno2 = ::pthread_mutex_consistent(&mutex_);
        if (eno2 != 0)
        {
            break;
        }
        int eno3 = ::pthread_mutex_unlock(&mutex_);
        if (eno3 != 0)
        {
            break;
        }
    }
    break;
    default:
        break;
    }
    throw std::system_error{eno, std::system_category()};
}

bool Mutex::unlock() noexcept
{
    if (!valid())
        return false;
    int eno;
    if ((eno = ::pthread_mutex_unlock(&mutex_)) != 0)
    {
        return false;
    }
    return true;
}

} // namespace detail
} // namespace ipc
