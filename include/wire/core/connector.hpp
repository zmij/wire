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
#include <wire/future_config.hpp>

#include <wire/core/endpoint.hpp>
#include <wire/core/identity.hpp>

#include <wire/core/connector_fwd.hpp>
#include <wire/core/connection_fwd.hpp>
#include <wire/core/adapter_fwd.hpp>
#include <wire/core/proxy_fwd.hpp>
#include <wire/core/reference_fwd.hpp>
#include <wire/core/object_fwd.hpp>
#include <wire/core/locator_fwd.hpp>
#include <wire/core/connection_observer_fwd.hpp>
#include <wire/core/detail/configuration_options_fwd.hpp>

#include <wire/core/functional.hpp>
#include <wire/core/context.hpp>

#include <wire/core/detail/future_traits.hpp>

#include <future>

namespace boost {
namespace program_options {

class options_description;
class variables_map;

}  /* namespace program_options */
}  /* namespace boost */

namespace wire {
namespace core {

class connector : public ::std::enable_shared_from_this<connector> {
public:
    using args_type             = ::std::vector<::std::string>;
    using connection_callback   = ::std::function< void(connection_ptr) >;
    using local_servant         = ::std::pair<object_ptr, adapter_ptr>;
public:
    //@{
    /** @name Connector creation functions */
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
    //@}
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
    //@{
    /** @name Configuration */
    /**
     * Configure connector with a command line
     * @param argc
     * @param argv
     */
    void
    confugure(int argc, char* argv[]);
    /**
     * Configure connector with a vector of command line arguments
     * @param
     */
    void
    configure(args_type const&);
    /**
     * Configure connector with a vector of command line arguments and
     * configuration file contents
     * @param cmd
     * @param cfg
     */
    void
    configure(args_type const& cmd, ::std::string const& cfg);
    /**
     * Configure connector with configuration file contents
     * @param cfg
     */
    void
    configure(::std::string const& cfg);

    /**
     * Connector options access
     * @return
     */
    detail::connector_options const&
    options() const;

    /**
     * Configure external options with the unrecognized options captured
     * on configuration step.
     * Particularly useful for services.
     * @param desc
     * @param vm
     */
    void
    configure_options(::boost::program_options::options_description const& desc,
            ::boost::program_options::variables_map& vm) const;
    //@}

    asio_config::io_service_ptr
    io_service();

    ::std::string const&
    name() const;

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

    //@{
    /** @name References */
    bool
    is_local(reference const& ref) const;
    local_servant
    find_local_servant(reference const& ref) const;

    object_prx
    string_to_proxy(::std::string const& str) const;

    object_prx
    make_proxy(reference_data const& ref) const;
    //@}

    //@{
    /** @name Connections */
    /**
     * Get an existing connection to the endpoint specified or create a new one.
     * @param
     * @return
     */
    template < template <typename> class _Promise = promise >
    connection_ptr
    get_outgoing_connection(endpoint const& ep)
    {
        auto future = get_outgoing_connection_async< _Promise >(
                ep, detail::promise_want_io_throttle<_Promise<connection_ptr>>::value );
        return future.get();
    }

    template < template <typename> class _Promise = promise >
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

    template < template <typename> class _Promise = promise >
    connection_ptr
    resolve_connection(reference_data const& ref) const
    {
        auto future = resolve_connection_async<_Promise>(
                ref, detail::promise_want_io_throttle<_Promise<connection_ptr>>::value );
        return future.get();
    }

    void
    resolve_connection_async(reference_data const& ref,
        functional::callback<connection_ptr> result,
        functional::exception_callback      exception   = nullptr,
        bool                                run_sync    = false
    ) const;

    template < template <typename> class _Promise = promise >
    auto
    resolve_connection_async(reference_data const& ref,
        bool                                run_sync    = false) const
        -> decltype(::std::declval<_Promise<connection_ptr>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<connection_ptr> >();

        resolve_connection_async(ref,
            [promise](connection_ptr res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, run_sync
        );

        return promise->get_future();
    }

    //@}

    //@{
    /** @name Locator */
    template < template <typename> class _Promise = promise >
    locator_prx
    get_locator(context_type const& ctx = no_context) const
    {
        auto future = get_locator_async<_Promise>(
                ctx, detail::promise_want_io_throttle<_Promise<locator_prx>>::value);
        return future.get();
    }
    void
    get_locator_async(
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception   = nullptr,
        context_type const&                             = no_context,
        bool                                run_sync    = false
    ) const;

    template < template <typename> class _Promise = promise >
    auto
    get_locator_async(
        context_type const&                 ctx         = no_context,
        bool                                run_sync    = false) const
        -> decltype(::std::declval<_Promise<locator_prx>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<locator_prx> >();

        get_locator_async(
            [promise](locator_prx res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, ctx, run_sync
        );

        return promise->get_future();
    }
    void
    set_locator(locator_prx);


    template < template <typename> class _Promise = promise >
    locator_registry_prx
    get_locator_registry(context_type const& ctx = no_context) const
    {
        auto future = get_locator_registry_async<_Promise>(
                ctx, detail::promise_want_io_throttle<_Promise<locator_registry_prx>>::value);
        return future.get();
    }

    void
    get_locator_registry_async(
        functional::callback<locator_registry_prx> result,
        functional::exception_callback      exception   = nullptr,
        context_type const&                             = no_context,
        bool                                run_sync    = false
    ) const;

    template < template <typename> class _Promise = promise >
    auto
    get_locator_registry_async(
        context_type const&                 ctx         = no_context,
        bool                                run_sync    = false) const
        -> decltype(::std::declval<_Promise<locator_registry_prx>>().get_future())
    {
        auto promise = ::std::make_shared< _Promise<locator_registry_prx> >();

        get_locator_registry_async(
            [promise](locator_registry_prx res)
            {
                promise->set_value(res);
            },
            [promise](::std::exception_ptr ex)
            {
                promise->set_exception(ex);
            }, ctx, run_sync
        );

        return promise->get_future();
    }
    //@}

    //@{
    /** @name Connection observers */
    void
    add_observer(connection_observer_ptr observer);
    void
    remove_observer(connection_observer_ptr observer);
    //@}
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
