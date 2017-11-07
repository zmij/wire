/*
 * ping_pong_options.hpp
 *
 *  Created on: Nov 7, 2017
 *      Author: zmij
 */

#ifndef WIRE_TEST_CONNECTOR_PING_PONG_OPTIONS_HPP_
#define WIRE_TEST_CONNECTOR_PING_PONG_OPTIONS_HPP_

#include "sparring/options.hpp"

namespace wire {
namespace test {

class ping_pong_options : public sparring::options {
public:
    static ping_pong_options&
    instance();
public:
    ::std::size_t   tcp_req_per_thread  = 10;
    ::std::int64_t  tcp_test_timeout    = 5;
    ::std::size_t   ssl_req_per_thread  = 10;
    ::std::int64_t  ssl_test_timeout    = 5;
private:
    ping_pong_options() {}
};

} /* namespace test */
} /* namespace wire */


#endif /* WIRE_TEST_CONNECTOR_PING_PONG_OPTIONS_HPP_ */
