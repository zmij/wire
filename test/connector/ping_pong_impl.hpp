/*
 * ping_pong_impl.hpp
 *
 *  Created on: May 6, 2016
 *      Author: zmij
 */

#ifndef CONNECTOR_PING_PONG_IMPL_HPP_
#define CONNECTOR_PING_PONG_IMPL_HPP_

#include <test/ping_pong.hpp>

namespace wire {
namespace test {

class ping_pong_server : public ::test::ping_pong {
public:
    ::std::int32_t
    test_int(::std::int32_t val,
            ::wire::core::current const& = ::wire::core::no_current) const override;

    void
    test_string(::std::string const& val,
            test_string_return_callback __resp,
            ::wire::core::callbacks::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) override;

    void
    test_struct(::test::data const& val,
            test_struct_return_callback __resp,
            ::wire::core::callbacks::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) const override;

    void
    error(::wire::core::current const& = ::wire::core::no_current) override;

    void
    async_error(::wire::core::callbacks::void_callback __resp,
            ::wire::core::callbacks::exception_callback __exception,
            ::wire::core::current const& = ::wire::core::no_current) const override;
};

}  /* namespace test */
}  /* namespace wire */

#endif /* CONNECTOR_PING_PONG_IMPL_HPP_ */
