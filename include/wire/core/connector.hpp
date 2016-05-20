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
#include <wire/core/identity.hpp>

#include <wire/core/connector_fwd.hpp>
#include <wire/core/connection_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/reference_fwd.hpp>
#include <wire/core/object_fwd.hpp>
#include <wire/core/functional.hpp>

#include <future>

namespace wire {
namespace core {

class connector : public ::std::enable_shared_from_this<connector> {
public:
    using args_type             = ::std::vector<::std::string>;
    using connection_callback   = ::std::function< void(connection_ptr) >;
public:
    static connector_ptr
    create_connector();
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, int argc, char* argv[]);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, args_type const&);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, args_type const&, ::std::string const& cgf);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, ::std::string const& name);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, ::std::string const& name, int argc, char* argv[]);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, ::std::string const& name, args_type const&);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, ::std::string const& name, args_type const&, ::std::string const& cfg);
    static connector_ptr
    create_connector(asio_config::io_service_ptr svc, ::std::string const& name, ::std::string const& cfg);
private:
    connector(asio_config::io_service_ptr svc);
    connector(asio_config::io_service_ptr svc, ::std::string const& name);

    static connector_ptr
    do_create_connector(asio_config::io_service_ptr svc);
    static connector_ptr
    do_create_connector(asio_config::io_service_ptr svc, ::std::string const& name);
    template< typename ... T >
    static connector_ptr
    do_create_connector(asio_config::io_service_ptr svc, T&& ... args);
    template < typename ... T >
    static connector_ptr
    do_create_connector(asio_config::io_service_ptr svc, ::std::string const& name, T&& ... args);
public:
    ~connector();
    void
    confugure(int argc, char* argv[]);
    void
    configure(args_type const&);
    void
    configure(args_type const& cmd, ::std::string const& cfg);
    void
    configure(::std::string const& cfg);

    asio_config::io_service_ptr
    io_service();

    void
    run();
    void
    stop();

    //@{
    /** @name Adapters */
    adapter_ptr
    create_adapter(identity const& id);
    adapter_ptr
    create_adapter(identity const& id, endpoint_list const& eps);
    adapter_ptr
    bidir_adapter();

    adapter_ptr
    find_adapter(identity const& id) const;
    identity_seq
    adapter_ids() const;
    void
    adapter_online(adapter_ptr, endpoint const& ep);
    //@}

    bool
    is_local(reference const& ref) const;
    object_ptr
    find_local_servant(reference const& ref) const;

    object_prx
    string_to_proxy(::std::string const& str) const;

    object_prx
    make_proxy(reference_data const& ref) const;

    /**
     * Get an existing connection to the endpoint specified or create a new one.
     * @param
     * @return
     */
    connection_ptr
    get_outgoing_connection(endpoint const&);

    template < template <typename> class _Promise = ::std::promise >
    auto
    get_outgoing_connection_async(endpoint const& ep, bool sync = false)
        -> decltype(::std::declval<_Promise<connection_ptr>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<connection_ptr> >();
        get_outgoing_connection_async(ep,
            [promise](connection_ptr val)
            {
                promise->set_value(val);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, sync
        );

        return promise->get_future();
    }
    void
    get_outgoing_connection_async(endpoint const&,
            connection_callback             on_get,
            functional::exception_callback  on_error,
            bool sync = false);
private:
    connector(connector const&) = delete;
    connector(connector&&) = delete;
    connector&
    operator = (connector const&) = delete;
    connector&
    operator = (connector&&) = delete;
    struct impl;
    typedef ::std::unique_ptr<impl> pimpl;
    pimpl pimpl_;
};

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONNECTOR_HPP_ */
