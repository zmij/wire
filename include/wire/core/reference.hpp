/*
 * reference.hpp
 *
 *  Created on: Apr 11, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_REFERENCE_HPP_
#define WIRE_CORE_REFERENCE_HPP_

#include <wire/core/identity.hpp>
#include <wire/core/endpoint.hpp>

#include <wire/core/connection_fwd.hpp>
#include <wire/core/connector_fwd.hpp>
#include <wire/core/reference_fwd.hpp>

namespace wire {
namespace core {

struct reference_data {
    identity                object_id;
    ::std::string           facet;
    optional_identity       adapter;
    endpoint_list           endpoints;
};

::std::ostream&
operator << (::std::ostream& os, reference_data const& val);
::std::istream&
operator >> (::std::istream& is, reference_data& val);

/**
 * Class for a reference.
 */
class reference {
public:
    reference(connector_ptr cn, reference_data const& ref)
        : connector_{cn}, ref_{ref} {}
    virtual ~reference() = default;

    identity const&
    object_id() const
    { return ref_.object_id; }

    ::std::string const&
    facet() const
    { return ref_.facet; }

    connector_ptr
    get_connector() const
    { return connector_.lock(); }

    virtual connection_ptr
    get_connection() const = 0;
private:
    connector_weak_ptr  connector_;
protected:
    reference_data      ref_;
};

using reference_ptr = ::std::shared_ptr<reference>;

/**
 * Fixed reference
 */
class fixed_reference : public reference {
public:
    fixed_reference(connector_ptr cn, reference_data const& ref);

    connection_ptr
    get_connection() const override;
private:
    connection_weak_ptr mutable             connection_;
    endpoint_list::const_iterator mutable   current_;
};

class routed_reference : public reference {
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_REFERENCE_HPP_ */
