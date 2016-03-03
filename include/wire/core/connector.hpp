/*
 * connector.hpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_CONNECTOR_HPP_
#define WIRE_CORE_CONNECTOR_HPP_

#include <memory>

#include <wire/asio_config.hpp>

#include <wire/core/endpoint.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>

namespace wire {
namespace core {

class connector {
public:
	typedef ::std::vector<::std::string> args_type;
public:
	static connector_ptr
	create_connector(asio_config::io_service_ptr svc);
	static connector_ptr
	create_connector(asio_config::io_service_ptr svc, int argc, char* argv[]);
	static connector_ptr
	create_connector(asio_config::io_service_ptr svc, args_type const&);
	static connector_ptr
	create_connector(asio_config::io_service_ptr svc, ::std::string const& name);
	static connector_ptr
	create_connector(asio_config::io_service_ptr svc, ::std::string const& name, int argc, char* argv[]);
	static connector_ptr
	create_connector(asio_config::io_service_ptr svc, ::std::string const& name, args_type const&);
private:
	connector(asio_config::io_service_ptr svc);
	connector(asio_config::io_service_ptr svc, int argc, char* argv[]);
	connector(asio_config::io_service_ptr svc, args_type const&);
	connector(asio_config::io_service_ptr svc, ::std::string const& name);
	connector(asio_config::io_service_ptr svc, ::std::string const& name, int argc, char* argv[]);
	connector(asio_config::io_service_ptr svc, ::std::string const& name, args_type const&);

	template< typename ... T >
	static connector_ptr
	do_create_connector(T ... args);
public:
	void
	confugure(int argc, char* argv[]);
	void
	configure(args_type const&);

	asio_config::io_service_ptr
	io_service();

	void
	run();
	void
	stop();

	adapter_ptr
	create_adapter(::std::string const& name);
	adapter_ptr
	create_adapter(::std::string const& name, endpoint_list const& eps);
private:
	struct impl;
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONNECTOR_HPP_ */
