/*
 * polymorphic_io.inl
 *
 *  Created on: May 10, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_POLYMORPHIC_IO_INL_
#define WIRE_ENCODING_DETAIL_POLYMORPHIC_IO_INL_

#include <wire/encoding/detail/polymorphic_io.hpp>
#include <wire/errors/exceptions.hpp>
#include <mutex>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
object_factory<T>&
object_factory<T>::instance()
{
    static object_factory _factory;
    return _factory;
}

template < typename T >
::std::string const&
object_factory<T>::root_type_id()
{
    return root_type::wire_static_type_id();
}

template < typename T >
void
object_factory<T>::add_factory(::std::string const& type_id, hash_value_type hash,
        factory_function func)
{
    static ::std::mutex _mtx;
    ::std::lock_guard<::std::mutex> lock{_mtx};

    if (id_to_factory_.count(type_id))
        throw errors::logic_error{ "A factory for exception ", type_id, " is already registered" };

    if (hash_to_factory_.count(hash)) {
        throw errors::logic_error{"A factory for exception ", type_id,
            " with hash ", hash, " is already registered"};
    }
    id_to_factory_.emplace(type_id, func);
    hash_to_factory_.emplace(hash, func);
    names_.emplace(hash, type_id);
}

template < typename T >
typename object_factory<T>::factory_function
object_factory<T>::get_factory(::std::string const& type_id) const
{
    auto f = id_to_factory_.find(type_id);
    if (f == id_to_factory_.end()) {
        throw errors::unmarshal_error{"Type id for exception ", type_id, " not found"};
    }
    return f->second;
}

template < typename T >
typename object_factory<T>::factory_function
object_factory<T>::get_factory(hash_value_type type_id_hash) const
{
    auto f = hash_to_factory_.find(type_id_hash);
    if (f == hash_to_factory_.end()) {
        throw errors::unmarshal_error{"Type id hash for exception ", type_id_hash, " not found"};
    }
    return f->second;
}

template < typename T >
typename object_factory<T>::factory_function
object_factory<T>::get_factory(encoding::segment_header::type_id_type const& id) const
{
    switch (id.which()) {
        case 0:
            return get_factory(::boost::get< ::std::string >( id ));
        case 1:
            return get_factory(::boost::get< hash_value_type >( id ));
        default:
            throw errors::unmarshal_error{ "Unexpected type in segment header" };
    }
}

template < typename T >
bool
object_factory<T>::has_factory(::std::string const& type_id) const
{
    return id_to_factory_.count(type_id) > 0;
}

template < typename T >
bool
object_factory<T>::has_factory(hash_value_type type_id_hash) const
{
    return hash_to_factory_.count(type_id_hash) > 0;
}

template < typename T >
bool
object_factory<T>::has_factory(segment_header::type_id_type const& id) const
{
    switch (id.which()) {
        case 0:
            return has_factory(::boost::get< ::std::string >( id ));
        case 1:
            return has_factory(::boost::get< hash_value_type >( id ));
        default:
            throw errors::unmarshal_error{ "Unexpected type in segment header" };
    }
}

template < typename T >
::std::string const&
object_factory<T>::factory_name(segment_header::type_id_type const& id) const
{
    switch (id.which()) {
        case 0:
            return ::boost::get< ::std::string >(id);
        case 1:
            return names_.at( ::boost::get< hash_value_type >(id) );
        default:
            throw errors::unmarshal_error{ "Unexpected type in segment header" };
    }
}

template < typename T >
template < typename U >
::std::shared_ptr< U >
object_factory<T>::read(input_iterator& begin, input_iterator end) const
{
    static_assert(::std::is_base_of<root_type, U>::value,
            "Invalid object factory usage");
    auto encaps = begin.incoming_encapsulation();
    segment_header seg_head;
    encaps.read_segment_header(begin, end, seg_head);

    while (!has_factory(seg_head.type_id)) {
        // skip segment
        begin += seg_head.size;
        if (begin == end) {
            throw errors::unmarshal_error{ "Failed to read object ", U::wire_static_type_id() };
        }
        encaps.read_segment_header(begin, end, seg_head);
    }

    auto func = get_factory(seg_head.type_id);
    auto obj = func();
    if (!obj) {
        throw errors::unmarshal_error{ "Factory ",
            factory_name(seg_head.type_id), " returned an empty object." };
    }
    auto derived = ::std::dynamic_pointer_cast< U >(obj);
    if (!obj) {
        throw errors::unmarshal_error{ "Object created by factory ",
            factory_name(seg_head.type_id), " cannot be cast to ", U::wire_static_type_id() };
    }
    derived->__wire_read(begin, end, false);
    return derived;
}

}  /* namespace detail */
}  /* namespace encoding */
}  /* namespace wire */

#endif /* WIRE_ENCODING_DETAIL_POLYMORPHIC_IO_INL_ */
