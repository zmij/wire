/*
 * ssl-client.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: zmij
 */

#include <iostream>
#include <string>
#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/program_options.hpp>

enum { max_length = 1024 };

class client {
public:
	typedef boost::asio::ssl::stream< boost::asio::ip::tcp::socket > transport_type;
	typedef boost::asio::io_service io_service;
	typedef boost::asio::ssl::context ssl_context;
public:
	client(io_service& io, ssl_context& context,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
		: transport_(io, context)
	{
		transport_.set_verify_mode(boost::asio::ssl::verify_peer);
		transport_.set_verify_callback(
			std::bind(&client::verify_certificate, this,
					std::placeholders::_1, std::placeholders::_2) );

		boost::asio::async_connect(transport_.lowest_layer(), endpoint_iterator,
			std::bind(&client::handle_connect, this, std::placeholders::_1));
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
	handle_connect(boost::system::error_code const& ec)
	{
		if (!ec) {
			transport_.async_handshake(boost::asio::ssl::stream_base::client,
				std::bind(&client::handle_handshake, this, std::placeholders::_1)
			);
		} else {
			std::cerr << "Connect failed: " << ec.message() << "\n";
		}
	}

	void
	handle_handshake(boost::system::error_code const& ec)
	{
		if (!ec) {
			std::cout << "Handshake ok\n";
			std::cout << "Enter message: ";
			std::cin.getline(request_, max_length);
			size_t request_length = strlen(request_);

			boost::asio::async_write(transport_,
					boost::asio::buffer(request_, request_length),
					std::bind(&client::handle_write, this,
							std::placeholders::_1,
							std::placeholders::_2));
		} else {
			std::cerr << "Handshake failed: " << ec.message() << "\n";
		}
	}

	void
	handle_write(boost::system::error_code const& ec, size_t bytes_transfered)
	{
		if (!ec) {
			boost::asio::async_read(transport_,
				boost::asio::buffer(reply_, bytes_transfered),
				std::bind(&client::handle_read, this,
					std::placeholders::_1, std::placeholders::_2)
				);
		} else {
			std::cerr << "Write failed: " << ec.message() << "\n";
		}
	}
	void
	handle_read(boost::system::error_code const& ec, size_t bytes_transfered)
	{
		if (!ec) {
			std::cout << "Reply: ";
			std::cout.write(reply_, bytes_transfered);
			std::cout << "\n";
		} else {
			std::cerr << "Read failed: " << ec.message() << "\n";
		}
	}
private:
	transport_type transport_;

	char request_[max_length];
	char reply_[max_length];
};

int
main(int argc, char* argv[])
{
	using boost::asio::ip::tcp;
	try {
		namespace po = boost::program_options;

		std::string host;
		std::string port;
		std::string verify_file;
		std::string cert_file;
		std::string key_file;

		po::options_description options("SSL client options");

		options.add_options()
			("host,h", po::value< std::string >(&host), "Host")
			("port,p", po::value< std::string >(&port), "Port")
			("verify-file,v", po::value< std::string >(&verify_file), "Verify file")
			("cert-file,c", po::value< std::string >(&cert_file), "Certificate file")
			("key-file,k", po::value< std::string >(&key_file), "Private key file")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, options), vm);
		po::notify(vm);

		if (host.empty() || port.empty()) {
			std::cerr << "Usage:\n" << options << "\n";
			return 2;
		}

		if (!cert_file.empty() && key_file.empty()) {
			std::cerr << "Private key required for certificate\n";
			return 2;
		}

		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(host, port);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
		if (!verify_file.empty()) {
			ctx.load_verify_file(verify_file);
		}

		if (!cert_file.empty()) {
			ctx.use_certificate_chain_file(cert_file);
			ctx.use_private_key_file(key_file, boost::asio::ssl::context_base::pem);
		}

		client c(io_service, ctx, iterator);

		io_service.run();
	} catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}

