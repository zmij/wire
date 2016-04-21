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
            bool scope = false;
            for (auto const& c : val.components) {
                if (scope)
                    os << "::";
                os << c;
                scope = true;
            }
            if (!val.parameters.empty()) {
                os << "<";
                bool comma = false;
                for (auto const& p : val.parameters) {
                    if (comma)
                        os << ", ";
                    os << p;
                    comma = true;
                }
                os << ">";
            }
        }
    }
    return os;
}

}  /* namespace idl */
}  /* namespace wire */
