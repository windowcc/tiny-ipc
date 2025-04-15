#ifndef _IPC_SYNC_SCOPEGUARD_H_
#define _IPC_SYNC_SCOPEGUARD_H_

#include <utility>     // std::forward, std::move
#include <algorithm>   // std::swap
#include <type_traits> // std::decay

namespace ipc
{
namespace detail
{
////////////////////////////////////////////////////////////////
/// Execute guard function when the enclosing scope exits
////////////////////////////////////////////////////////////////

template <typename F>
class ScopeGuard
{
public:
    template <typename D>
    ScopeGuard(D &&destructor)
        : destructor_(std::forward<D>(destructor)), dismiss_(false)
    {
    }

    ScopeGuard(ScopeGuard &&rhs)
        : destructor_(std::move(rhs.destructor_))
        , dismiss_(true)
    {
        std::swap(dismiss_, rhs.dismiss_);
    }

    ~ScopeGuard()
    {
        try
        {
            do_exit();
        }
        catch (...)
        { /* Do nothing */
        }
    }

    void dismiss() const noexcept
    {
        dismiss_ = true;
    }

    void do_exit()
    {
        if (!dismiss_)
        {
            dismiss_ = true;
            destructor_();
        }
    }
private:
    F destructor_;
    mutable bool dismiss_;
};

template <typename D>
constexpr auto Guard(D &&destructor) noexcept
{
    return ScopeGuard<std::decay_t<D>>
    {
        std::forward<D>(destructor)
    };
}

} // namespace detail
} // namespace ipc

#endif // ! _IPC_SYNC_SCOPEGUARD_H_