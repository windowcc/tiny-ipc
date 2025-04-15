#include "content.h"

namespace ipc
{
namespace detail
{

uint32_t Content::rd() const noexcept
{
    return r_.load(std::memory_order_acquire);
}

uint32_t Content::wr() const noexcept
{
    return w_.load(std::memory_order_acquire);
}

} // namespace detail
} // namespace ipc