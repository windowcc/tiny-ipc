#ifndef _IPC_CONNECT_INFO_H_
#define _IPC_CONNECT_INFO_H_

#include "queue.hpp"
#include "description.h"
#include "sync/waiter.h"

namespace ipc
{
namespace detail
{

template <typename Choose>
class Message
{
public:
    explicit Message(
        char const *prefix,
        char const *name)
        : prefix_{make_string(prefix)}
        , name_{make_string(name)}
    { 
    }

    ~Message()
    {
    }
public:
    bool init()
    {
        if (!queue_.valid())
        {
            if(!queue_.open(make_prefix(prefix_,{"_",this->name_}).c_str()))
            {
                return false;
            }
        }
        return true;
    }

    inline std::string prefix() const
    {
        return prefix_;
    }

    inline std::string name() const
    {
        return name_;
    }

    inline auto *queue()
    {
        return &queue_;
    }

    template <typename F>
    void wait_for(
        F &&pred,
        std::uint64_t tm)
    {
        waiter()->wait_for(std::forward<F>(pred), tm);
    }

    inline Waiter *waiter() noexcept
    {
        return queue_.waiter();
    }

    void disconnect()
    {
        waiter()->quit();
        queue_.disconnect();
    }
private:
    std::string prefix_;
    std::string name_;
    Queue<Description, Choose> queue_;
};


} // namespace detail
} // namespace ipc

#endif // ! _IPC_CONNECT_INFO_H_