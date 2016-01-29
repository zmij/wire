/*
 * tcp_sparring.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include "tcp_sparring.hpp"
#include "sparring_options.hpp"
#include <iostream>
#include <wire/encoding/message.hpp>

namespace wire {
namespace test {
namespace tcp {

session::~session()
{
}

void
session::start()
{
	using test::sparring_options;
	sparring_options& opts = sparring_options::instance();
	if (opts.validate_message) {
		if (limit_requests_)
			++requests_;
		std::vector<char> b;

		encoding::write(std::back_inserter(b),
				encoding::message{ encoding::message::validate, 0 });
		std::copy(b.begin(), b.end(), data_);
		ASIO_NS::async_write(socket_, ASIO_NS::buffer(data_, b.size()),
				std::bind(&session::handle_write, this,
					std::placeholders::_1, std::placeholders::_2));
	} else {
		start_read();
	}
}

void
session::start_read()
{
	socket_.async_read_some(
		ASIO_NS::buffer(data_, max_length),
		std::bind(&session::handle_read, this,
				std::placeholders::_1, std::placeholders::_2));
}

void
session::handle_read(asio_config::error_code const& ec, size_t bytes_transferred)
{
	if (!ec) {
		ASIO_NS::async_write(socket_, ASIO_NS::buffer(data_, bytes_transferred),
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
			start_read();
			return;
		} else if (requests_ > 0){
			--requests_;
			if (requests_ > 0) {
				start_read();
				return;
			}
		}
	}
	socket_.close();
	delete this;
}

server::server(asio_config::io_service& svc)
	: io_service_(svc),
	  acceptor_{svc, asio_config::tcp::endpoint{ asio_config::tcp::v4(), 0 }},
	  connections_(sparring_options::instance().connections), limit_connections_(connections_ > 0),
	  requests_(sparring_options::instance().requests)
{
	asio_config::tcp::endpoint ep = acceptor_.local_endpoint();
	std::cout << ep.port() << std::endl;
	start_accept();
}

void
server::start_accept()
{
	session* new_session = new session(io_service_, requests_);
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

}  // namespace tcp
}  // namespace test
}  // namespace wire
