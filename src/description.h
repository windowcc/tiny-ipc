#ifndef _IPC_MESSAGE_H_
#define _IPC_MESSAGE_H_

#include <thread>

#include "ipc/def.h"


namespace ipc
{
namespace detail
{

class Description
{
public:
    Description();
    Description(
        const uint32_t &id,
        const std::size_t &offset,const std::size_t &len);
    ~Description();
public:
    void id(
        const uint32_t &id);
    uint32_t id() const;

    void offset(
        const std::size_t &offset);
    std::size_t offset() const;

    void length(
        const std::size_t &size);
    std::size_t length() const;

private:
    // data storage file name generate
    uint32_t id_;
    // data offset
    std::size_t offset_;
    // data length
    std::size_t length_;
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_MESSAGE_H_