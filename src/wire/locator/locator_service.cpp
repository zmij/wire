/*
 * locator_service.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#include <wire/locator/locator_service.hpp>
#include <wire/locator/locator_options.hpp>
#include <wire/locator/locator_impl.hpp>

#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>

#include <wire/util/make_unique.hpp>

#include <boost/program_options.hpp>

namespace wire {
namespace svc {

struct locator_service::impl {
    ::std::string       name_;
    core::adapter_ptr   locator_adapter_;
    core::adapter_ptr   registry_adapter_;

    impl(::std::string const& name)
        : name_{name} {}

    void
    start(core::connector_ptr cnctr)
    {
        namespace po = ::boost::program_options;
        locator_options opts;
        po::options_description opts_desc{"Locator options"};
        opts_desc.add_options()
            ((name_ + ".id").c_str(),
                po::value<core::identity>(&opts.locator_id)->default_value("locator.locator"_wire_id),
                "Identity for locator object")
            ((name_ + ".adapter").c_str(),
                po::value<core::identity>(&opts.locator_adapter)->default_value("locator"_wire_id),
                "Identity for locator object adapter")
            ((name_ + ".endpoints").c_str(),
                po::value<core::endpoint_list>(&opts.locator_endpoints)->required(),
                "Endpoints for locator object adapter")
            ((name_ + ".registry.id").c_str(),
                po::value<core::identity>(&opts.registry_id)->default_value("locator.registry"_wire_id),
                "Identity for locator registry object")
            ((name_ + ".registry.adapter").c_str(),
                po::value<core::identity>(&opts.registry_adapter)->default_value("locator"_wire_id),
                "Identity for locator registry object adapter")
            ((name_ + ".registry.endpoints").c_str(),
                po::value<core::endpoint_list>(&opts.registry_endpoints),
                "Endpoints for locator registry object adapter")
        ;

        po::variables_map vm;
        cnctr->configure_options(opts_desc, vm);
        locator_adapter_ = cnctr->create_adapter(opts.locator_adapter, opts.locator_endpoints);
        locator_adapter_->activate(true);
        if (opts.registry_adapter != opts.locator_adapter) {
            if (opts.registry_endpoints.empty()) {
                opts.registry_endpoints.push_back( core::endpoint::tcp("0.0.0.0", 0) );
            }
            // create a separate adapter for registry
            registry_adapter_ = cnctr->create_adapter(opts.registry_adapter, opts.registry_endpoints);
            registry_adapter_->activate(true);
        } else {
            // use the same adapter
            registry_adapter_ = locator_adapter_;
        }

        auto prx = registry_adapter_->create_direct_proxy(opts.registry_id);
        auto reg_prx = core::unchecked_cast< core::locator_registry_proxy >(prx);
        auto loc = ::std::make_shared<locator>(reg_prx);
        auto reg = ::std::make_shared<locator_registry>(loc);

        prx = locator_adapter_->add_object(opts.locator_id, loc);
        registry_adapter_->add_object(opts.registry_id, reg);
        auto loc_prx = core::unchecked_cast<core::locator_proxy>(prx);

        cnctr->set_locator(loc_prx);
        locator_adapter_->register_adapter();
        if (registry_adapter_ != locator_adapter_)
            registry_adapter_->register_adapter();
    }

    void
    stop()
    {
        if (registry_adapter_ != locator_adapter_)
            registry_adapter_->deactivate();
        locator_adapter_->deactivate();
    }
};

locator_service::locator_service(::std::string const& name)
    : pimpl_{ util::make_unique< impl >(name) }
{
}

locator_service::~locator_service() = default;

void
locator_service::start(core::connector_ptr cnctr)
{
    pimpl_->start(cnctr);
}

void
locator_service::stop()
{
    pimpl_->stop();
}

} /* namespace svc */
} /* namespace wire */
