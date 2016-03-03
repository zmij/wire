/*
 * connector.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: zmij
 */

#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/detail/configuration_options.hpp>

#include <boost/program_options.hpp>

#include <sstream>
#include <fstream>

namespace wire {
namespace core {

namespace po = boost::program_options;

struct connector::impl {
	connector_weak_ptr			owner_;
	asio_config::io_service_ptr	io_service_;
	detail::connector_options	options_;

	po::options_description		cmd_line_options_;
	po::options_description		cfg_file_options_;

	args_type					unrecognized_cmd_;
	args_type					unrecognized_cfg_;

	impl(asio_config::io_service_ptr svc)
		: io_service_{svc}, options_{},
		  cmd_line_options_{ options_.name + " command line options" },
		  cfg_file_options_{ options_.name + " config file options" }
	{
		create_options_description();
	}

	impl(asio_config::io_service_ptr svc, ::std::string const& name)
		: io_service_{svc}, options_{name},
		  cmd_line_options_{ options_.name + " command line options" },
		  cfg_file_options_{ options_.name + " config file options" }
	{
		create_options_description();
	}

	void
	create_options_description()
	{
		std::string const& name = options_.name;

		po::options_description cfg_opts("Configuration file");
		cfg_opts.add_options()
		((name + ".config_file").c_str(),
					po::value<::std::string>(&options_.config_file),
					"Configuration file")
		;
		po::options_description server_ssl_opts("Server SSL Options");
		server_ssl_opts.add_options()
		((name + ".ssl.server.certificate").c_str(),
				po::value<::std::string>(&options_.server_ssl.cert_file),
				"SSL certificate")
		((name + ".ssl.server.key").c_str(),
				po::value<::std::string>(&options_.server_ssl.key_file),
				"SSL key file")
		((name + ".ssl.server.verify_file").c_str(),
				po::value<::std::string>(&options_.server_ssl.verify_file),
				"SSL verify file (CA root). If missing, default system certificates are used")
		((name + ".ssl.server.require_peer_cert").c_str(),
				po::bool_switch(&options_.server_ssl.require_peer_cert),
				"Require client SSL certificate")
		;
		po::options_description client_ssl_opts("Client SSL Options");
		client_ssl_opts.add_options()
		((name + ".ssl.client.certificate").c_str(),
				po::value<::std::string>(&options_.client_ssl.cert_file),
				"SSL certificate")
		((name + ".ssl.client.key").c_str(),
				po::value<::std::string>(&options_.client_ssl.key_file),
				"SSL key file")
		((name + ".ssl.client.verify_file").c_str(),
				po::value<::std::string>(&options_.client_ssl.verify_file),
				"SSL verify file (CA root). If missing, default system certificates are used")
		;

		cmd_line_options_.add(cfg_opts)
				.add(server_ssl_opts)
				.add(client_ssl_opts);
		cfg_file_options_.add(server_ssl_opts)
				.add(client_ssl_opts);
	}

	template< typename ... T >
	void
	parse_configuration( T ... args )
	{
		po::parsed_options parsed_cmd =
				po::command_line_parser(args ... ).options(cmd_line_options_).
							allow_unregistered().run();

		po::variables_map vm;
		po::store(parsed_cmd, vm);
		unrecognized_cmd_ = po::collect_unrecognized(parsed_cmd.options, po::exclude_positional);

		if (!options_.config_file.empty()) {
			std::ifstream cfg(options_.config_file.c_str());
			if (cfg) {
				po::parsed_options parsed_cfg = po::parse_config_file(cfg, cfg_file_options_, true);
				po::store(parsed_cfg, vm);
				unrecognized_cfg_ = po::collect_unrecognized(parsed_cfg.options, po::exclude_positional);
			} else {
				throw ::std::runtime_error(
					"Failed to open configuration file " + options_.config_file );
			}
		}
		po::notify(vm);
	}

	void
	configure(int argc, char* argv[])
	{
		parse_configuration(argc, argv);
	}

	void
	configure(args_type const& args)
	{
		parse_configuration(args);
	}

