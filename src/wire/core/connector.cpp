/*
 * connector.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: zmij
 */

#include <wire/core/connector.hpp>
#include <wire/core/identity.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/reference.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/proxy.hpp>

#include <wire/core/locator.hpp>
#include <wire/core/connector_admin_impl.hpp>
#include <wire/core/adapter_admin_impl.hpp>

#include <wire/core/detail/configuration_options.hpp>
#include <wire/core/detail/io_service_monitor.hpp>
#include <wire/core/detail/reference_resolver.hpp>

#include <wire/util/io_service_wait.hpp>

#include <boost/program_options.hpp>

#include <tbb/concurrent_hash_map.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>

namespace wire {
namespace core {

namespace po = boost::program_options;

struct connector::impl {
    using mutex_type            = ::std::mutex;
    using lock_guard            = ::std::lock_guard<mutex_type>;

    using connection_container  = ::tbb::concurrent_hash_map<endpoint, connection_ptr>;
    using connection_accessor   = connection_container::accessor;
    using connection_const_accessor = connection_container::const_accessor;

    using adapter_map           = ::tbb::concurrent_hash_map<endpoint, adapter_ptr>;
    using adapter_accessor      = adapter_map::accessor;
    using adapter_const_accessor = adapter_map::const_accessor;

    using adapter_list          = ::std::list<adapter_ptr>;

    using locator_map           = ::tbb::concurrent_hash_map<reference_data,
                                            detail::reference_resolver>;

    connector_weak_ptr          owner_;
    asio_config::io_service_ptr io_service_;

    //@{
    /** @name Configuration */
    detail::connector_options   options_;

    po::options_description     cmd_line_options_;
    po::options_description     cfg_file_options_;

    args_type                   unrecognized_cmd_;
    ::std::string               unrecognized_cfg_;
    //@}

    //@{
    /** @name Connections */
    connection_container        outgoing_;
    //@}

    //@{
    /** @name Adapters */
    //mutex_type                  mtx_;
    ::std::once_flag            bidir_created_;
    adapter_ptr                 bidir_adapter_;

    adapter_list                listen_adapters_;
    adapter_map                 online_adapters_;
    //@}

    //@{
    /** @name Locator */
    detail::reference_resolver  default_resolver_;
    locator_map                 locators_;
    //@}

    //@{
    /** @name Connection observers */
    connection_observer_set     connection_observers_;
    //@}

    impl(asio_config::io_service_ptr svc)
        : io_service_{svc}, options_{},
          cmd_line_options_{ options_.name + " command line options" },
          cfg_file_options_{ options_.name + " config file options" }
    {
        register_shutdown_observer();
        create_options_description();
    }

    impl(asio_config::io_service_ptr svc, ::std::string const& name)
        : io_service_{svc}, options_{name},
          cmd_line_options_{ options_.name + " command line options" },
          cfg_file_options_{ options_.name + " config file options" }
    {
        register_shutdown_observer();
        create_options_description();
    }

    void
    set_owner(connector_ptr cnctr)
    {
        owner_ = cnctr;
        default_resolver_.set_owner(cnctr);
    }

    void
    register_shutdown_observer()
    {
        auto& mon = asio_ns::use_service< detail::io_service_monitor >(*io_service_);
        mon.add_observer([](){});
    }

    void
    handle_shutdown()
    {
        ;
    }

