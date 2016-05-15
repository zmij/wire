/*
 * object.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_OBJECT_HPP_
#define WIRE_CORE_OBJECT_HPP_

#include <wire/types.hpp>
#include <wire/core/object_fwd.hpp>
#include <wire/core/current.hpp>
#include <wire/core/detail/dispatch_request_fwd.hpp>

namespace wire {
namespace core {

class object {
public:
    using type_list = ::std::vector< ::std::string >;
    using dispatch_seen_list = ::std::set< hash_value_type >;
public:
    virtual
    ~object() = default;

    virtual bool
    wire_is_a(std::string const&, current const& = no_current) const;
    void
    __wire_is_a(detail::dispatch_request const&, current const&);

    virtual void
    wire_ping(current const& = no_current) const;
    void
    __wire_ping(detail::dispatch_request const&, current const&);

    virtual ::std::string const&
    wire_type(current const& = no_current) const;
    void
    __wire_type(detail::dispatch_request const&, current const&);

    virtual type_list const&
    wire_types(current const& = no_current) const;
    void
    __wire_types(detail::dispatch_request const&, current const&);

    void
    __dispatch(detail::dispatch_request const&, current const&);

    static ::std::string const&
    wire_static_type_id();
    static ::std::int64_t
    wire_static_type_id_hash();
protected:
    virtual bool
    __wire_dispatch(detail::dispatch_request const&, current const&,
            dispatch_seen_list&, bool throw_not_found);
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_OBJECT_HPP_ */
