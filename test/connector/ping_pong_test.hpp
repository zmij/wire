/*
 * ping_pong_test.hpp
 *
 *  Created on: Oct 31, 2017
 *      Author: zmij
 */

#ifndef WIRE_TEST_CONNECTOR_PING_PONG_TEST_HPP_
#define WIRE_TEST_CONNECTOR_PING_PONG_TEST_HPP_

#include <test/ping_pong.hpp>
#include <wire/core/connector.hpp>

#include "sparring/sparring_test.hpp"

namespace wire {
namespace test {

class PingPong : public wire::test::sparring::SparringTest {
protected:
    void
    SetUp() override;

    void
    SetupArgs(args_type& args) override;

    void
    ReadSparringOutput(::std::istream& is) override;

    core::connector_ptr connector_;
    core::object_prx    prx_;

    static ::std::string const LIPSUM_TEST_STRING;
protected:
    void
    CheckedCast();

    void
    WireFunctions();

    void
    OneWayPing();

    void
    SyncRoundtrip();

    void
    MTConnectionUsage();

    void
    ConnectFailException();

    void
    AsyncConnectFailException();
};

} /* namespace test */
} /* namespace wire */



#endif /* WIRE_TEST_CONNECTOR_PING_PONG_TEST_HPP_ */
