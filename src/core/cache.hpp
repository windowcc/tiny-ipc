#ifndef _IPC_CORE_Cache_H_
#define _IPC_CORE_Cache_H_

#include <config.h>
#include <thread>
#include <type_traits>
#include <sstream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <memory_resource>

#include "handle.h"
#include "sync/spinlock.h"

namespace ipc
{
namespace detail
{

static constexpr std::size_t DEFAULT_CACHE_SIZE = 1024 * 1024 * 1024; // 1G
static const std::string DEFAULT_SHM_NAME = "tiny_ipc_";
static constexpr uint32_t DEFAULT_TIMEOUT_VALUE = 10 * 1000; // mill

static std::string thread_id_to_string(const uint32_t &id)
{
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

// Just need to add a lock on the writer end
// When releasing memory, it will first determine whether the usage count of the current memory segment has been reset.
// Therefore, there is no need to perform a locking operation
class Sender
{
public:
    Sender()
        : handle_ {}
        , pool_ {nullptr}
    {
        auto thread_id = std::this_thread::get_id();
        id_ = *(uint32_t*)&thread_id;


        auto name = DEFAULT_SHM_NAME + thread_id_to_string(id_);
        if (!handle_.acquire(name.c_str(), DEFAULT_CACHE_SIZE))
        {
            return ;
        }

        // using monotonic_buffer_resource = std::pmr::monotonic_buffer_resource;
        pool_ = std::make_shared<std::pmr::monotonic_buffer_resource>(handle_.get(),handle_.size(),
                    std::pmr::null_memory_resource());
    }
    ~Sender()
    {
        // free all apply memory
        // There may be some memory that has not been read properly
        for (auto &[key,value] : map_)
        {
            pool_->deallocate(key,std::get<0>(value));
        }
        map_.clear();
    }

public:
    std::shared_ptr<void> write(
        void const *data,
        const std::size_t &size,
        const uint32_t &cnt)
    {
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        recyle_memory(now);

        auto pool_size = align_size(size + sizeof(uint32_t), alignof(std::max_align_t));
        void *pool_data = nullptr;

        {
            std::lock_guard<SpinLock> l(lock_);
            pool_data = pool_->allocate(pool_size);
        }

        if(!pool_data)
        {
            return {};
        }

        std::atomic<uint32_t> *count = static_cast<std::atomic<uint32_t>*>(pool_data);
        count->store(cnt,std::memory_order_relaxed);
        memcpy(static_cast<char*>(pool_data) + sizeof(uint32_t),data,size);
        
        map_.emplace(pool_data, std::make_pair(pool_size,now));

        return std::make_shared<Description>
        (
            id_,
            reinterpret_cast<std::size_t>(pool_data) - reinterpret_cast<std::size_t>(handle_.get()),
            pool_size
        );
    }

private:
    void recyle_memory(
        const std::chrono::time_point<std::chrono::steady_clock> &now)
    {
        if(map_.empty())
        {
            return;
        }
        for (auto it = map_.begin(); it != map_.end();)
        {
            std::atomic<uint32_t> *count = static_cast<std::atomic<uint32_t>*>(it->first);
            auto &[key,value] = it->second;
            if(!count->load() ||
                (std::chrono::duration_cast<std::chrono::milliseconds>(now - value).count() >= DEFAULT_TIMEOUT_VALUE))
            {
                pool_->deallocate(it->first,key);
                it = map_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
private:
    // thread id
    uint32_t id_;
    // shm handle
    Handle handle_;
    SpinLock lock_;
    // shared memory manager
    std::shared_ptr<std::pmr::monotonic_buffer_resource> pool_;
    // already memory using map
    std::unordered_map<void*,
        std::pair<std::size_t,std::chrono::time_point<std::chrono::steady_clock>>> map_;
};


class Receiver
{
public:
    Receiver()
        : handles_()
    {
        
    }
    ~Receiver()
    {
        handles_.clear();
    }
public:
    bool read(
        const Description &desc,
        std::function<void(const Buffer *)> callback)
    {
        Handle *handle = get_handle(desc.id());
        if(!handle || !callback)
        {
            return false;
        }
        void *pool_data = static_cast<char*>(handle->get()) + desc.offset();

        Buffer buf(static_cast<char*>(pool_data) + sizeof(uint32_t), desc.length() - sizeof(uint32_t));
        if(!buf.empty())
        { 
            callback(&buf);
        }
        static_cast<std::atomic<uint32_t>*>(pool_data)->fetch_sub(1, std::memory_order_relaxed);
        return !(*static_cast<std::atomic<uint32_t>*>(pool_data));
    }

private:
    Handle *get_handle(
        const uint32_t &id)
    {
        auto it = handles_.find(id);
        if(it == handles_.end())
        {
            Handle handle;
            auto name = DEFAULT_SHM_NAME + thread_id_to_string(id);
            if (!handle.acquire(name.c_str(), DEFAULT_CACHE_SIZE, open))
            {
                return nullptr;
            }
            else
            {
                handles_.insert(
                    {id, std::move(handle)}
                );
                return &(handles_[id]);
            }
        }
        else
        {
            return &(it->second);
        }
    }

private:
    std::unordered_map<uint32_t,Handle> handles_;
};

template<typename T>
struct is_sender{};

template<>
struct is_sender<Sender>
{
    static const bool value = true;
};

template<>
struct is_sender<Receiver>
{
    static const bool value = false;
};

template<typename T>
class Cache
{
public:
    Cache()
        :cache_ {std::make_unique<T>()}
    {
    }

    ~Cache()
    {
    }
public:
    template <typename U = T>
    auto write(
        void const *data,
        const std::size_t &size,
        const uint32_t &cnt) 
        -> std::enable_if_t<ipc::detail::is_sender<U>::value, std::shared_ptr<void>>
    {
        return cache_->write(data,size,cnt);
    }

    template <typename U = T>
    auto read(
        const Description &desc,
        std::function<void(const Buffer *)> callback)
        -> std::enable_if_t<!ipc::detail::is_sender<U>::value, bool>
    {
        return cache_->read(desc,callback);
    }

private:
    std::unique_ptr<T> cache_;
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_CORE_Cache_H_