    void
    create_options_description()
    {
        ::std::string const& name = options_.name;

        po::options_description cfg_opts("Configuration file");
        cfg_opts.add_options()
        ((name + ".config_file,c").c_str(),
                    po::value<::std::string>(&options_.config_file),
                    "Configuration file")
        ;
        po::options_description connector_options("Connector options");
        connector_options.add_options()
        ((name + ".locator").c_str(),
                po::value<reference_data>(&options_.locator_ref),
                "Locator proxy")
        ((name + ".admin.endpoints").c_str(),
                po::value<endpoint_list>(&options_.admin_endpoints),
                "Connector admin enpoints")
        ((name + ".admin.adapter").c_str(),
                po::value<identity>(&options_.admin_adapter),
                "Identity for admin adapter")
        ((name + ".admin.connector").c_str(),
                po::value<identity>(&options_.admin_connector),
                "Identity for connector admin")
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
        po::options_description connection_mgmt_opts("Connection management options");
        connection_mgmt_opts.add_options()
        ((name + ".cm.enable").c_str(),
                po::bool_switch(&options_.enable_connection_timeouts)->default_value(false),
                "Enable connection management")
        ((name + ".cm.timeout").c_str(),
                po::value<int32_t>(&options_.connection_idle_timeout)->default_value(30000),
                "Connection timeout, in milliseconds")
        ((name + ".cm.request_timeout").c_str(),
                po::value<::std::size_t>(&options_.request_timeout)->default_value(5000),
                "Request timeout, in milliseconds")
        ;

        cmd_line_options_.add(cfg_opts)
                .add(connector_options)
                .add(server_ssl_opts)
                .add(client_ssl_opts)
                .add(connection_mgmt_opts);
        cfg_file_options_
                .add(connector_options)
                .add(server_ssl_opts)
                .add(client_ssl_opts)
                .add(connection_mgmt_opts);
    }

    void
    parse_config_file(::std::istream& cfg)
    {
        po::variables_map vm;
        po::parsed_options parsed_cfg = po::parse_config_file(cfg, cfg_file_options_, true);
        po::store(parsed_cfg, vm);
        args_type unrecognized_params = po::collect_unrecognized(parsed_cfg.options, po::exclude_positional);
        ::std::ostringstream cfg_out;
        // Output configuration to a "file"
        for (args_type::iterator p = unrecognized_params.begin();
                p != unrecognized_params.end(); ++p) {
            auto optname = p++;
            if (p == unrecognized_params.end())
                break;
            cfg_out << *optname << "=" << *p << "\n";
        }
        unrecognized_cfg_ = cfg_out.str();
        po::notify(vm);
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
        po::notify(vm);

        if (!options_.config_file.empty()) {
            ::std::ifstream cfg(options_.config_file.c_str());
            if (cfg) {
                parse_config_file(cfg);
            } else {
                throw ::std::runtime_error(
                    "Failed to open configuration file " + options_.config_file );
            }
        }
    }

    void
    apply_options()
    {
        if (!options_.admin_endpoints.empty()) {
            create_connector_admin();
        }
    }

    void
    configure(int argc, char* argv[])
    {
        parse_configuration(argc, argv);
        apply_options();
    }

    void
    configure(args_type const& args, ::std::string const& cfg_str = ::std::string{})
    {
        parse_configuration(args);
        if (!cfg_str.empty()) {
            ::std::istringstream cfg{cfg_str};
            parse_config_file(cfg);
        }
        apply_options();
    }

    void
    configure(::std::string const& cfg_str)
    {
        if (!cfg_str.empty()) {
            ::std::istringstream cfg{cfg_str};
            parse_config_file(cfg);
        }
        apply_options();
    }

    void
    create_connector_admin()
    {
        adapter_ptr adm = create_adapter(options_.admin_adapter, options_.admin_endpoints);
        adm->activate();
        adm->add_object(options_.admin_connector,
                ::std::make_shared< connector_admin_impl >( owner_.lock(), adm ));
        auto adapt_adm = ::std::make_shared< adapter_admin_impl >( owner_.lock() );
        adm->add_default_servant(adapt_adm);
    }

