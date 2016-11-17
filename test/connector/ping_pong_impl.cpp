/*
 * ping_pong_impl.cpp
 *
 *  Created on: May 6, 2016
 *      Author: zmij
 */

#include "ping_pong_impl.hpp"

#include <wire/core/invocation.hpp>
#include <iostream>

namespace wire {
namespace test {

static_assert(!core::detail::is_sync_dispatch<decltype(&::test::ping_pong::test_string)>::value,
        "test_string is async");
static_assert(core::detail::is_sync_dispatch<decltype(&::test::ping_pong::test_int)>::value,
        "test_int is sync");
static_assert(!core::detail::is_sync_dispatch<decltype(&::test::ping_pong::async_error)>::value,
        "test_string is async");
static_assert(::std::is_same<
        ::psst::meta::function_traits<decltype(&::test::ping_pong::async_error)>::class_type,
        ::test::ping_pong>::value, "Correct class owner");

::std::int32_t
ping_pong_server::test_int(::std::int32_t val, ::wire::core::current const&) const
{
    #if DEBUG_OUTPUT >= 3
    ::std::ostringstream os;
    os << ::getpid() << " " << __FUNCTION__ << "\n";
    ::std::cerr << os.str();
    #endif
    return val;
}

void
ping_pong_server::test_string(::std::string const& val,
        test_string_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    #if DEBUG_OUTPUT >= 3
    ::std::ostringstream os;
    os << ::getpid() << " " << __FUNCTION__ << "\n";
    ::std::cerr << os.str();
    #endif
    __resp(val);
}

void
ping_pong_server::test_struct(::test::data const& val,
        test_struct_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&) const
{
    #if DEBUG_OUTPUT >= 3
    ::std::ostringstream os;
    os << ::getpid() << " " << __FUNCTION__ << " " << val << "\n";
    ::std::cerr << os.str();
    #endif
    __resp(val);
}

void
ping_pong_server::test_callback(::test::ping_pong::callback_prx cb,
        test_callback_return_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    #if DEBUG_OUTPUT >= 3
    ::std::ostringstream os;
    os << ::getpid() << " " << __FUNCTION__ << " " << *cb << "\n";
    ::std::cerr << os.str();
    #endif
    __resp(cb);
}

void
ping_pong_server::test_ball(::test::ball_ptr b,
            test_ball_return_callback __resp,
            ::wire::core::functional::exception_callback __exception,
            ::wire::core::current const&)
{
    ::std::ostringstream os;
    #if DEBUG_OUTPUT >= 3
    os << ::getpid() << " " << __FUNCTION__ << " ";
    ::std::cerr << os.str();
    if (b) {
        ::std::ostringstream os;
        os << ::getpid() << " Ball size " << b->size << "\n";
        ::std::cerr << os.str();
    } else {
        ::std::ostringstream os;
        os << ::getpid() << " No ball\n";
        ::std::cerr << os.str();
    }
    #endif
    __resp(b);
}

void
ping_pong_server::error(::wire::core::current const&)
{
    throw ::test::oops{ "Shit happens!" };
}

void
ping_pong_server::async_error(::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&) const
{
    #if DEBUG_OUTPUT >= 3
    ::std::ostringstream os;
    os << ::getpid() << " " << __FUNCTION__ << "\n";
    ::std::cerr << os.str();
    #endif
    __exception(::std::make_exception_ptr(::test::oops{ "Async shit happens!" }));
}

void
ping_pong_server::stop(::wire::core::functional::void_callback __resp,
        ::wire::core::functional::exception_callback __exception,
        ::wire::core::current const&)
{
    #if DEBUG_OUTPUT >= 3
    ::std::ostringstream os;
    os << ::getpid() << " " << __FUNCTION__ << "\n";
    ::std::cerr << os.str();
    #endif
    __resp();
    if (on_stop_)
        on_stop_();
}

}  /* namespace test */
}  /* namespace wire */


