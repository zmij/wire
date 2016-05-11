/*
 * ping_pong_impl.cpp
 *
 *  Created on: May 6, 2016
 *      Author: zmij
 */

#include "ping_pong_impl.hpp"
#include <iostream>

namespace wire {
namespace test {

::std::int32_t
ping_pong_server::test_int(::std::int32_t val, ::wire::core::current const&) const
{
    ::std::cerr << __FUNCTION__ << "\n";
    return val;
}

void
ping_pong_server::test_string(::std::string const& val,
        test_string_return_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&)
{
    ::std::cerr << __FUNCTION__ << "\n";
    __resp(val);
}

void
ping_pong_server::test_struct(::test::data const& val,
        test_struct_return_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&) const
{
    ::std::cerr << __FUNCTION__ << " " << val << "\n";
    __resp(val);
}

void
ping_pong_server::test_callback(::test::callback_prx cb,
        test_callback_return_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&)
{
    ::std::cerr << __FUNCTION__ << " " << *cb << "\n";
    __resp(cb);
}

void
ping_pong_server::test_ball(::test::ball_ptr b,
            test_ball_return_callback __resp,
            ::wire::core::callbacks::exception_callback __exception,
            ::wire::core::current const&)
{
    ::std::cerr << __FUNCTION__ << " ";
    if (b) {
        ::std::cerr << "Ball size " << b->size << "\n";
    } else {
        ::std::cerr << "No ball\n";
    }
    __resp(b);
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
    ::std::cerr << __FUNCTION__ << "\n";
    __exception(::std::make_exception_ptr(::test::oops{ "Async shit happens!" }));
}

void
ping_pong_server::stop(::wire::core::callbacks::void_callback __resp,
        ::wire::core::callbacks::exception_callback __exception,
        ::wire::core::current const&)
{
    ::std::cerr << __FUNCTION__ << "\n";
    __resp();
    if (on_stop_)
        on_stop_();
}

}  /* namespace test */
}  /* namespace wire */