    detail::adapter_options
    configure_adapter(identity const& id, endpoint_list const& eps = endpoint_list{})
    {
        ::std::string name;
        if (id.category.empty()) {
            name = ::boost::lexical_cast< ::std::string >( id );
        } else {
            name = id.category;
        }
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
        ((name + ".endpoints").c_str(), ep_value,
            "Adapter endpoints")
        ((name + ".timeout").c_str(),
            po::value<::std::size_t>(&aopts.timeout)->default_value(0),
            "Request timeout")
        ((name + ".register").c_str(),
            po::bool_switch(&aopts.registered)->default_value(true),
            "Register adapter in locator, if any")
        ((name + ".register_retries").c_str(),
            po::value<int>(&aopts.register_retries)->default_value(
                    invocation_options::default_timout / invocation_options::default_retry_timout),
            "Register retry count (-1 = disabled, 0 = infinite)")
        ((name + ".retry_timeout").c_str(),
            po::value<::std::size_t>(&aopts.retry_timeout)->default_value(invocation_options::default_retry_timout),
            "Register retry timeout")
        ((name + ".replicated").c_str(),
            po::bool_switch(&aopts.replicated)->default_value(false),
            "Replicated adapter, should register in locator")
        ((name + ".locator").c_str(),
                po::value<reference_data>(&aopts.locator_ref)->default_value(options_.locator_ref),
                "Locator proxy")
        ;


        po::options_description adapter_ssl_opts(name + " Adapter SSL Options");
        adapter_ssl_opts.add_options()
        ((name + ".ssl.certificate").c_str(),
                po::value<::std::string>(&aopts.adapter_ssl.cert_file),
                "SSL certificate")
        ((name + ".ssl.key").c_str(),
                po::value<::std::string>(&aopts.adapter_ssl.key_file),
                "SSL key file")
        ((name + ".ssl.verify_file").c_str(),
                po::value<::std::string>(&aopts.adapter_ssl.verify_file),
                "SSL verify file (CA root). If missing, default system certificates are used")
        ((name + ".ssl.require_peer_cert").c_str(),
                po::bool_switch(&aopts.adapter_ssl.require_peer_cert),
                "Require client SSL certificate")
        ;

        adapter_opts.add(adapter_general_opts).add(adapter_ssl_opts);

        po::variables_map vm;
        configure_options(adapter_opts, vm);
        return aopts;
    }

    void
    configure_options(po::options_description const& desc, po::variables_map& vm)
    {
        po::parsed_options parsed_cmd =
                po::command_line_parser(unrecognized_cmd_).options(desc).
                    allow_unregistered().run();
        po::store(parsed_cmd, vm);

        if (!unrecognized_cfg_.empty()) {
            ::std::istringstream cfg(unrecognized_cfg_);
            po::parsed_options parsed_cfg = po::parse_config_file(cfg, desc, true);
            po::store(parsed_cfg, vm);
        }
        po::notify(vm);
    }

    void
    stop()
    {
        for (auto a : listen_adapters_) {
            a->deactivate();
        }
    }

    void
    set_locator(locator_prx loc)
    {
        default_resolver_.set_locator(loc);
    }
    void
    get_locator_async(functional::callback< locator_prx >   result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            invocation_options const&                       opts)
    {
        default_resolver_.get_locator_async(result, exception, ctx, opts);
    }

    void
    get_locator_registry_async(
            functional::callback< locator_registry_prx >    result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            invocation_options const&                       opts)
    {
        default_resolver_.get_locator_registry_async(result, exception, ctx, opts);
    }

    void
    get_locator_async(reference_data const& locator_ref,
            functional::callback< locator_prx >             result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            invocation_options const&                       opts)
    {
        if (locator_ref == options_.locator_ref) {
            default_resolver_.get_locator_async(result, exception, ctx, opts);
        } else {
            locator_map::accessor acc;
            if (!locators_.find(acc, locator_ref)) {
                locators_.emplace(acc, locator_ref,
                        detail::reference_resolver(owner_.lock(), locator_ref));
            }
            acc->second.get_locator_async(result, exception, ctx, opts);
        }
    }

    void
    get_locator_registry_async(
            reference_data const& locator_ref,
            functional::callback< locator_registry_prx >    result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            invocation_options const&                       opts)
    {
        if (locator_ref == options_.locator_ref) {
            default_resolver_.get_locator_registry_async(result, exception, ctx, opts);
        } else {
            locator_map::accessor acc;
            if (!locators_.find(acc, locator_ref)) {
                locators_.emplace(acc, locator_ref,
                        detail::reference_resolver(owner_.lock(), locator_ref));
            }
            acc->second.get_locator_registry_async(result, exception, ctx, opts);
        }
    }

    void
    resolve_connection(reference_data const&    ref,
        functional::callback<connection_ptr>    result,
        functional::exception_callback          exception,
        invocation_options const&               opts)
    {
        if (!ref.locator || ref.locator->wire_get_reference()->data() == options_.locator_ref) {
            // Use default locator
            default_resolver_.resolve_reference_async(ref, result, exception, opts);
        } else {
            auto const& loc_ref = ref.locator->wire_get_reference()->data();
            locator_map::accessor acc;
            if (!locators_.find(acc, loc_ref)) {
                locators_.emplace(acc, loc_ref, detail::reference_resolver(owner_.lock(), loc_ref));
            }
            acc->second.resolve_reference_async(ref, result, exception, opts);
        }
    }

