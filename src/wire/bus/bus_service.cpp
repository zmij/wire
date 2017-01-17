/*
 * bus_service.cpp
 *
 *  Created on: 16 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/bus/bus_service.hpp>
#include <wire/bus/bus_options.hpp>
#include <wire/bus/bus_impl.hpp>

#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>

#include <wire/util/make_unique.hpp>

#include <boost/program_options.hpp>

namespace wire {
namespace svc {

struct bus_service::impl {
    ::std::string           name_;

    core::adapter_ptr       bus_adapter_;
    core::adapter_ptr       publisher_adapter_;
    bus::bus_registry_prx   registry_;

    impl(::std::string const& name)
        : name_{name} {}

    void
    start(core::connector_ptr cnctr)
    {
        namespace po = ::boost::program_options;
        bus_options opts;
        po::options_description opts_desc{"Bus options"};
        opts_desc.add_options()
            ((name_ + ".id").c_str(),
                po::value<core::identity>(&opts.bus_registry_id)->default_value("bus.registry"_wire_id),
                "Identity for bus registry object")
            ((name_ + ".adapter").c_str(),
                po::value<::std::string>(&opts.bus_adapter_category)->default_value("wire.bus"),
                "Category and replica name for bus adapter")
            ((name_ + ".endpoints").c_str(),
                po::value<core::endpoint_list>(&opts.bus_endpoints)->required(),
                "Endpoints for bus adapter")
            ((name_ + ".category").c_str(),
                po::value<::std::string>(&opts.bus_category)->default_value("bus"),
                "Category for bus objects")
            ((name_ + ".publisher.adapter").c_str(),
                po::value<::std::string>(&opts.publisher_adapter_category)->default_value("wire.bus.publisher"),
                "Category and replica name for publisher objects adapter")
            ((name_ + ".publisher.endpoints").c_str(),
                po::value<core::endpoint_list>(&opts.publisher_endpoints),
                "Endpoints for publiser objects adapter")
            ((name_ + ".bus").c_str(),
                po::value<core::identity_seq>(&opts.predefined_buses),
                "List of predefined buses")
            ((name_ + ".allow-create").c_str(),
                po::bool_switch(&opts.allow_create)->default_value(true),
                "Allow creation of new bus objects")
            ((name_ + ".collocate-registry").c_str(),
                po::bool_switch(&opts.collocate_registry)->default_value(false),
                "Locator registry is collocated in same process")
            ((name_ + ".print-proxy").c_str(),
                po::bool_switch(&opts.print_proxy),
                "Print bus registry proxy to ::std::cout")
        ;

        po::variables_map vm;
        cnctr->configure_options(opts_desc, vm);

        if (opts.publisher_endpoints.empty())
            opts.publisher_endpoints.push_back(core::endpoint::tcp("0.0.0.0", 0));

        bus_adapter_ = cnctr->create_adapter(
                core::identity::random(opts.bus_adapter_category),
                opts.bus_endpoints);
        bus_adapter_->activate();

        publisher_adapter_ = cnctr->create_adapter(
                core::identity::random(opts.publisher_adapter_category),
                opts.publisher_endpoints);
        publisher_adapter_->activate();

        auto pub_ptr = ::std::make_shared<publisher>();
        publisher_adapter_->add_default_servant(pub_ptr);
        auto bus_ptr = ::std::make_shared<bus_impl>(publisher_adapter_, pub_ptr);
        bus_adapter_->add_default_servant(bus_ptr);
        auto reg_ptr = ::std::make_shared<registry_impl>(bus_adapter_, pub_ptr);
        auto prx = bus_adapter_->add_object(opts.bus_registry_id, reg_ptr);
        registry_ = core::unchecked_cast< bus::bus_registry_proxy >(prx);

        for (auto const& id : opts.predefined_buses) {
            pub_ptr->add_bus(id);
        }
        if (opts.print_proxy) {
            ::std::cout << *registry_ << ::std::endl;
        }
    }

    void
    stop()
    {
        publisher_adapter_->deactivate();
        bus_adapter_->deactivate();
    }
};

bus_service::bus_service(::std::string const& name)
    : pimpl_{util::make_unique<impl>(name)}
{
}

bus_service::~bus_service() = default;

void
bus_service::start(core::connector_ptr cnctr)
{
    pimpl_->start(cnctr);
}

void
bus_service::stop()
{
    pimpl_->stop();
}

bus::bus_registry_prx
bus_service::registry() const
{
    return pimpl_->registry_;
}

}  /* namespace svc */
}  /* namespace wire */
