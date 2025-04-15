#include "description.h"

namespace ipc
{
namespace detail
{

Description::Description()
{

}

Description::Description(
    const uint32_t &id,
    const std::size_t &offset,
    const std::size_t &len)
    : id_(id)
    , offset_(offset)
    , length_(len)
{

}

Description::~Description()
{

}

void Description::id(
    const uint32_t &id)
{
    id_ = id;
}

uint32_t Description::id() const
{
    return id_;
}

void Description::offset(
    const std::size_t &offset)
{
    offset_ = offset;
}

std::size_t Description::offset() const
{
    return offset_;
}

void Description::length(
    const std::size_t &length)
{
    length_ = length;
}

std::size_t Description::length() const
{
    return length_;
}


} // namespace detail
} // namespace ipc