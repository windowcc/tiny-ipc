#ifndef _IPC_BUFFER_H_
#define _IPC_BUFFER_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include "ipc/export.h"

namespace ipc {

class IPC_EXPORT Buffer
{
    using destructor_t = void (*)(void*, std::size_t);
public:
    /** Smart/shared pointer to an object of this type */
	using ptr_t = std::shared_ptr<Buffer>;
	/** Smart/shared pointer to a const object of this type */
	using const_ptr_t = std::shared_ptr<const Buffer>;

    Buffer();
    Buffer(
        void* p,
        std::size_t s);
    Buffer(
        void* p,
        std::size_t s,
        destructor_t d);

public:
    template <std::size_t N>
    explicit Buffer(
        uint8_t const (& data)[N]);

    explicit Buffer(
        char const & c);

    Buffer(
        Buffer&& rhs);

    ~Buffer();

    void swap(
        Buffer& rhs);
    Buffer& operator=(
        Buffer rhs);

    bool empty() const noexcept;

    void *       data()       noexcept;
    void const * data() const noexcept;

    template <typename T>
    inline T get() const
    {
        return T(data());
    }

    std::size_t size() const noexcept;

    friend IPC_EXPORT bool operator==(
        Buffer const & b1,
        Buffer const & b2);
        
    friend IPC_EXPORT bool operator!=(
        Buffer const & b1,
        Buffer const & b2);

private:
    class BufferImpl;
    std::unique_ptr<BufferImpl> impl_;
};

/** Smart/shared pointer to a callback object */
using BufferPtr = Buffer::ptr_t;

/** Smart/shared pointer to a const callback object */
using ConstBufferPtr = Buffer::const_ptr_t;

} // namespace ipc

#endif // ! _IPC_BUFFER_H_