/*
 * tcp_sparring.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include "socket_sparring.hpp"
#include "sparring_options.hpp"
#include <iostream>

namespace wire {
namespace test {
namespace socket {

void
session::start()
{
    socket_.async_read_some(
        asio_ns::buffer(data_, max_length),
        std::bind(&session::handle_read, this,
                std::placeholders::_1, std::placeholders::_2));
}

void
session::handle_read(asio_config::error_code const& ec, size_t bytes_transferred)
{
    if (!ec) {
        asio_ns::async_write(socket_, asio_ns::buffer(data_, bytes_transferred),
                std::bind(&session::handle_write, this,
                    std::placeholders::_1, std::placeholders::_2));
    } else {
        delete this;
    }
}

void
session::handle_write(asio_config::error_code const& ec, size_t bytes_transferred)
{
    if (!ec) {
        if (!limit_requests_) {
            start();
            return;
        } else if (requests_ > 0){
            --requests_;
            if (requests_ > 0) {
                start();
                return;
            }
        }
    }
    socket_.close();
    delete this;
}

server::server(asio_config::io_service_ptr svc)
    : io_service_(svc),
      acceptor_{*svc,
            endpoint_type{ "/tmp/." + std::to_string(::getpid()) + "_test_sparring" }},
      signals_{*svc},
      connections_(sparring_options::instance().connections), limit_connections_(connections_ > 0),
      requests_(sparring_options::instance().requests)
{
    endpoint_type ep = acceptor_.local_endpoint();
    std::cout << ep.path() << std::endl;

    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(std::bind(&server::handle_stop, this));

    start_accept();
}

server::~server()
{
}

void
server::start_accept()
{
    session* new_session = new session(*io_service_, requests_);
    acceptor_.async_accept(new_session->socket(),
            std::bind(&server::handle_accept, this, new_session, std::placeholders::_1));
}

void
server::handle_accept(session* new_session, asio_config::error_code const& ec)
{
    if (!ec) {
        new_session->start();
    } else {
        delete new_session;
    }
    if (!limit_connections_) {
        start_accept();
    } else {
        --connections_;
        if (connections_ > 0) {
            start_accept();
        }
    }
}

void
server::handle_stop()
{
    if (!io_service_->stopped()) {
        acceptor_.cancel();
        io_service_->stop();
        ::unlink( acceptor_.local_endpoint().path().c_str() );
    }
}

}  // namespace socket
}  // namespace test
}  // namespace wire
