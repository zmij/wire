/*
 * client_connection_test.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/connection.hpp>
#include <wire/core/transport.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/proxy.hpp>

#include "sparring/sparring_test.hpp"
#include "config.hpp"

#include <sstream>
#include <bitset>

namespace wire {
namespace core {
namespace test {

namespace {

::std::string const LIPSUM_TEST_STRING = R"~(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque malesuada
ut nulla vitae elementum. Curabitur dictum egestas mauris et accumsan. Aliquam
erat volutpat. Proin tempor enim vitae purus hendrerit, id varius tellus
malesuada. Phasellus mattis molestie est non auctor. Etiam porttitor est at
commodo mollis. Sed ut imperdiet velit. Vivamus eget sapien in lorem consequat
varius nec vitae eros. Suspendisse interdum arcu dui, eu placerat libero rutrum
quis. Nullam molestie mattis rhoncus. Duis imperdiet, massa sit amet varius
malesuada, turpis quam fringilla orci, in consequat lectus nisi non massa.
Mauris nec purus aliquam massa pellentesque finibus quis sed nisi. Nunc ac
sapien nulla. Duis volutpat dui vitae vestibulum molestie. Suspendisse sagittis
quis ex vitae pulvinar. Phasellus hendrerit in erat non convallis.

Nulla facilisi. Nunc et enim sed tortor mollis varius. Cras quis bibendum
sapien. Maecenas vitae lectus vel lorem gravida blandit. Vestibulum faucibus
sed sapien at scelerisque. Curabitur sit amet purus varius, volutpat orci id,
auctor libero. Fusce vulputate lectus sapien, eu ornare sem maximus quis. Sed
pellentesque tempus porta. Nam consequat tincidunt molestie. Quisque mollis ut
quam quis ultricies. Donec consectetur leo odio, a faucibus justo interdum et.
Vivamus fermentum justo eu sapien lobortis, non luctus nisi sodales. Donec
ullamcorper felis et justo consequat, a gravida magna interdum. Morbi at dictum
nisl. Nam condimentum bibendum turpis, eget aliquam velit vulputate vitae.
Aliquam non nibh et mi blandit varius ut nec quam.

In mattis, diam eget lobortis auctor, odio tellus tristique lectus, et sodales
odio est ut ligula. Maecenas elit odio, fermentum et leo euismod, blandit
vestibulum magna. Morbi non elementum quam. Nunc varius erat sed eros porttitor
auctor. Praesent luctus orci rutrum, fermentum nisl sit amet, varius nisi.
Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia
Curae; Mauris id turpis eget ligula consequat tincidunt. Duis sed sem ante. Nunc
fermentum risus nec orci dapibus accumsan. Maecenas vitae nunc lorem. Aenean
maximus elementum maximus. Phasellus finibus mi dolor, in molestie sem ultrices
ac.
)~";

}  // namespace

class Client : public wire::test::sparring::SparringTest {

protected:
    void
    SetupArgs(args_type& args) override
    {
        std::ostringstream os;
        os << current_transport;
        args.insert(args.end(), {
            "--transport", os.str(),
        });

        if (!add_args.empty()) {
            args.insert(args.end(), add_args.begin(), add_args.end());
        }
    }
    void
    ReadSparringOutput(std::istream& is) override
    {
        switch (current_transport) {
            case transport_type::tcp:
                endpoint_ = endpoint{ ReadEnpointPort< detail::tcp_endpoint_data >(is) };
                break;
            case transport_type::ssl:
                endpoint_ = endpoint{ ReadEnpointPort< detail::ssl_endpoint_data >(is) };
                break;
            default:
                break;
        }
    }

    endpoint        endpoint_;
    transport_type     current_transport;
    args_type        add_args;
};

TEST_F(Client, TCPConnect)
{
    typedef transport_type_traits< transport_type::tcp > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--validate-message",
        "-r2"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector_ptr cnctr = connector::create_connector(io_svc);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_TRUE(connected);
    EXPECT_FALSE(error);
}

TEST_F(Client, TCPConnectFail)
{
    typedef transport_type_traits< transport_type::tcp > used_transport;
    current_transport = used_transport::value;
    add_args.push_back("--validate-message");
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);
    StopPartner();

    connector_ptr cnctr = connector::create_connector(io_svc);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_FALSE(connected);
    EXPECT_TRUE(error);
}

TEST_F(Client, TCPConnectInvalidValidate)
{
    typedef transport_type_traits< transport_type::tcp > used_transport;
    current_transport = used_transport::value;
    // TODO Test with small or no greeting
    add_args.insert(add_args.end(), {"--greet-message", "hello"});
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector_ptr cnctr = connector::create_connector(io_svc);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_FALSE(connected);
    EXPECT_TRUE(error);
}

TEST_F(Client, DISABLED_TCPSendRequest)
{
    typedef transport_type_traits< transport_type::tcp > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--validate-message",
        "--ping-pong",
        "-r2"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector_ptr cnctr = connector::create_connector(io_svc);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    std::bitset< 5 > tests;

    std::string test_str;
    bool test_flag = false;
    uint32_t test_int = 0;

    invocation_options opts{};

    c.connect_async(endpoint_,
    [&](){
        tests[0] = true;
        c.invoke({identity::random("test"), ""}, std::string("ping_pong"), context_type{},
        opts,
        [&](std::string const& str, bool flag, uint32_t i) {
            std::cerr << "Response received\n";

            tests[1] = true;
            test_str = str;
            test_flag = flag;
            test_int = i;

            c.close();
            io_svc->stop();
        },
        [&](std::exception_ptr){
            std::cerr << "Exception received\n";
            tests[2] = true;
            c.close();
            io_svc->stop();
        },
        [&](bool ){
            std::cerr << "Request sent\n";
            tests[3] = true;
        },
        LIPSUM_TEST_STRING, true, uint32_t(42));
    },
    [&](std::exception_ptr) {
        tests[4] = true;
        c.close();
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_TRUE(tests[0]);    // Connected
    EXPECT_TRUE(tests[1]);    // Response received
    EXPECT_FALSE(tests[2]);    // No wire exceptions
    EXPECT_TRUE(tests[3]);    // Request sent
    EXPECT_FALSE(tests[4]);    // No connection exceptions

    EXPECT_EQ(LIPSUM_TEST_STRING, test_str);
    EXPECT_TRUE(test_flag);
    EXPECT_EQ(42, test_int);
}

TEST_F(Client, DISABLED_ProxyPingAsyncTest)
{
    typedef transport_type_traits< transport_type::tcp > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--validate-message",
        "--ping-pong",
        "-r2"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector_ptr conn = connector::create_connector(io_svc);
    reference_data ref { identity::random("test"), {}, {}, { endpoint_ } };
    ::std::ostringstream os;
    os << ref;
    ::std::cerr << "Reference is " << os.str() << "\n";

    std::bitset< 3 > tests;

    auto obj = conn->string_to_proxy(os.str());
    ASSERT_TRUE(obj.get());
    EXPECT_EQ(ref.object_id, obj->wire_identity());
    obj->wire_ping_async(
    [&]()
    {
        ::std::cerr << "Ping response received\n";
        tests[0] = true;
        io_svc->stop();
    },
    [&](::std::exception_ptr ex)
    {
        ::std::cerr << "Ping exceptions\n";
        tests[1] = true;
        io_svc->stop();
    },
    [&](bool sent)
    {
        tests[2] = sent;
        if (sent) {
            ::std::cerr << "Request sent\n";
        } else {
            ::std::cerr << "Request not sent\n";
        }
    });

    io_svc->run();

    EXPECT_TRUE(tests[0]);
    EXPECT_FALSE(tests[1]);
    EXPECT_TRUE(tests[2]);
}

TEST_F(Client, DISABLED_ProxyPingSyncTest)
{
    typedef transport_type_traits< transport_type::tcp > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--validate-message",
        "--ping-pong",
        "-r2"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector_ptr conn = connector::create_connector(io_svc);
    reference_data ref { identity::random("test"), {}, {}, { endpoint_ } };
    ::std::ostringstream os;
    os << ref;
    ::std::cerr << "Reference is " << os.str() << "\n";

    std::bitset< 2 > tests;

    auto obj = conn->string_to_proxy(os.str());
    ASSERT_TRUE(obj.get());
    EXPECT_EQ(ref.object_id, obj->wire_identity());
    try {
        obj->wire_ping();
        tests[0] = true;
    } catch (::std::exception const& e) {
        tests[1] = true;
    }

    EXPECT_TRUE(tests[0]);
    EXPECT_FALSE(tests[1]);
}

TEST_F(Client, SSL)
{
    typedef transport_type_traits< transport_type::ssl > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--cert-file", wire::test::SERVER_CERT,
        "--key-file", wire::test::SERVER_KEY,
        "--validate-message"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector::args_type config {
        "--wire.connector.ssl.client.verify_file",
        wire::test::CA_ROOT
    };


    connector_ptr cnctr = connector::create_connector(io_svc, config);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_TRUE(connected);
    EXPECT_FALSE(error);
}

TEST_F(Client, SSLInvalidServerCert)
{
    typedef transport_type_traits< transport_type::ssl > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--cert-file", wire::test::SERVER_CERT,
        "--key-file", wire::test::SERVER_KEY,
        "--validate-message"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector_ptr cnctr = connector::create_connector(io_svc);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_FALSE(connected);
    EXPECT_TRUE(error);
}

TEST_F(Client, SSLValidClientCert)
{
    typedef transport_type_traits< transport_type::ssl > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--cert-file", wire::test::SERVER_CERT,
        "--key-file", wire::test::SERVER_KEY,
        "--verify-file", wire::test::CA_ROOT,
        "--verify-client",
        "--validate-message"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector::args_type config {
        "--wire.connector.ssl.client.verify_file",
        wire::test::CA_ROOT,
        "--wire.connector.ssl.client.certificate",
        wire::test::CLIENT_CERT,
        "--wire.connector.ssl.client.key",
        wire::test::CLIENT_KEY
    };


    connector_ptr cnctr = connector::create_connector(io_svc, config);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_TRUE(connected);
    EXPECT_FALSE(error);
}

TEST_F(Client, SSLInvalidClientCert)
{
    typedef transport_type_traits< transport_type::ssl > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--cert-file", wire::test::SERVER_CERT,
        "--key-file", wire::test::SERVER_KEY,
        "--verify-file", wire::test::CA_ROOT,
        "--verify-client",
        "--validate-message"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector::args_type config {
        "--wire.connector.ssl.client.verify_file",
        wire::test::OTHER_CA,
        "--wire.connector.ssl.client.certificate",
        wire::test::OTHER_CERT,
        "--wire.connector.ssl.client.key",
        wire::test::OTHER_KEY
    };


    connector_ptr cnctr = connector::create_connector(io_svc, config);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_FALSE(connected);
    EXPECT_TRUE(error);
}

TEST_F(Client, SSLNoClientCert)
{
    typedef transport_type_traits< transport_type::ssl > used_transport;
    current_transport = used_transport::value;
    add_args.insert(add_args.end(), {
        "--cert-file", wire::test::SERVER_CERT,
        "--key-file", wire::test::SERVER_KEY,
        "--verify-file", wire::test::CA_ROOT,
        "--verify-client",
        "--validate-message"
    });
    StartPartner();

    ASSERT_NE(0, child_.pid);
    ASSERT_EQ(used_transport::value, endpoint_.transport());
    ASSERT_NE(0, endpoint_.get< used_transport::endpoint_data >().port);

    connector::args_type config {
        "--wire.connector.ssl.client.verify_file",
        wire::test::CA_ROOT
    };


    connector_ptr cnctr = connector::create_connector(io_svc, config);
    adapter_ptr bidir = cnctr->bidir_adapter();
    connection c{client_side{}, bidir, endpoint_.transport(), [](connection const*){}};

    bool connected = false;
    bool error = false;

    c.connect_async(endpoint_,
    [&](){
        connected = true;
        c.close();
        io_svc->stop();
    },
    [&](std::exception_ptr) {
        error = true;
        io_svc->stop();
    });

    io_svc->run();

    EXPECT_FALSE(connected);
    EXPECT_TRUE(error);
}

}  // namespace test
}  // namespace core
}  // namespace wire
