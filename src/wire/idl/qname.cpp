/*
 * qname.cpp
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/idl/qname.hpp>
#include <wire/idl/qname_grammar.hpp>
#include <wire/idl/ast.hpp>

namespace wire {
namespace idl {

qname::qname(char const* name)
    : qname(::std::string{name})
{
}

qname::qname(::std::string const& name)
{
    namespace qi = ::boost::spirit::qi;
    using iterator = ::std::string::const_iterator;
    using grammar = parse::qualified_name_grammar< iterator >;

    auto b = name.begin();
    auto e = name.end();

    if (!qi::parse(b, e, grammar{}, *this) || b != e)
        // FIXME More specific exception
        throw ::std::runtime_error( "Invalid name" );
}

bool
qname::operator ==(qname const& rhs) const
{
    if (fully != rhs.fully)
        return false;
    auto lhss = search();
    auto rhss = rhs.search();

    auto l = lhss.begin;
    auto r = rhss.begin;
    for (; l != lhss.end && r != rhss.end && *l == *r; ++l, ++r);

    return l == lhss.end && r == rhss.end;
}

qname&
qname::operator +=(qname const& rhs)
{
    components.insert(components.end(), rhs.components.begin(), rhs.components.end());
    return *this;
}

qname
qname::parse(::std::string const& name)
{
    return qname{ name };
}

::std::ostream&
operator << (::std::ostream& os, qname const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        if (ast::type::is_built_in(val)) {
            os << val.name();
        } else {
            if (val.fully)
                os << "::";
            for (auto c = val.components.begin(); c != val.components.end(); ++c) {
                if (c != val.components.begin())
                    os << "::";
                os << *c;
            }
        }
    }
    return os;
}

::std::ostream&
operator << (::std::ostream& os, qname_search const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        if (val.fully)
            os << "::";
        for (auto c = val.begin; c != val.end; ++c) {
            if (c != val.begin)
                os << "::";
            os << *c;
        }
    }
    return os;
}


}  /* namespace idl */
}  /* namespace wire */
