#ifndef _IPC_POLICY_H_
#define _IPC_POLICY_H_

#include <type_traits>
#include "core/content.h"
#include "core/segment.hpp"

namespace ipc
{
namespace detail
{

template <template <typename, std::size_t...> class Segment>
struct Choose;

template<>
struct Choose<Segment>
{
    template <std::size_t DataSize, std::size_t AlignSize>
    using segment_t = Segment<Content, DataSize, AlignSize>;
};

} // namespace detail
} // namespace ipc

#endif // ! _IPC_POLICY_H_