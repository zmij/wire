/*
 * sparring_partner.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <iostream>
#include <boost/program_options.hpp>
#include <wire/core/endpoint.hpp>

#include "tcp_sparring.hpp"

int
main(int argc, char* argv[])
{
	namespace po = boost::program_options;

	try {
		wire::core::transport_type transport;
		std::size_t connection_count(0);
		std::size_t request_count(1);

		po::options_description options("Sparring server options");
		options.add_options()
			("transport,t",
				po::value<wire::core::transport_type>(&transport)->required(),
				"Start server for transport [tcp, ssl, udp, socket]")
			("connections,c",
				po::value<std::size_t>(&connection_count)->default_value(0),
				"Number of connections to accept")
			("requests,r",
				po::value<std::size_t>(&request_count)->default_value(1),
				"Number of reqests per connection to handle")
		;

		po::options_description cmd_line;
		cmd_line.add(options);

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, cmd_line), vm);
		po::notify(vm);
		

		wire::asio_config::io_service io_service;

		switch (transport) {
			case wire::core::transport_type::tcp: {
				wire::test::tcp::server s(io_service, connection_count, request_count);
				io_service.run();
				break;
			}
			default:
				break;
		}

	} catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
