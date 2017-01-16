/*
 * bus_test_intf_impl.hpp
 *
 *  Created on: Jan 16, 2017
 *      Author: zmij
 */

#ifndef BUS_BUS_TEST_INTF_IMPL_HPP_
#define BUS_BUS_TEST_INTF_IMPL_HPP_

#include <wire/core/functional.hpp>
#include <test/bus_test_interface.hpp>

namespace wire {
namespace test {

class subscriber : public events {
public:
    subscriber(core::functional::void_callback cb)
        : cb_{cb} {}
    void
    event(core::current const&) const override
    {
        if (cb_) cb_();
    }
private:
    core::functional::void_callback cb_;
};

}   // test
}   // wire

#endif /* BUS_BUS_TEST_INTF_IMPL_HPP_ */