    adapter_ptr
    create_adapter(identity const& id, endpoint_list const& eps = endpoint_list{})
    {
        connector_ptr conn = owner_.lock();
        if (!conn) {
            throw errors::runtime_error("Connector already destroyed");
        }
        if (find_adapter(id))
            throw errors::runtime_error("Adapter ", id, " is already configured");
        detail::adapter_options opts = configure_adapter(id, eps);
        auto adptr = adapter::create_adapter(conn, id, opts, connection_observers_);
        listen_adapters_.push_back(adptr);
        return adptr;
    }

    void create_bidir_adapter()
    {
        connector_ptr conn = owner_.lock();
        if (!conn) {
            throw errors::runtime_error("Connector already destroyed");
        }
        detail::adapter_options opts { {}, 0, false, -1, 0, false, options_.client_ssl};
            bidir_adapter_ = adapter::create_adapter(
                conn, identity::random("client"), opts, connection_observers_);
    }

    void
    add_observer(connection_observer_ptr observer)
    {
        connection_observers_.insert(observer);
        if (bidir_adapter_)
            bidir_adapter_->add_observer(observer);
        for (auto a : listen_adapters_) {
            a->add_observer(observer);
        }
    }
    void
    remove_observer(connection_observer_ptr observer)
    {
        connection_observers_.erase(observer);
        if (bidir_adapter_)
            bidir_adapter_->remove_observer(observer);
        for (auto a : listen_adapters_) {
            a->remove_observer(observer);
        }
    }

    adapter_ptr
    bidir_adapter()
    {
        ::std::call_once(bidir_created_, [this](){ create_bidir_adapter(); });
        return bidir_adapter_;
    }

    adapter_ptr
    find_adapter(identity const& id)
    {
        for (auto const& a: listen_adapters_) {
            if (a->id() == id) {
                return a;
            }
        }
        return adapter_ptr{};
    }
    identity_seq
    adapter_ids()
    {
        identity_seq ids;
        for (auto const& a : listen_adapters_) {
            ids.push_back( a->id() );
        }
        return ids;
    }

    void
    adapter_online(adapter_ptr adptr, endpoint const& ep)
    {
        endpoint_list eps;
        ep.published_endpoints(eps);
        for (auto const& e : eps) {
            online_adapters_.erase(e);
        }
        #if DEBUG_OUTPUT >= 1
        tag(::std::cerr) << " Adapter " << adptr->id() << " is listening to " << eps << "\n";
            #endif
        for (auto e : eps)
            online_adapters_.emplace(e, adptr);
        }
    // TODO Erase adapter on deactivation
    void
    adapter_offline(adapter_ptr adptr, endpoint const& ep) {
        endpoint_list eps;
        ep.published_endpoints(eps);
        #if DEBUG_OUTPUT >= 1
        tag(::std::cerr) << " Adapter " << adptr->id() << " listening to " << eps << " went offline\n";
        #endif
        for (auto const& e : eps) {
            online_adapters_.erase(e);
    }
    }

    bool
    is_local(reference const& ref)
    {
        if (!ref.data().endpoints.empty()) {
            for (auto const& ep : ref.data().endpoints) {
                if (online_adapters_.count(ep))
                    return true;
            }
        } else {
            // Check reference adapter id
        }
        return false;
    }

    local_servant
    find_local_servant(reference const& ref)
    {
        if (!ref.data().endpoints.empty()) {
            for (auto const& ep : ref.data().endpoints) {
                adapter_const_accessor acc;
                if (online_adapters_.find(acc, ep)) {
                    return {
                        acc->second->find_object(ref.object_id()),
                        acc->second
                    };
                }
            }
        } else {
            // Check reference adapter id
        }
        return {object_ptr{}, adapter_ptr{}};
    }

    object_prx
    string_to_proxy(::std::string const& name)
    {
        ::std::istringstream is{name};
        reference_data ref_data;
        if ((bool)(is >> ref_data)) {
            return make_proxy(ref_data);
        }
        throw errors::runtime_error("Invalid reference string");
    }

