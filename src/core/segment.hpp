#ifndef _IPC_CORE_ELEMARRAY_H_
#define _IPC_CORE_ELEMARRAY_H_

#include <atomic>
#include <limits>
#include <utility>
#include <type_traits>
#include "head.hpp"

namespace ipc
{
namespace detail
{

template <typename Content,
          std::size_t DataSize,
          std::size_t AlignSize = (std::min)(DataSize, alignof(std::max_align_t))>
class Segment : public Head<Content>
{
public:
    using cursor_t = decltype(std::declval<Content>().rd());
    using elem_t   = typename Content::template elem_t<DataSize, AlignSize>;

public:

    template <typename Q, typename F>
    bool push(
        Q* que,
        F&& f)
    {
        return Head<Content>::base_t::content_.push(que, std::forward<F>(f), block_);
    }

    template <typename Q, typename F, typename R>
    bool pop(
        Q* que,
        cursor_t &cur,
        F&& f,
        R&& out)
    {
        return Head<Content>::base_t::content_.pop(que, cur, std::forward<F>(f), std::forward<R>(out), block_);
    }

private:
    elem_t block_[(std::numeric_limits<uint8_t>::max)() + 1] {};
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_CORE_ELEMARRAY_H_