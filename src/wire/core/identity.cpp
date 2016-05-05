/*
 * identity.cpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/identity.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

namespace wire {
namespace core {

namespace detail {
struct identity_hash_visitor : ::boost::static_visitor< ::std::size_t > {
    ::std::size_t
    operator()(::std::string const& s) const
    {
        static ::std::hash<::std::string> str_hash;
        return str_hash(s);
    }
    ::std::size_t
    operator()(::boost::uuids::uuid const& u) const
    {
        return hash_value(u);
    }
};

struct identity_out_visitor : ::boost::static_visitor<> {
    ::std::ostream& os;

    identity_out_visitor(::std::ostream& os_) : os{os_} {}
    void
    operator()(::std::string const& s) const
    {
        os << s;
    }

    void
    operator()(::boost::uuids::uuid const& u) const
    {
        os << u;
    }
};

}  // namespace detail

::std::ostream&
operator << (::std::ostream& os, identity const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        if (!val.category.empty()) {
            os << val.category << "/";
        }
        detail::identity_out_visitor out{os};
        ::boost::apply_visitor(out, val.id);
    }
    return os;
}


::std::size_t
hash(identity const& v)
{
    detail::identity_hash_visitor hasher;
    ::std::size_t id_hash = ::boost::apply_visitor(hasher, v.id) ^ (v.id.which() << 1);
    return hasher(v.category) ^ (id_hash << 1);
}

}  // namespace core
}  // namespace wire
