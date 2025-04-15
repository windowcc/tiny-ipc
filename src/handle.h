#ifndef _IPC_HANDLE_H_
#define _IPC_HANDLE_H_

#include <cstddef>
#include <cstdint>
#include "resource.hpp"

namespace ipc
{
namespace detail
{

enum : unsigned
{
    create = 0x01,
    open = 0x02
};

class Handle
{
public:
    Handle();
    Handle(
        char const *name,
        std::size_t size,
        unsigned mode = create | open);

    Handle(
        Handle &&rhs);
    Handle &operator=(
        Handle rhs);

    ~Handle();

public:
    bool valid() noexcept;
    std::size_t size() const noexcept;
    char const *name() const noexcept;

    std::int32_t ref() const noexcept;
    void sub_ref() noexcept;

    bool acquire(
        char const *name,
        std::size_t size,
        unsigned mode = create | open);
    
    std::int32_t release();

    void *get();

private:
    int fd_ = -1;
    void *mem_ = nullptr;
    std::size_t size_ = 0;
    std::string name_;
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_HANDLE_H_