/*
 * endpoints_grammar_test.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <grammar/grammar_parse_test.hpp>
#include <wire/core/grammar/endpoint_parse.hpp>

namespace wire {
namespace core {
namespace test {

template < typename InputIterator >
using tcp_data_grammar =
        grammar::parse::ip_endpoint_data_grammar<
            InputIterator, detail::tcp_endpoint_data >;

GRAMMAR_TEST(tcp_data_grammar, TCPEndpoint,
        ::testing::Values(
            "127.0.0.1:10",
            "255.255.2.1:80",
            "10.0.254.1:3232",
            "192.0.2.33:7678",
            "google.com:8888",
            "www.mail.ru:765",
            "0.0.0.0:0",
            "[::8]:5678",
            "127.0.0.1:786",
            "[ffaa:1234::aadd:255.255.255.0]:567"
        ),
        ::testing::Values(
            "255.255.2.1:456789",
            "10.0.254.1:-78",
            "192.0.2.33:bla",
            "127.0.0.256",
            "255.255.2.387",
            "400.0.254.1",
            "192.0..33"
        )
);

GRAMMAR_TEST(grammar::parse::socket_endpoint_grammar, SocketEndpoint,
    ::testing::Values(
        "/tmp/.socket", "/blabla/.123123/adfa/socket"
    ),
    ::testing::Values(
        "/", " "
    )
);

GRAMMAR_PARSE_TEST(grammar::parse::endpoint_grammar, Endpoint, endpoint,
    ::testing::Values(
        ParseEndpoint::make_test_data("tcp://localhost:5432",
                endpoint::tcp("localhost", 5432)),
        ParseEndpoint::make_test_data("tcp://0.0.0.0:0",
                endpoint::tcp("0.0.0.0", 0)),
        ParseEndpoint::make_test_data("ssl://aw.my.com:5432",
                endpoint::ssl("aw.my.com", 5432)),
        ParseEndpoint::make_test_data("udp://127.0.0.1:5432",
                endpoint::udp("127.0.0.1", 5432)),
        ParseEndpoint::make_test_data("socket:///tmp/.123.thasocket",
                endpoint::socket("/tmp/.123.thasocket"))
    )
);

template < typename InputIterator >
using endpoint_list_grammar = grammar::parse::endpoints_grammar< InputIterator, endpoint_list >;
GRAMMAR_TEST(endpoint_list_grammar, EndpointList,
    ::testing::Values(
        "socket:///tmp/.socket,tcp://localhost:5432,udp://127.0.0.1:5432",
        "socket:///tmp/.socket,tcp://localhost:5432,udp://127.0.0.1:5432",
        "socket:///blabla/.123123/adfa/socket"
    ),
    ::testing::Values(
        "socket:///tmp/.socket, tcp://localhost:5432, udp://127.0.0.1:5432",
        "/", " "
    )
);

template < typename InputIterator >
using endpoint_set_grammar = grammar::parse::endpoints_grammar< InputIterator, endpoint_set >;
GRAMMAR_TEST(endpoint_set_grammar, EndpointSet,
    ::testing::Values(
        "socket:///tmp/.socket,tcp://localhost:5432,udp://127.0.0.1:5432",
        "socket:///tmp/.socket,tcp://localhost:5432,udp://127.0.0.1:5432",
        "socket:///blabla/.123123/adfa/socket"
    ),
    ::testing::Values(
        "socket:///tmp/.socket, tcp://localhost:5432, udp://127.0.0.1:5432",
        "/", " "
    )
);

}  // namespace test
}  // namespace core
}  // namespace wire
