
#include "handle.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <atomic>
#include <string>
#include <utility>
#include <cstring>

namespace
{

struct info_t
{
    std::atomic<std::int32_t> acc_;
};

inline auto &acc_of(
    void *mem,
    std::size_t size)
{
    return reinterpret_cast<info_t *>(static_cast<uint8_t *>(mem) + size - sizeof(info_t))->acc_;
}

} // internal-linkage

namespace ipc
{
namespace detail
{

Handle::Handle()
    : fd_(-1)
    , mem_(nullptr)
    , size_(0)
    , name_()
{
}

Handle::Handle(
    char const *name,
    std::size_t size,
    unsigned mode)
    : Handle()
{
    acquire(name, size, mode);
}

Handle::Handle(
    Handle &&rhs)
    : Handle()
{
    fd_ = rhs.fd_;
    rhs.fd_ = -1;

    mem_ = rhs.mem_;
    rhs.mem_ = nullptr;

    size_ = rhs.size_;
    rhs.size_ = 0;

    name_ = std::move(rhs.name_);
}

Handle::~Handle()
{
    release();
}


Handle &Handle::operator=(
    Handle rhs)
{
    fd_ = rhs.fd_;
    rhs.fd_ = -1;
    mem_ = rhs.mem_;
    rhs.mem_ = nullptr;
    size_ = rhs.size_;
    name_ = std::move(rhs.name_);

    return *this;
}

bool Handle::valid() noexcept
{
    return get() != nullptr;
}

std::size_t Handle::size() const noexcept
{
    return size_;
}

char const *Handle::name() const noexcept
{
    return name_.c_str();
}

std::int32_t Handle::ref() const noexcept
{
    if (mem_ == nullptr || size_ == 0)
    {
        return 0;
    }
    return acc_of(mem_, size_).load(std::memory_order_acquire);
}

void Handle::sub_ref() noexcept
{
    if (mem_ == nullptr || size_ == 0)
    {
        return;
    }
    acc_of(mem_, size_).fetch_sub(1, std::memory_order_acq_rel);
}

bool Handle::acquire(
    char const *name,
    std::size_t size,
    unsigned mode)
{
    release();

    if (!is_valid_string(name))
    {
        return false;
    }
    // For portable use, a shared memory object should be identified by name of the form /somename.
    std::string op_name = std::string{"/"} + name;
    // Open the object for read-write access.
    int flag = O_RDWR;
    switch (mode)
    {
    case open:
        size = 0;
        break;
    // The check for the existence of the object,
    // and its creation if it does not exist, are performed atomically.
    case create:
        flag |= O_CREAT | O_EXCL;
        break;
    // Create the shared memory object if it does not exist.
    default:
        flag |= O_CREAT;
        break;
    }
    int fd = ::shm_open(op_name.c_str(), flag, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1)
    {
        return false;
    }

    fd_ = fd;
    size_ = size;
    name_ = std::move(op_name);

    return valid();
}

std::int32_t Handle::release()
{
    std::int32_t ret = -1;
    if (mem_ == nullptr || size_ == 0)
    {
    }
    else if ((ret = acc_of(mem_, size_).fetch_sub(1, std::memory_order_acq_rel)) <= 1)
    {
        ::munmap(mem_, size_);
        if (!name_.empty())
        {
            ::shm_unlink(name_.c_str());
        }
    }
    else
    {
        ::munmap(mem_, size_);
    }

    fd_ = -1;
    mem_ = nullptr;
    size_ = 0;
    name_.clear();
    
    return ret;    
}

void *Handle::get()
{
    if (mem_ != nullptr)
    {
        return mem_;
    }
    int fd = fd_;
    if (fd == -1)
    {
        return nullptr;
    }
    if (size_ == 0)
    {
        struct stat st;
        if (::fstat(fd, &st) != 0)
        {
            return nullptr;
        }
        size_ = static_cast<std::size_t>(st.st_size);
        if ((size_ <= sizeof(info_t)) || (size_ % sizeof(info_t)))
        {
            return nullptr;
        }
    }
    else
    {
        size_ = [](const std::size_t &size){
            return ((((size - 1) / alignof(info_t)) + 1) * alignof(info_t)) + sizeof(info_t);
        }(size_);

        if (::ftruncate(fd, static_cast<off_t>(size_)) != 0)
        {
            return nullptr;
        }
    }
    void *mem = ::mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED)
    {
        return nullptr;
    }
    ::close(fd);
    fd_ = -1;
    mem_ = mem;
    acc_of(mem, size_).fetch_add(1, std::memory_order_release);
    return mem;
}

} // namespace detail
} // namespace ipc