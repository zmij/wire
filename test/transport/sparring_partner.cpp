/*
 * sparring_partner.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <iostream>
#include <boost/program_options.hpp>
#include <wire/core/endpoint.hpp>

#include "sparring_options.hpp"
#include "tcp_sparring.hpp"
#include "tcp_sparring_client.hpp"
#include "ssl_sparring.hpp"
#include "ssl_sparring_client.hpp"
#include "udp_sparring.hpp"
#include "socket_sparring.hpp"

int
main(int argc, char* argv[])
{
	namespace po = boost::program_options;
	using wire::test::sparring_options;

	try {
		sparring_options& opts = sparring_options::instance();

		po::options_description options("Sparring server options");
		options.add_options()
			("help,h", "Show help message")
			("transport,t",
				po::value<wire::core::transport_type>(&opts.transport)->required(),
				"Start server for transport [tcp, ssl, udp, socket]")
			("port,p",
				po::value<uint16_t>(&opts.port), "Fixed port value")
			("connections,c",
				po::value<std::size_t>(&opts.connections)->default_value(0),
				"Number of connections to accept")
			("requests,r",
				po::value<std::size_t>(&opts.requests)->default_value(1),
				"Number of reqests per connection to handle")
			("greet-message",
				po::value<std::string>(&opts.greet_message),
				"Send message after accepting connection (incompatible with validate-message)")
			("validate-message,v",
				po::bool_switch(&opts.validate_message),
				"Send validate message after accepting connection (incompatible with greet-message)")
			("ping-pong",
				po::bool_switch(&opts.ping_pong),
				"Read request message and send encapsulated data back")
			("client",
				po::bool_switch(&opts.client), "")
		;

		po::options_description ssl_opts("SSL sparring options");
		ssl_opts.add_options()
			("cert-file",
				po::value< std::string >(&opts.cert_file), "Certificate file")
			("key-file,k",
				po::value< std::string >(&opts.key_file), "Private key file")
			("verify-file",
				po::value< std::string >(&opts.verify_file), "Verify file")
			("verify-client",
				po::bool_switch(&opts.require_peer_cert), "Verify client")
		;

		po::options_description cmd_line;
		cmd_line.add(options).add(ssl_opts);

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, cmd_line), vm);
		if (vm.count("help")) {
			std::cerr << "Usage:\n" << argv[0] << " <options>\n"
					<< cmd_line << "\n";
			return 0;
		}
		po::notify(vm);

		wire::asio_config::io_service_ptr io_service{ std::make_shared<wire::asio_config::io_service>() };

		switch (opts.transport) {
			case wire::core::transport_type::tcp: {
				if (opts.client) {
					wire::test::tcp::client c(io_service);
					io_service->run();
				} else {
					wire::test::tcp::server s(io_service);
					io_service->run();
				}
				break;
			}
			case wire::core::transport_type::ssl: {
				if (opts.client) {
					wire::test::ssl::client c(io_service);
					io_service->run();
				} else {
					if (opts.cert_file.empty() || opts.key_file.empty()) {
						std::cerr << "No certificate or private key file\n"
								<< cmd_line << "\n";
						return 2;
					}
					wire::test::ssl::server s(io_service);
					io_service->run();
				}
				break;
			}
			case wire::core::transport_type::udp: {
				wire::test::udp::server s(io_service);
				io_service->run();
				break;
			}
			case wire::core::transport_type::socket: {
				wire::test::socket::server s(io_service);
				io_service->run();
				break;
			}
			default:
				std::cerr << "Usage\n" << cmd_line << "\n";
				break;
		}

	} catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
