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
    identity                            object_id;
    ::boost::optional<::std::string>    facet;
    optional_identity                   adapter;
    endpoint_list                       endpoints;
};

::std::ostream&
operator << (::std::ostream& os, reference_data const& val);

/**
 * Class for a reference.
 */
class reference {
public:
    virtual ~reference() = default;

    /**
     * Parse a stringified reference.
     * Reference format:
     *  reference = identity ('[' facet ']')? ('@' adapter)|(endpoints)
     * @param
     * @return
     */
    static reference_ptr
    parse_string(::std::string const&);
private:
    connector_weak_ptr  connector_;

    identity            object_id_;
    ::std::string       facet_;
};

using reference_ptr = ::std::shared_ptr<reference>;

/**
 * Fixed reference
 */
class fixed_reference : public reference {
private:
    connection_weak_ptr connection_;
};

class routed_reference : public reference {
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_REFERENCE_HPP_ */
