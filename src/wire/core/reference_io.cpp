/*
 * reference_io.cpp
 *
 *  Created on: May 8, 2016
 *      Author: zmij
 */

#include <wire/core/reference.hpp>
#include <wire/core/grammar/reference_parse.hpp>

#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

namespace wire {
namespace core {

//----------------------------------------------------------------------------
//      Reference data implementation
//----------------------------------------------------------------------------

::std::ostream&
operator << (::std::ostream& os, reference_data const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        os << val.object_id;
        if (!val.facet.empty()) {
            os << "[" << val.facet << "]";
        }
        if (val.adapter.is_initialized()) {
            os << "@" << *val.adapter;
        }
        if (!val.endpoints.empty()) {
            os << " " << val.endpoints;
        }
    }
    return os;
}

::std::istream&
operator >> (::std::istream& is, reference_data& val)
{
    ::std::istream::sentry s(is);
    if (s) {
        namespace qi = ::boost::spirit::qi;
        using istreambuf_iterator   = ::std::istreambuf_iterator<char>;
        using stream_iterator       = ::boost::spirit::multi_pass< istreambuf_iterator >;
        using grammar               = grammar::parse::reference_grammar<stream_iterator>;

        stream_iterator in { istreambuf_iterator{ is } };
        stream_iterator eos { istreambuf_iterator{} };

        if (!qi::parse(in, eos, grammar{}, val))
            is.setstate(::std::ios_base::badbit);
    }
    return is;
}

}  // namespace core
}  // namespace wire
