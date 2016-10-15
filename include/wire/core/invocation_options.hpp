/*
 * invocation_options.hpp
 *
 *  Created on: 15 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_INVOCATION_OPTIONS_HPP_
#define WIRE_CORE_INVOCATION_OPTIONS_HPP_

#include <functional>

namespace wire {
namespace core {

enum class invocation_flags {
    none        = 0x00,
    sync        = 0x01,
    one_way     = 0x02,
    unspecified = 0x10
};

inline invocation_flags
operator | (invocation_flags lhs, invocation_flags rhs)
{
    using integral_type = ::std::underlying_type<invocation_flags>::type;
    return static_cast<invocation_flags>( static_cast< integral_type >(lhs) | static_cast< integral_type >(rhs) );
}

inline invocation_flags
operator & (invocation_flags lhs, invocation_flags rhs)
{
    using integral_type = ::std::underlying_type<invocation_flags>::type;
    return static_cast<invocation_flags>( static_cast< integral_type >(lhs) & static_cast< integral_type >(rhs) );
}

inline invocation_flags
operator ^ (invocation_flags lhs, invocation_flags rhs)
{
    using integral_type = ::std::underlying_type<invocation_flags>::type;
    return static_cast<invocation_flags>( static_cast< integral_type >(lhs) ^ static_cast< integral_type >(rhs) );
}

inline bool
any(invocation_flags v)
{
    return v != invocation_flags::none;
}

struct invocation_options {
    using timeout_type          = ::std::size_t;
    static constexpr timeout_type default_timout = 5000;
    static const invocation_options unspecified;

    invocation_flags    flags   = invocation_flags::none;
    timeout_type        timeout = default_timout;

    constexpr invocation_options() {}
    constexpr invocation_options(invocation_flags f)
        : flags{f} {}
    constexpr invocation_options(invocation_flags f, timeout_type t)
        : flags{f}, timeout{t} {}

    bool
    operator == (invocation_options const& rhs) const
    {
        return flags == rhs.flags && timeout == rhs.timeout;
    }
    bool
    operator != (invocation_options const& rhs) const
    {
        return !(*this == rhs);
    }

    bool
    is_sync() const
    {
        return any(flags & invocation_flags::sync);
    }

    invocation_options
    with_timeout(timeout_type t) const
    {
        return invocation_options{ flags, t };
    }
};

inline invocation_flags&
operator |= (invocation_flags& lhs, invocation_flags rhs)
{
    return lhs = lhs | rhs;
}

inline invocation_flags&
operator &= (invocation_flags& lhs, invocation_flags rhs)
{
    return lhs = lhs & rhs;
}

inline invocation_flags&
operator ^= (invocation_flags& lhs, invocation_flags rhs)
{
    return lhs = lhs ^ rhs;
}

inline bool operator ! (invocation_flags v)
{
    return v == invocation_flags::none;
}

inline invocation_options
operator | (invocation_options const& lhs, invocation_flags rhs)
{
    return invocation_options{ lhs.flags | rhs, lhs.timeout };
}

inline invocation_options
operator & (invocation_options const& lhs, invocation_flags rhs)
{
    return invocation_options{ lhs.flags & rhs, lhs.timeout };
}

inline invocation_options
operator ^ (invocation_options const& lhs, invocation_flags rhs)
{
    return invocation_options{ lhs.flags ^ rhs, lhs.timeout };
}

inline invocation_options
operator + (invocation_options const& lhs, invocation_options::timeout_type t)
{
    return invocation_options{ lhs.flags, lhs.timeout + t };
}

inline invocation_options
operator - (invocation_options const& lhs, invocation_options::timeout_type t)
{
    return invocation_options{ lhs.flags, lhs.timeout - t };
}

namespace functional {
using invocation_function   = ::std::function< void(invocation_options const&) >;
}  /* namespace functional */

}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_INVOCATION_OPTIONS_HPP_ */
