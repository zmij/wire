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
#include <wire/core/connection_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/identity_fwd.hpp>
#include <wire/core/reference_fwd.hpp>

namespace wire {
namespace core {

class connector : public ::std::enable_shared_from_this<connector> {
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
    create_adapter(identity const& id);
    adapter_ptr
    create_adapter(identity const& id, endpoint_list const& eps);

    object_prx
    string_to_proxy(::std::string const& str);

    bool
    is_local(reference const& ref) const;
    /**
     * Get an existing connection to the endpoint specified or create a new one.
     * @param
     * @return
     */
    connection_ptr
    get_outgoing_connection(endpoint const&);
private:
    struct impl;
    typedef std::shared_ptr<impl> pimpl;
    pimpl pimpl_;
};

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONNECTOR_HPP_ */
