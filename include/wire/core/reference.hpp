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

/**
 * Class for a reference.
 */
class reference {
public:
    virtual ~reference() = default;

    static reference_ptr
    parse_string(::std::string const&);
private:
    connector_weak_ptr  connector_;

    identity            object_id_;
    ::std::string       facet_;
};

using reference_ptr = ::std::shared_ptr<reference>;

class fixed_reference : public reference {
private:
    connection_weak_ptr connection_;
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_REFERENCE_HPP_ */
