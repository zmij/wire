/*
 * invocation_options.hpp
 *
 *  Created on: 15 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_INVOCATION_OPTIONS_HPP_
#define WIRE_CORE_INVOCATION_OPTIONS_HPP_

#include <functional>
#include <future>

namespace wire {
namespace core {

enum class invocation_flags {
    none        = 0x00,
    sync        = 0x01,
    one_way     = 0x02,
    unspecified = 0x10
};

constexpr inline invocation_flags
operator | (invocation_flags lhs, invocation_flags rhs)
{
    using integral_type = ::std::underlying_type<invocation_flags>::type;
    return static_cast<invocation_flags>( static_cast< integral_type >(lhs) | static_cast< integral_type >(rhs) );
}

constexpr inline invocation_flags
operator & (invocation_flags lhs, invocation_flags rhs)
{
    using integral_type = ::std::underlying_type<invocation_flags>::type;
    return static_cast<invocation_flags>( static_cast< integral_type >(lhs) & static_cast< integral_type >(rhs) );
}

constexpr inline invocation_flags
operator ^ (invocation_flags lhs, invocation_flags rhs)
{
    using integral_type = ::std::underlying_type<invocation_flags>::type;
    return static_cast<invocation_flags>( static_cast< integral_type >(lhs) ^ static_cast< integral_type >(rhs) );
}

constexpr inline bool
any(invocation_flags v)
{
    return v != invocation_flags::none;
}

struct invocation_options {
    using timeout_type          = ::std::size_t;
    using retry_count_type      = int;

    static constexpr timeout_type default_timout        = 5000;

    static constexpr retry_count_type no_retries        = -1;
    static constexpr retry_count_type infinite_retries  = 0;
    static constexpr timeout_type default_retry_timout  = 1000;

    static const invocation_options unspecified;

    invocation_flags    flags           = invocation_flags::none;
    timeout_type        timeout         = default_timout;
    retry_count_type    retries         = no_retries;
    timeout_type        retry_timeout   = default_retry_timout;

    constexpr invocation_options() {}
    constexpr invocation_options(invocation_flags f)
        : flags{f} {}
    constexpr invocation_options(invocation_flags f, timeout_type t)
        : flags{f}, timeout{t} {}
    constexpr
    invocation_options(invocation_flags f, timeout_type t,
            retry_count_type r, timeout_type rt)
        : flags{f}, timeout{t}, retries{r}, retry_timeout{rt} {}

    constexpr bool
    operator == (invocation_options const& rhs) const
    {
        return flags == rhs.flags && timeout == rhs.timeout
                && retries == rhs.retries && retry_timeout == rhs.retry_timeout;
    }
    constexpr bool
    operator != (invocation_options const& rhs) const
    {
        return !(*this == rhs);
    }

    constexpr bool
    is_unspecified() const
    {
        return flags == invocation_flags::unspecified;
    }

    constexpr bool
    is_sync() const
    {
        return any(flags & invocation_flags::sync);
    }

    constexpr bool
    is_one_way() const
    {
        return any(flags & invocation_flags::one_way);
    }

    constexpr invocation_options
    with_timeout(timeout_type t) const
    {
        return invocation_options{ flags, t, retries, retry_timeout };
    }

    constexpr invocation_options
    with_retries(retry_count_type r, timeout_type rt)
    {
        return invocation_options{ flags, timeout, r, rt };
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
    return invocation_options{ lhs.flags | rhs,
        lhs.timeout, lhs.retries, lhs.retry_timeout };
}

inline invocation_options
operator & (invocation_options const& lhs, invocation_flags rhs)
{
    return invocation_options{ lhs.flags & rhs,
        lhs.timeout, lhs.retries, lhs.retry_timeout };
}

inline invocation_options
operator ^ (invocation_options const& lhs, invocation_flags rhs)
{
    return invocation_options{ lhs.flags ^ rhs,
        lhs.timeout, lhs.retries, lhs.retry_timeout };
}

inline invocation_options
operator + (invocation_options const& lhs, invocation_options::timeout_type t)
{
    return invocation_options{ lhs.flags,
        lhs.timeout + t, lhs.retries, lhs.retry_timeout };
}

inline invocation_options
operator - (invocation_options const& lhs, invocation_options::timeout_type t)
{
    return invocation_options{ lhs.flags,
        lhs.timeout - t, lhs.retries, lhs.retry_timeout };
}

template <typename _Promise>
struct promise_invocation_flags :
        ::std::integral_constant<invocation_flags, invocation_flags::none> {};

template <typename T>
struct promise_invocation_flags<::std::promise<T>> :
        ::std::integral_constant<invocation_flags, invocation_flags::sync> {};

namespace functional {
using invocation_function   = ::std::function< void(invocation_options const&) >;
}  /* namespace functional */

}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_INVOCATION_OPTIONS_HPP_ */
