/*
 * ssl_ping_pong_test.cpp
 *
 *  Created on: Oct 25, 2017
 *      Author: zmij
 */

#include <gtest/gtest.h>


#include <atomic>
#include "ping_pong_test.hpp"
#include "ping_pong_options.hpp"
#include "config.hpp"

namespace wire {
namespace test {

class SSLPingPong : public PingPong {
protected:
    void
    SetUp() override
    {
        connector_ = core::connector::create_connector(
            io_svc,
            {
                "--wire.connector.ssl.client.certificate=" + wire::test::CLIENT_CERT,
                "--wire.connector.ssl.client.key=" + wire::test::CLIENT_KEY,
                "--wire.connector.ssl.client.verify_file=" + wire::test::CA_ROOT,
                "--wire.connector.cm.request_timeout=60000"
            });
        StartPartner();
    }

    void
    SetupArgs(args_type& args) override
    {
        args.insert(args.end(), {
            "--transport=ssl",
            #if DEBUG_OUTPUT > 1
            "--port=14888",
            #endif
            "--wire.connector.ssl.server.certificate=" + wire::test::SERVER_CERT,
            "--wire.connector.ssl.server.key=" + wire::test::SERVER_KEY,
            "--wire.connector.ssl.server.verify_file=" + wire::test::CA_ROOT,
            "--wire.connector.ssl.server.reqire_peer_cert=true"
        });
    }
private:
    ::std::size_t
    ReqPerThread() const override
    {
        return ping_pong_options::instance().ssl_req_per_thread;
    }

    ::std::int64_t
    MTTestTimeout() const override
    {
        return ping_pong_options::instance().ssl_test_timeout;
    }

};


TEST_F(SSLPingPong, CheckedCast)
{
    CheckedCast();
}

TEST_F(SSLPingPong, WireFunctions)
{
   WireFunctions();
}

TEST_F(SSLPingPong, OneWayPing)
{
    OneWayPing();
}

TEST_F(SSLPingPong, SyncRoundtrip)
{
    SyncRoundtrip();
}

TEST_F(SSLPingPong, MTConnectionUsage)
{
    MTConnectionUsage();
}

TEST_F(SSLPingPong, ConnectFailException)
{
    ConnectFailException();
}

TEST_F(SSLPingPong, AsyncConnectFailException)
{
    AsyncConnectFailException();
}

} /* namespace test */
} /* namespace wire */


