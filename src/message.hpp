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
class Message final 
    : public Queue<Description, Choose>
{
    using queue_t = Queue<Description, Choose>;
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
        if (!queue_t::valid() && 
            queue_t::open(make_prefix(prefix_,{"_",this->name_}).c_str()))
        {
            return true;
        }
        return false;
    }

    inline std::string prefix() const
    {
        return prefix_;
    }

    inline std::string name() const
    {
        return name_;
    }
private:
    std::string prefix_;
    std::string name_;
};


} // namespace detail
} // namespace ipc

#endif // ! _IPC_CONNECT_INFO_H_