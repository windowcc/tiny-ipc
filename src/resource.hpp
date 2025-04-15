#ifndef _IPC_RESOURCE_H_
#define _IPC_RESOURCE_H_

#include <string>
#include <system_error>
#include <sys/time.h>

namespace ipc {

/**
 * @brief Check string validity.
 * 
 * @param str 
 * @return true 
 * @return false 
 */
constexpr bool is_valid_string(char const *str) noexcept
{
    return (str != nullptr) && (str[0] != '\0');
}

/**
 * @brief Make a valid string.
 * 
 * @param str 
 * @return std::string 
 */
inline std::string make_string(char const *str)
{
    return is_valid_string(str) ? std::string{str} : std::string{};
}

/**
 * @brief Combine prefix from a list of strings.
 * 
 * @param prefix 
 * @param args 
 * @return std::string 
 */
inline std::string make_prefix(std::string prefix, std::initializer_list<std::string> args)
{
    prefix += "tiny_ipc_";
    for (auto const &txt: args) {
        if (txt.empty()) continue;
        prefix += txt;
    }
    return prefix;
}

inline timespec make_timespec(std::uint64_t tm /*ms*/) noexcept(false)
{
    timespec ts {};
    timeval now;
    if (::gettimeofday(&now, NULL) != 0)
    {
        throw std::system_error{static_cast<int>(errno), std::system_category()};
    }
    ts.tv_nsec = (now.tv_usec + (tm % 1000) * 1000) * 1000;
    ts.tv_sec  =  now.tv_sec  + (tm / 1000) + (ts.tv_nsec / 1000000000l);
    ts.tv_nsec %= 1000000000l;

    return ts;
}

inline std::size_t align_size(const std::size_t &size, const std::size_t &align)
{
    return (size + align - 1) & ~(align - 1);
}

} // namespace ipc

#endif // ! _IPC_RESOURCE_H_