    object_prx
    make_proxy(reference_data const& ref_data)
    {
        if (ref_data.object_id.empty())
            throw errors::runtime_error{ "Empty object identity in reference" };
        return ::std::make_shared< object_proxy > (
                reference::create_reference(owner_.lock(), ref_data));
    }

    void
    get_outgoing(endpoint const&            ep,
            connection_callback             on_get,
            functional::exception_callback  exception,
            invocation_options const&       opts)
    {
        bool new_conn = false;
        connection_ptr conn;
        {
            connection_accessor acc;
            if (outgoing_.find(acc, ep)) {
                conn = acc->second;
            } else {
                conn = ::std::make_shared< connection >(
                        client_side{}, bidir_adapter(), ep.transport(),
                        [this, ep](connection const* c)
                        {
                            erase_outgoing(ep);
                        });
                new_conn = outgoing_.emplace(acc, ep, conn);
                conn = acc->second;
            }
        }
        if (new_conn) {
            auto res = ::std::make_shared< ::std::atomic<bool> >(false);
            #if DEBUG_OUTPUT >= 2
            ::std::ostringstream os;
            tag(os) << " Start connection to " << ep << "\n";
            ::std::cerr << os.str();
            #endif
            conn->connect_async(ep,
                [on_get, conn, res, ep]()
                {
                    #ifdef DEBUG_OUTPUT
                    tag(::std::cerr) << " Connected to " << ep << "\n";
                    #endif
                    *res = true;
                    try {
                        on_get(conn);
                    } catch(...) {
                        #ifdef DEBUG_OUTPUT
                        ::std::cerr << getpid() << " Exception while setting connection\n";
                        #endif
                    }
                },
                [this, exception, res, ep](::std::exception_ptr ex)
                {
                    #if DEBUG_OUTPUT >= 2
                    tag(::std::cerr) << " Failed to connect to " << ep << "\n";
                    #endif
                    *res = true;
                    try {
                        exception(ex);
                    } catch (...) {}
                });

            if (opts.is_sync()) {
                util::run_until(io_service_, [res](){ return (bool)*res; });
            }
        } else {
            #if DEBUG_OUTPUT >= 2
            ::std::ostringstream os;
            tag(os) << " Connection to " << ep << " already initiated\n";
            ::std::cerr << os.str();
            #endif
            try {
                on_get(conn);
            } catch(...) {
                #ifdef DEBUG_OUTPUT
                ::std::cerr << getpid() << " Exception while setting connection\n";
                #endif
            }
        }
    }

    void
    erase_outgoing(endpoint ep)
    {
        #if DEBUG_OUTPUT >= 1
        tag(::std::cerr) << " Outgoing connection to " << ep << " closed\n";
        #endif
        connection_accessor acc;
        if (outgoing_.find(acc, ep)) {
            #if DEBUG_OUTPUT >= 1
            tag(::std::cerr) << " Erase connection to " << ep << "\n";
            #endif
            outgoing_.erase(acc);
        }
    }

    static ::std::ostream&
    tag(::std::ostream& os)
    {
        os << ::getpid() << " (connector) ";
        return os;
    }
};

connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc)
{
    connector_ptr c(new connector(svc));
    c->pimpl_->set_owner(c);
    return c;
}

connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc, ::std::string const& name)
{
    connector_ptr c(new connector(svc, name));
    c->pimpl_->set_owner(c);
    return c;
}

template < typename ... T >
connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc, T&& ... args)
{
    connector_ptr c(new connector(svc));
    c->pimpl_->set_owner(c);
    c->pimpl_->configure(::std::forward<T>(args) ...);
    return c;
}

template < typename ... T >
connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc,
        ::std::string const& name, T&& ... args)
{
    connector_ptr c(new connector(svc, name));
    c->pimpl_->set_owner(c);
    c->pimpl_->configure(::std::forward<T>(args) ...);
    return c;
}

