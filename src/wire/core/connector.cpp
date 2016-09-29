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

#include <wire/util/io_service_wait.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>

namespace wire {
namespace core {

namespace po = boost::program_options;

struct connector::impl {
    using connection_container  = ::std::unordered_map<endpoint, connection_ptr>;
    using mutex_type            = ::std::mutex;
    using lock_guard            = ::std::lock_guard<mutex_type>;
    using adapter_map           = ::std::unordered_map<endpoint, adapter_ptr>;
    using adapter_list          = ::std::list<adapter_ptr>;

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
    mutex_type                  mtx_;
    connection_container        outgoing_;
    //@}

    //@{
    /** @name Adapters */
    adapter_ptr                 bidir_adapter_;
    adapter_list                listen_adapters_;
    adapter_map                 online_adapters_;
    //@}

    //@{
    /** @name Locator */
    mutex_type                  locator_mtx_;
    locator_prx                 locator_;
    locator_registry_prx        locator_reg_;
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
    register_shutdown_observer()
    {
        auto& mon = ASIO_NS::use_service< detail::io_service_monitor >(*io_service_);
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
        ((name + ".config_file").c_str(),
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

        cmd_line_options_.add(cfg_opts)
                .add(connector_options)
                .add(server_ssl_opts)
                .add(client_ssl_opts);
        cfg_file_options_
                .add(connector_options)
                .add(server_ssl_opts)
                .add(client_ssl_opts);
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
        auto name = ::boost::lexical_cast< ::std::string >( id );
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
        ((name + ".replicated").c_str(),
            po::bool_switch(&aopts.replicated)->default_value(false),
            "Replicated adapter, should register in locator")
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
        lock_guard lock{locator_mtx_};
        locator_ = loc;
    }
    void
    get_locator_async(functional::callback< locator_prx >  result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            bool                                            run_sync)
    {
        locator_prx loc;
        {
            lock_guard lock{locator_mtx_};
            loc = locator_;
        }

        if (!loc && !options_.locator_ref.object_id.empty()) {
            #if DEBUG_OUTPUT >= 1
            ::std::cerr << "Try to otain locator proxy "
                    << options_.locator_ref << "\n";
            #endif
            try {
                if (options_.locator_ref.endpoints.empty())
                    throw errors::runtime_error{ "Locator reference must have endpoints" };
                object_prx prx = make_proxy(options_.locator_ref);
                checked_cast_async< locator_proxy >(
                    prx,
                    [this, result](locator_prx loc)
                    {
                        if (loc) {
                            lock_guard lock{locator_mtx_};
                            locator_ = loc;
                        }
                        try {
                            result(locator_);
                        } catch (...) {}
                    },
                    [exception](::std::exception_ptr ex)
                    {
                        if (exception) {
                            try {
                                exception(ex);
                            } catch (...) {}
                        }
                    }, nullptr, ctx, run_sync
                );
            } catch (...) {
                if (exception) {
                    try {
                        exception(::std::current_exception());
                    } catch (...) {}
                }
            }
        } else {
            try {
                result(loc);
            } catch (...) {}
        }
    }

    void
    get_locator_registry_async(
            functional::callback< locator_registry_prx >    result,
            functional::exception_callback                  exception,
            context_type const&                             ctx,
            bool                                            run_sync)
    {
        locator_registry_prx reg;
        {
            lock_guard lock{locator_mtx_};
            reg = locator_reg_;
        }
        if (!reg) {
            get_locator_async(
                [this, result, exception, ctx, run_sync](locator_prx loc)
                {
                    if (loc) {
                        #if DEBUG_OUTPUT >= 1
                        ::std::cerr << "Try to otain locator registry proxy from locator "
                                << *loc << "\n";
                        #endif
                        loc->get_registry_async(
                            [this, result](locator_registry_prx reg)
                            {
                                if (reg) {
                                    lock_guard lock{locator_mtx_};
                                    locator_reg_ = reg;
                                }
                                try {
                                    result(locator_reg_);
                                } catch (...) {}
                            }, exception, nullptr, ctx, run_sync);
                    } else {
                        try {
                            result(locator_reg_);
                        } catch (...) {}
                    }
                },
                exception, ctx, run_sync);
        } else {
            try {
                result(reg);
            } catch (...) {}
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
        auto adptr = adapter::create_adapter(conn, id, opts);
        listen_adapters_.push_back(adptr);
        return adptr;
    }

    adapter_ptr
    bidir_adapter()
    {
        if (!bidir_adapter_) {
            connector_ptr conn = owner_.lock();
            if (!conn) {
                throw errors::runtime_error("Connector already destroyed");
            }
            detail::adapter_options opts{{}, 0, false, false, options_.client_ssl};
            bidir_adapter_ = adapter::create_adapter(conn, identity::random("client"), opts);
        }
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
        lock_guard lock(mtx_);
        auto f = online_adapters_.find(ep);
        if ( f != online_adapters_.end() ) {
            #ifdef DEBUG_OUTPUT
            ::std::cerr << "Adapter " << f->second->id() << " was listening to " << ep << "\n";
            #endif
            online_adapters_.erase(f);
        }
        endpoint_list eps;
        ep.published_endpoints(eps);
        #ifdef DEBUG_OUTPUT
        ::std::cerr << "Adapter " << adptr->id() << " is listening to " << eps << "\n";
        #endif
        for (auto e : eps)
            online_adapters_.emplace(e, adptr);
    }
    // TODO Erase adapter on deactivation

    bool
    is_local(reference const& ref)
    {
        if (!ref.data().endpoints.empty()) {
            lock_guard lock(mtx_);
            for (auto const& ep : ref.data().endpoints) {
                if (online_adapters_.count(ep))
                    return true;
            }
        } else {
            // Check reference adapter id
        }
        return false;
    }

    object_ptr
    find_local_servant(reference const& ref)
    {
        if (!ref.data().endpoints.empty()) {
            lock_guard lock(mtx_);
            for (auto const& ep : ref.data().endpoints) {
                auto f = online_adapters_.find(ep);
                if (f != online_adapters_.end()) {
                    return f->second->find_object(ref.object_id());
                }
            }
        } else {
            // Check reference adapter id
        }
        return object_ptr{};
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
    get_outgoing(endpoint const& ep,
            connection_callback on_get,
            functional::exception_callback on_error,
            bool sync)
    {
        auto f = outgoing_.end();
        bool new_conn = false;
        connection_ptr conn;
        {
            lock_guard lock{mtx_};
            f = outgoing_.find(ep);
            if (f != outgoing_.end() ) {
                conn = f->second;
            } else {
                conn = ::std::make_shared< connection >(
                        client_side{}, bidir_adapter(), ep.transport(),
                        [this](connection const* c)
                        {
                            erase_outgoing(c);
                        });
                outgoing_.emplace(ep, conn);
                new_conn = true;
            }
        }
        if (new_conn) {
            // TODO move connect outside the mutex
            auto res = ::std::make_shared< ::std::atomic<bool> >(false);
            conn->connect_async(ep,
                [on_get, conn, res, ep]()
                {
                    #ifdef DEBUG_OUTPUT
                    ::std::cerr << "Connected to " << ep << "\n";
                    #endif
                    *res = true;
                    try {
                        on_get(conn);
                    } catch(...) {}
                },
                [this, on_error, conn, res, ep](::std::exception_ptr ex)
                {
                    #ifdef DEBUG_OUTPUT
                    ::std::cerr << "Failed to connect to " << ep << "\n";
                    #endif
                    *res = true;
                    erase_outgoing(conn.get());
                    try {
                        on_error(ex);
                    } catch (...) {}
                });

            if (sync) {
                util::run_until(io_service_, [res](){ return (bool)*res; });
            }
        } else {
            try {
                on_get(conn);
            } catch(...) {}
        }
    }

    void
    erase_outgoing(connection const* conn)
    {
        lock_guard lock{mtx_};
        for (auto c = outgoing_.begin(); c != outgoing_.end(); ++c) {
            if (c->second.get() == conn) {
                #ifdef DEBUG_OUTPUT
                ::std::cerr << "Outgoing connection to " << c->first << " closed\n";
                #endif
                outgoing_.erase(c);
                break;
            }
        }
    }
};

connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc)
{
    connector_ptr c(new connector(svc));
    c->pimpl_->owner_ = c;
    return c;
}

connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc, ::std::string const& name)
{
    connector_ptr c(new connector(svc, name));
    c->pimpl_->owner_ = c;
    return c;
}

template < typename ... T >
connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc, T&& ... args)
{
    connector_ptr c(new connector(svc));
    c->pimpl_->owner_ = c;
    c->pimpl_->configure(::std::forward<T>(args) ...);
    return c;
}

template < typename ... T >
connector_ptr
connector::do_create_connector(asio_config::io_service_ptr svc,
        ::std::string const& name, T&& ... args)
{
    connector_ptr c(new connector(svc, name));
    c->pimpl_->owner_ = c;
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

object_ptr
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

connection_ptr
connector::get_outgoing_connection(endpoint const& ep)
{
    auto future = get_outgoing_connection_async(ep, true);
    return future.get();
}

void
connector::get_outgoing_connection_async(endpoint const& ep,
        connection_callback             on_get,
        functional::exception_callback  on_error,
        bool sync)
{
    pimpl_->get_outgoing(ep, on_get, on_error, sync);
}

void
connector::set_locator(locator_prx loc)
{
    pimpl_->set_locator(loc);
}

locator_prx
connector::get_locator(context_type const& ctx) const
{
    auto future = get_locator_async(ctx, true);
    return future.get();
}

void
connector::get_locator_async(
        functional::callback<locator_prx>   result,
        functional::exception_callback      exception,
        context_type const&                 ctx,
        bool                                run_sync) const
{
    pimpl_->get_locator_async(result, exception, ctx, run_sync);
}

locator_registry_prx
connector::get_locator_registry(context_type const& ctx) const
{
    auto future = get_locator_registry_async(ctx, true);
    return future.get();
}

void
connector::get_locator_registry_async(
        functional::callback<locator_registry_prx>  result,
        functional::exception_callback              exception,
        context_type const&                         ctx,
        bool                                        run_sync) const
{
    pimpl_->get_locator_registry_async(result, exception, ctx, run_sync);
}

}  // namespace core
}  // namespace wire