	detail::adapter_options
	configure_adapter(::std::string const& name, endpoint_list const& eps = endpoint_list{})
	{
		detail::adapter_options aopts;

		po::options_description adapter_opts(name + " Adapter Options");

		po::options_description adapter_general_opts(name + " Adapter General Options");

		auto ep_value = po::value<endpoint_list>(&aopts.endpoints);
		if (eps.empty()) {
			ep_value = ep_value->required();
		} else {
			aopts.endpoints = eps;
		}

		adapter_general_opts.add_options()
		((name + ".endpoints").c_str(), ep_value, "Adapter endpoints")
		((name + ".timeout").c_str(),
			po::value<::std::size_t>(&aopts.timeout)->default_value(0), "Request timeout")
		;

		po::options_description adapter_ssl_opts(name + " Adapter SSL Options");
		adapter_ssl_opts.add_options()
		((name + ".ssl.certificate").c_str(),
				po::value<::std::string>(&aopts.server_ssl.cert_file),
				"SSL certificate")
		((name + ".ssl.key").c_str(),
				po::value<::std::string>(&aopts.server_ssl.key_file),
				"SSL key file")
		((name + ".ssl.verify_file").c_str(),
				po::value<::std::string>(&aopts.server_ssl.verify_file),
				"SSL verify file (CA root). If missing, default system certificates are used")
		((name + ".ssl.require_peer_cert").c_str(),
				po::bool_switch(&aopts.server_ssl.require_peer_cert),
				"Require client SSL certificate")
		;

		adapter_opts.add(adapter_general_opts).add(adapter_ssl_opts);

		po::variables_map vm;
		po::parsed_options parsed_cmd =
				po::command_line_parser(unrecognized_cmd_).options(adapter_opts).
					allow_unregistered().run();
		po::store(parsed_cmd, vm);
		unrecognized_cmd_ = po::collect_unrecognized(parsed_cmd.options, po::exclude_positional);

		po::parsed_options parsed_cfg =
				po::command_line_parser(unrecognized_cfg_).options(adapter_opts).
					allow_unregistered().run();
		po::store(parsed_cfg, vm);
		unrecognized_cfg_ = po::collect_unrecognized(parsed_cfg.options, po::exclude_positional);

		po::notify(vm);

		return aopts;
	}

	adapter_ptr
	create_adapter(::std::string const& name, endpoint_list const& eps = endpoint_list{})
	{
		connector_ptr conn = owner_.lock();
		if (!conn) {
			throw std::runtime_error("Connector already destroyed");
		}
		detail::adapter_options opts = configure_adapter(name, eps);
		return adapter::create_adapter(conn, name, opts);
	}
};

template < typename ... T >
connector_ptr
connector::do_create_connector(T ... args)
{
	connector_ptr c(new connector(args ...));
	c->pimpl_->owner_ = c;
	return c;
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc)
{
	return do_create_connector(svc);
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, int argc, char* argv[])
{
	return do_create_connector(svc, argc, argv);
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, args_type const& args)
{
	return do_create_connector(svc, args);
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, ::std::string const& name)
{
	return do_create_connector(svc, name);
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, ::std::string const& name,
		int argc, char* argv[])
{
	return do_create_connector(svc, name, argc, argv);
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, ::std::string const& name,
		args_type const& args)
{
	return do_create_connector(svc, name, args);
}
//

connector::connector(asio_config::io_service_ptr svc)
	: pimpl_(::std::make_shared<impl>(svc))
{
}

connector::connector(asio_config::io_service_ptr svc, int argc, char* argv[])
	: pimpl_(::std::make_shared<impl>(svc))
{
	pimpl_->configure(argc, argv);
}

connector::connector(asio_config::io_service_ptr svc, args_type const& args)
	: pimpl_(::std::make_shared<impl>(svc))
{
	pimpl_->configure(args);
}

connector::connector(asio_config::io_service_ptr svc, ::std::string const& name)
	: pimpl_(::std::make_shared<impl>(svc, name))
{
}

connector::connector(asio_config::io_service_ptr svc, ::std::string const& name,
		int argc, char* argv[])
	: pimpl_(::std::make_shared<impl>(svc, name))
{
	pimpl_->configure(argc, argv);
}

connector::connector(asio_config::io_service_ptr svc, ::std::string const& name,
		args_type const& args)
	: pimpl_(::std::make_shared<impl>(svc, name))
{
	pimpl_->configure(args);
}

void
connector::confugure(int argc, char* argv[])
{
	pimpl_->configure(argc, argv);
}

void
connector::configure(args_type const& args)
{
	pimpl_->configure(args);
}

asio_config::io_service_ptr
connector::io_service()
{
	return pimpl_->io_service_;
}

adapter_ptr
connector::create_adapter(::std::string const& name)
{
	return pimpl_->create_adapter(name);
}

adapter_ptr
connector::create_adapter(::std::string const& name, endpoint_list const& eps)
{
	return pimpl_->create_adapter(name, eps);
}


}  // namespace core
}  // namespace wire
