/*
 * ping_pong_impl.cpp
 *
 *  Created on: May 6, 2016
 *      Author: zmij
 */

#include "ping_pong_impl.hpp"

namespace wire {
namespace test {

::std::int32_t
ping_pong_server::test_int(::std::int32_t val, ::wire::core::current const&) const
{
    return val;
}

void
ping_pong_server::test_string(::std::string const& val,
        test_string_return_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&)
{
    __resp(val);
}

void
ping_pong_server::test_struct(::test::data const& val,
        test_struct_return_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&) const
{
    __resp(val);
}

void
ping_pong_server::error(::wire::core::current const&)
{
    throw ::test::oops{ "Shit happens!" };
}

void
ping_pong_server::async_error(::wire::core::callbacks::void_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&) const
{
    ::std::make_exception_ptr(::test::oops{ "Async shit happens!" });
}


}  /* namespace test */
}  /* namespace wire */


