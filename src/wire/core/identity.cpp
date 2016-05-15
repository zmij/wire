/*
 * identity.cpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/identity.hpp>
#include <wire/core/grammar/identity_parse.hpp>

#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

namespace wire {
namespace core {

namespace detail {
struct identity_empty_visitor : ::boost::static_visitor< bool > {
    bool
    operator()(::std::string const& s) const
    {
        return s.empty();
    }
    bool
    operator()(::boost::uuids::uuid const& u) const
    {
        return u.is_nil();
    }
};

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

bool
identity::empty() const
{
    return ::boost::apply_visitor(detail::identity_empty_visitor{}, id);
}

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

::std::istream&
operator >> (::std::istream& is, identity& val)
{
    ::std::istream::sentry s(is);
    if (s) {
        namespace qi = ::boost::spirit::qi;
        using istreambuf_iterator   = ::std::istreambuf_iterator<char>;
        using stream_iterator       = ::boost::spirit::multi_pass< istreambuf_iterator >;
        using grammar               = grammar::parse::identity_grammar<stream_iterator>;

        stream_iterator in { istreambuf_iterator{ is } };
        stream_iterator eos { istreambuf_iterator{} };
        if (!qi::parse(in, eos, grammar{}, val))
            is.setstate(::std::ios_base::badbit);
    }
    return is;
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
