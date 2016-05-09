/*
 * user_exception.cpp
 *
 *  Created on: May 9, 2016
 *      Author: zmij
 */

#include <wire/errors/user_exception.hpp>
#include <mutex>

namespace wire {
namespace errors {

user_exception_factory&
user_exception_factory::instance()
{
    static user_exception_factory _factory;
    return _factory;
}

void
user_exception_factory::add_factory(::std::string const& type_id,
        ::std::uint64_t type_id_hash, factory_function func)
{
    static ::std::mutex _mtx;
    ::std::lock_guard<::std::mutex> lock{_mtx};
    if (id_to_factory_.count(type_id)) {
        throw logic_error{"A factory for exception ", type_id, " is already registered"};
    }
    if (hash_to_factory_.count(type_id_hash)) {
        throw logic_error{"A factory for exception ", type_id,
            " with hash ", type_id_hash, " is already registered"};
    }
    id_to_factory_.emplace(type_id, func);
    hash_to_factory_.emplace(type_id_hash, func);
}

user_exception_factory::factory_function
user_exception_factory::get_factory(::std::string const& type_id) const
{
    auto f = id_to_factory_.find(type_id);
    if (f == id_to_factory_.end()) {
        throw unmarshal_error{"Type id for exception ", type_id, " not found"};
    }
    return f->second;
}

user_exception_factory::factory_function
user_exception_factory::get_factory(::std::uint64_t type_id_hash) const
{
    auto f = hash_to_factory_.find(type_id_hash);
    if (f == hash_to_factory_.end()) {
        throw unmarshal_error{"Type id hash for exception ", type_id_hash, " not found"};
    }
    return f->second;
}

user_exception_factory::factory_function
user_exception_factory::get_factory(encoding::segment_header::type_id_type const& id) const
{
    switch (id.which()) {
        case 0:
            return get_factory(::boost::get< ::std::string >( id ));
        case 1:
            return get_factory(::boost::get< ::std::uint64_t >( id ));
        default:
            throw unmarshal_error{ "Unexpected type in segment header" };
    }
}

bool
user_exception_factory::has_factory(::std::string const& type_id) const
{
    return id_to_factory_.count(type_id) > 0;
}

bool
user_exception_factory::has_factory(::std::uint64_t type_id_hash) const
{
    return hash_to_factory_.count(type_id_hash) > 0;
}

bool
user_exception_factory::has_factory(encoding::segment_header::type_id_type const& id) const
{
    switch (id.which()) {
        case 0:
            return has_factory(::boost::get< ::std::string >( id ));
        case 1:
            return has_factory(::boost::get< ::std::uint64_t >( id ));
        default:
            throw unmarshal_error{ "Unexpected type in segment header" };
    }
}

}  /* namespace errors */
}  /* namespace wire */
