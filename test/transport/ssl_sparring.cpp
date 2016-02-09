/*
 * tcp_sparring.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include "ssl_sparring.hpp"
#include "sparring_options.hpp"
#include <iostream>
#include <wire/encoding/message.hpp>

namespace wire {
namespace test {
namespace ssl {

session::session(asio_config::io_service& svc, ssl_context& ctx)
	: socket_(svc, ctx),
	  requests_(sparring_options::instance().requests), limit_requests_(requests_ > 0)
{
	sparring_options& opts = sparring_options::instance();
	if (opts.require_peer_cert) {
		socket_.set_verify_mode(ASIO_NS::ssl::verify_peer |
				ASIO_NS::ssl::verify_fail_if_no_peer_cert);
	}
	socket_.set_verify_callback(
			std::bind(&session::verify_certificate, this,
					std::placeholders::_1, std::placeholders::_2) );
}

void
session::start()
{
	socket_.async_handshake(
		ASIO_NS::ssl::stream_base::server,
		std::bind(&session::handle_handshake, this,
				std::placeholders::_1));
}

bool
session::verify_certificate(bool preverified, ASIO_NS::ssl::verify_context& ctx)
{
	char subject_name[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	std::cerr << "Verifying " << subject_name << "\n";

	return preverified;
}

void
session::handle_handshake(asio_config::error_code const& ec)
{
	if (!ec) {
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
	} else {
		std::cerr << "Handshake failed: " << ec.message() << "\n";
		delete this;
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
	socket_.lowest_layer().close();
	delete this;
}

server::server(asio_config::io_service_ptr svc)
	: io_service_(svc),
	  acceptor_{*svc, asio_config::tcp::endpoint{ asio_config::tcp::v4(), sparring_options::instance().port }},
	  context_{ ASIO_NS::ssl::context::sslv23 },
	  connections_(sparring_options::instance().connections), limit_connections_(connections_ > 0),
	  requests_(sparring_options::instance().requests)
{
	sparring_options& opts = sparring_options::instance();
	context_.set_options(
		asio_config::ssl_context::default_workarounds |
		asio_config::ssl_context::no_sslv2 |
		asio_config::ssl_context::single_dh_use
	);
	if (!opts.verify_file.empty()) {
		context_.load_verify_file(opts.verify_file);
	}
	context_.use_certificate_chain_file(opts.cert_file);
	context_.use_private_key_file(opts.key_file, ASIO_NS::ssl::context::pem);
	asio_config::tcp::endpoint ep = acceptor_.local_endpoint();
	std::cout << ep.port() << std::endl;
	start_accept();
}

void
server::start_accept()
{
	session* new_session = new session(*io_service_, context_);
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

}  // namespace ssl
}  // namespace test
}  // namespace wire
