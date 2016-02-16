/*
 * ssl-server.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: zmij
 */

#include <iostream>
#include <string>
#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/address.hpp>

#include <boost/program_options.hpp>

namespace boost {
namespace asio {
namespace ip {
std::istream&
operator >> (std::istream& is, address& val)
{
	std::istream::sentry s(is);
	if (s) {
		std::string str;
		if (is >> str) {
			try {
				val = address::from_string(str);
			} catch (boost::system::error_code const& ec) {
				is.setstate(std::ios_base::badbit);
			}
		}
	}
	return is;
}
}  // namespace ip
}  // namespace asio
}  // namespace boost

#ifdef SO_REUSEPORT
namespace test {
typedef boost::asio::detail::socket_option::boolean<
		BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT > reuse_port;
}  // namespace test
#endif

typedef boost::asio::ssl::stream< boost::asio::ip::tcp::socket > ssl_socket_type;
enum { max_length = 1024 };

class session {
public:
	typedef boost::asio::io_service io_service;
public:
	session(io_service& svc, boost::asio::ssl::context& ctx)
		: socket_(svc, ctx)
	{
		socket_.set_verify_mode(boost::asio::ssl::verify_peer |
				boost::asio::ssl::verify_fail_if_no_peer_cert);
		socket_.set_verify_callback(
				std::bind(&session::verify_certificate, this,
						std::placeholders::_1, std::placeholders::_2) );
	}

	ssl_socket_type::lowest_layer_type&
	socket()
	{
		return socket_.lowest_layer();
	}
	void
	start()
	{
		socket_.async_handshake(boost::asio::ssl::stream_base::server,
			std::bind(&session::handle_handshake, this,
					std::placeholders::_1));
	}
private:
	bool
	verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
	{
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		std::cout << "Verifying " << subject_name << "\n";

		return preverified;
	}

	void
	handle_handshake(boost::system::error_code const& ec)
	{
		if (!ec) {
			socket_.async_read_some(boost::asio::buffer(data_, max_length),
				std::bind(&session::handle_read, this,
					std::placeholders::_1, std::placeholders::_2));
		} else {
			std::cerr << "Handshake failed: " << ec.message() << "\n";
			delete this;
		}
	}

	void
	handle_read(boost::system::error_code const& ec, size_t bytes_transferred)
	{
		if (!ec) {
			boost::asio::async_write( socket_,
				boost::asio::buffer(data_, bytes_transferred),
				std::bind(&session::handle_write, this,
					std::placeholders::_1, std::placeholders::_2));
		} else {
			std::cerr << "Read failed: " << ec.message() << "\n";
			delete this;
		}
	}
	void
	handle_write(boost::system::error_code const& ec, size_t bytes_transferred)
	{
		if (!ec) {
			socket_.async_read_some(boost::asio::buffer(data_, max_length),
				std::bind(&session::handle_read, this,
					std::placeholders::_1, std::placeholders::_2));
		} else {
			std::cerr << "Write failed: " << ec.message() << "\n";
			delete this;
		}
	}
private:
	ssl_socket_type socket_;
	char data_[ max_length ];
};

class server {
public:
	typedef boost::asio::io_service io_service;
	typedef boost::asio::ip::tcp::acceptor acceptor;
public:
	server(io_service& svc, boost::asio::ip::tcp::endpoint const& endpoint,
			std::string const& verify_file,
			std::string const& cert_file, std::string const& key_file)
		: io_service_(svc),
		  acceptor_(svc),
		  context_(boost::asio::ssl::context::sslv23)
	{
		context_.set_options(
			boost::asio::ssl::context::default_workarounds |
			boost::asio::ssl::context::no_sslv2 |
			boost::asio::ssl::context::single_dh_use
		);
		if (!verify_file.empty()) {
			context_.load_verify_file(verify_file);
		}
		context_.use_certificate_chain_file(cert_file);
		context_.use_private_key_file(key_file, boost::asio::ssl::context::pem);

		acceptor_.open(endpoint.protocol());
#ifdef SO_REUSEPORT
		acceptor_.set_option(test::reuse_port(true));
		std::cerr << "Reuse port: 1\n";
#endif
		acceptor_.bind(endpoint);
		acceptor_.listen();

		start_accept();
	}
private:
	void
	start_accept()
	{
		session* new_session = new session(io_service_, context_);
		acceptor_.async_accept(new_session->socket(),
			std::bind(&server::handle_accept, this,
					new_session, std::placeholders::_1));
	}

	void
	handle_accept(session* new_session, boost::system::error_code const& ec)
	{
		if (!ec) {
			new_session->start();
		} else {
			std::cerr << "Failed to accept: " << ec.message() << "\n";
			delete new_session;
		}
		start_accept();
	}
private:
	io_service& io_service_;
	acceptor acceptor_;
	boost::asio::ssl::context context_;
};

int
main(int argc, char* argv[])
{
	try {
		namespace po = boost::program_options;

		boost::asio::ip::address bind_addr;
		unsigned short bind_port = 0;
		std::string verify_file;
		std::string cert_file;
		std::string key_file;

		po::options_description options("SSL server options");
		options.add_options()
			("help,h", "Produce help message")
			("bind-address,a",
					po::value< boost::asio::ip::address >(&bind_addr)
					->default_value(boost::asio::ip::address_v4{}),
					"Bind to interface")
			("bind-port,p", po::value< unsigned short >(&bind_port)->default_value(0),
					"Bind to port")
			("verify-file,v", po::value< std::string >(&verify_file), "Verify file")
			("cert-file,c", po::value< std::string >(&cert_file)->required(), "Certificate file")
			("key-file,k", po::value< std::string >(&key_file)->required(), "Private key file")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, options), vm);
		if (vm.count("help")) {
			std::cerr << "Usage:\n" << options << "\n";
			return 0;
		}
		po::notify(vm);

		if (cert_file.empty() || key_file.empty()) {
			std::cerr << "No certificate or private key file\n"
					<< options << "\n";
			return 2;
		}

		std::cerr << "Binding to " << bind_addr.to_string()
				<< ":" << bind_port << "\n";

		boost::asio::ip::tcp::endpoint endpoint(bind_addr, bind_port);
		boost::asio::io_service io_service;
		server s(io_service, endpoint, verify_file, cert_file, key_file);

		io_service.run();
	} catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
