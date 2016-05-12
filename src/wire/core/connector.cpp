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

#include <wire/core/detail/configuration_options.hpp>

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
        po::notify(vm);

        if (!options_.config_file.empty()) {
            std::ifstream cfg(options_.config_file.c_str());
            if (cfg) {
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
        ((name + ".endpoints").c_str(), ep_value, "Adapter endpoints")
        ((name + ".timeout").c_str(),
            po::value<::std::size_t>(&aopts.timeout)->default_value(0), "Request timeout")
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
        po::parsed_options parsed_cmd =
                po::command_line_parser(unrecognized_cmd_).options(adapter_opts).
                    allow_unregistered().run();
        po::store(parsed_cmd, vm);

        if (!unrecognized_cfg_.empty()) {
            ::std::istringstream cfg(unrecognized_cfg_);
            po::parsed_options parsed_cfg = po::parse_config_file(cfg, adapter_opts, true);
            po::store(parsed_cfg, vm);
        }

        po::notify(vm);

        return aopts;
    }

    adapter_ptr
    create_adapter(identity const& id, endpoint_list const& eps = endpoint_list{})
    {
        connector_ptr conn = owner_.lock();
        if (!conn) {
            throw errors::runtime_error("Connector already destroyed");
        }
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
            detail::adapter_options opts{{}, 0, options_.client_ssl};
            bidir_adapter_ = adapter::create_adapter(conn, identity::random("client"), opts);
        }
        return bidir_adapter_;
    }

    void
    adapter_online(adapter_ptr adptr, endpoint const& ep)
    {
        lock_guard lock(mtx_);
        auto f = online_adapters_.find(ep);
        if ( f != online_adapters_.end() ) {
            ::std::cerr << "Adapter " << f->second->id() << " was listening to " << ep << "\n";
            online_adapters_.erase(f);
        }
        ::std::cerr << "Adapter " << adptr->id() << " is listening to " << ep << "\n";
        online_adapters_.emplace(ep, adptr);
    }

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
            return ::std::make_shared< object_proxy > (
                    reference::create_reference(owner_.lock(), ref_data));
        }
        throw errors::runtime_error("Invalid reference string");
    }

    connection_ptr
    get_connection(endpoint const& ep)
    {
        lock_guard lock{ mtx_ };
        auto f = outgoing_.find(ep);
        if (f != outgoing_.end()) {
            return f->second;
        } else {
            auto conn = ::std::make_shared< connection >(
            client_side{}, bidir_adapter(), ep,
            [ep](){
                ::std::cerr << "Connected to " << ep << "\n";
            },
            [ep](::std::exception_ptr ex) {
                ::std::cerr << "Failed to connect to " << ep << "\n";
            });
            outgoing_.emplace(ep, conn);
            return conn;
        }
        return connection_ptr{};
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
connector::string_to_proxy(::std::string const& str)
{
    return pimpl_->string_to_proxy(str);
}

connection_ptr
connector::get_outgoing_connection(endpoint const& ep)
{
    return pimpl_->get_connection(ep);
}

}  // namespace core
}  // namespace wire
