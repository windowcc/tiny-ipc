#include "ipc/buffer.h"
#include <cstring>

namespace ipc
{

bool operator==(
    Buffer const &b1,
    Buffer const &b2)
{
    return (b1.size() == b2.size()) && (std::memcmp(b1.data(), b2.data(), b1.size()) == 0);
}

bool operator!=(
    Buffer const &b1,
    Buffer const &b2)
{
    return !(b1 == b2);
}

class Buffer::BufferImpl
{
    friend class Buffer;
public:
    BufferImpl(
        void *p,
        std::size_t s,
        Buffer::destructor_t d,
        void *a)
        : p_(p)
        , s_(s)
        , a_(a)
        , d_(d)
    {
    }

    ~BufferImpl()
    {
        if (d_ == nullptr)
            return;
        d_(a_ == nullptr ? p_ : a_, s_);
    }
private:
    void *p_;
    std::size_t s_;
    void *a_;
    Buffer::destructor_t d_;
};

template <std::size_t N>
Buffer::Buffer(
    uint8_t const (&data)[N])
    : Buffer(data, sizeof(data))
{
}

Buffer::Buffer()
    : Buffer(nullptr, 0, nullptr)
{
}

Buffer::Buffer(
    void *p,
    std::size_t s,
    destructor_t d)
    : impl_(std::make_unique<BufferImpl>(p, s, d, nullptr))
{
}


Buffer::Buffer(
    void *p,
    std::size_t s)
    : Buffer(p, s, nullptr)
{
}

Buffer::Buffer(
    char const &c)
    : Buffer(const_cast<char *>(&c), 1)
{
}

Buffer::Buffer(
    Buffer &&rhs)
    : Buffer()
{
    swap(rhs);
}

Buffer::~Buffer()
{

}

void Buffer::swap(
    Buffer &rhs)
{
    std::swap(impl_, rhs.impl_);
}

Buffer &Buffer::operator=(
    Buffer rhs)
{
    swap(rhs);
    return *this;
}

bool Buffer::empty() const noexcept
{
    return (impl_->p_ == nullptr) || (impl_->s_ == 0);
}

void *Buffer::data() noexcept
{
    return impl_->p_;
}

void const *Buffer::data() const noexcept
{
    return impl_->p_;
}

std::size_t Buffer::size() const noexcept
{
    return impl_->s_;
}

} // namespace ipc
