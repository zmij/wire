/*
 * replicated_object_gate.hpp
 *
 *  Created on: Dec 25, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_REPLICATED_OBJECT_GATE_HPP_
#define WIRE_UTIL_REPLICATED_OBJECT_GATE_HPP_

#include <wire/core/connector.hpp>
#include <wire/core/connection.hpp>
#include <wire/core/locator.hpp>
#include <wire/core/proxy.hpp>

#include <wire/errors/not_found.hpp>

#include <wire/util/timed_cache.hpp>

namespace wire {
namespace util {

template < typename ProxyType >
class replicated_object_gate :
        public ::std::enable_shared_from_this<replicated_object_gate<ProxyType>> {
public:
    using proxy_type            = ProxyType;
    using prx                   = ::std::shared_ptr<proxy_type>;
    using gate_type             = replicated_object_gate<proxy_type>;

    using identity              = core::identity;
    using locator_prx           = core::locator_prx;

    using proxy_callback        = core::functional::callback< prx >;
    using exception_callback    = core::functional::exception_callback;
public:
    replicated_object_gate(identity const& ada_id, identity const& obj_id,
            locator_prx locator)
        : adapter_id_{ ada_id }, object_id_{ obj_id }, locator_{ locator } {}

    void
    random_object( proxy_callback __result, exception_callback __exception )
    {
        using core::reference_data;
        auto connector = locator_->wire_get_connector();

        endpoints_ptr   eps = endpoints_;
        if (!eps) {
            try {
                auto ada    = locator_->find_adapter(adapter_id_);
                endpoints_  = ada->wire_get_reference()->data().endpoints;
                eps         = endpoints_;
            } catch (core::adapter_not_found const&) {
            }
        }

        if (!eps || eps->empty()) {
            core::functional::report_exception(
                    __exception, core::object_not_found{ object_id_ });
            return;
        }
        // Copy the endpoint data now as it can go away while get_connector works
        auto ep_data = *eps;

        auto _this = this->shared_from_this();
        connector->resolve_connection_async(
            reference_data{ object_id_, {}, {}, ::std::move(ep_data) },
            [_this, __result](core::connection_ptr conn)
            {
                auto obj = _this->locator_->wire_get_connector()->make_proxy(
                        reference_data{ _this->object_id_, {}, {},
                            { conn->remote_endpoint() } }
                    );
                __result(core::unchecked_cast<proxy_type>(obj));
            }, __exception);
    }
private:
    using endpoint_list             = core::endpoint_list;
    using endpoints_ptr             = ::std::shared_ptr<endpoint_list>;
    using endpoint_cache            = timed_cache< endpoint_list, 10 >;

    identity const  adapter_id_;
    identity const  object_id_;
    locator_prx     locator_;

    endpoint_cache  endpoints_;
};

} /* namespace util */
} /* namespace wire */

#endif /* WIRE_UTIL_REPLICATED_OBJECT_GATE_HPP_ */