connector_ptr
connector::create_connector()
{
    asio_config::io_service_ptr svc = ::std::make_shared< asio_config::io_service >();
    return do_create_connector(svc);
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
connector::create_connector(asio_config::io_service_ptr svc,
        args_type const& args, ::std::string const& cfg_str)
{
    return do_create_connector(svc, args, cfg_str);
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

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, ::std::string const& name,
        args_type const& args, ::std::string const& cfg_str)
{
    return do_create_connector(svc, name, args, cfg_str);
}

connector_ptr
connector::create_connector(asio_config::io_service_ptr svc, ::std::string const& name,
        ::std::string const& cfg_str)
{
    return do_create_connector(svc, name, cfg_str);
}
//----------------------------------------------------------------------------

connector::connector(asio_config::io_service_ptr svc)
    : pimpl_(new impl{svc})
{
}

connector::connector(asio_config::io_service_ptr svc, ::std::string const& name)
    : pimpl_(new impl{svc, name})
{
}

connector::~connector() = default;

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

void
connector::configure_options(po::options_description const& desc, po::variables_map& vm) const
{
    pimpl_->configure_options(desc, vm);
}

asio_config::io_service_ptr
connector::io_service()
{
    return pimpl_->io_service_;
}

::std::string const&
connector::name() const
{
    return pimpl_->options_.name;
}

void
connector::run()
{
    pimpl_->io_service_->run();
}

void
connector::stop()
{
    pimpl_->stop();
}

detail::connector_options const&
connector::options() const
{
    return pimpl_->options_;
}

adapter_ptr
connector::create_adapter(identity const& id)
{
    return pimpl_->create_adapter(id);
}

adapter_ptr
connector::create_adapter(identity const& id, endpoint_list const& eps)
{
    return pimpl_->create_adapter(id, eps);
}

adapter_ptr
connector::bidir_adapter()
{
    return pimpl_->bidir_adapter();
}

adapter_ptr
connector::find_adapter(identity const& id) const
{
    return pimpl_->find_adapter(id);
}

identity_seq
connector::adapter_ids() const
{
    return pimpl_->adapter_ids();
}

void
connector::adapter_online(adapter_ptr adptr, endpoint const& ep)
{
    pimpl_->adapter_online(adptr, ep);
}

bool
connector::is_local(reference const& ref) const
{
    return pimpl_->is_local(ref);
}

connector::local_servant
connector::find_local_servant(reference const& ref) const
{
    return pimpl_->find_local_servant(ref);
}

object_prx
connector::string_to_proxy(::std::string const& str) const
{
    return pimpl_->string_to_proxy(str);
}

object_prx
connector::make_proxy(reference_data const& ref) const
{
    return pimpl_->make_proxy(ref);
}

void
connector::get_outgoing_connection_async(endpoint const& ep,
        connection_callback                 on_get,
        functional::exception_callback      on_error,
        invocation_options const&           opts)
{
    pimpl_->get_outgoing(ep, on_get, on_error, opts);
}

void
connector::resolve_connection_async(reference_data const&   ref,
        functional::callback<connection_ptr>                result,
        functional::exception_callback                      exception,
        invocation_options const&                           opts) const
{
    // TODO Lookup bidir connections by adapter
    pimpl_->resolve_connection(ref, result, exception, opts);
}

void
connector::set_locator(locator_prx loc)
{
    pimpl_->set_locator(loc);
}

void
connector::get_locator_async(
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception,
        context_type const&                 ctx,
        invocation_options const&           opts) const
{
    pimpl_->get_locator_async(result, exception, ctx, opts);
}

void
connector::get_locator_async(
        reference_data const&               loc_ref,
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception,
        context_type const&                 ctx,
        invocation_options const&           opts) const
{
    pimpl_->get_locator_async(loc_ref, result, exception, ctx, opts);
}

void
connector::get_locator_registry_async(
        functional::callback<locator_registry_prx>  result,
        functional::exception_callback              exception,
        context_type const&                         ctx,
        invocation_options const&                   opts) const
{
    pimpl_->get_locator_registry_async(result, exception, ctx, opts);
}

void
connector::get_locator_registry_async(
        reference_data const&                       loc_ref,
        functional::callback<locator_registry_prx>  result,
        functional::exception_callback              exception,
        context_type const&                         ctx,
        invocation_options const&                   opts) const
{
    pimpl_->get_locator_registry_async(loc_ref, result, exception, ctx, opts);
}

void
connector::add_observer(connection_observer_ptr observer)
{
    pimpl_->add_observer(observer);
}

void
connector::remove_observer(connection_observer_ptr observer)
{
    pimpl_->remove_observer(observer);
}

}  // namespace core
}  // namespace wire
