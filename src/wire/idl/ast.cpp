/*
 * ast.cpp
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/idl/ast.hpp>
#include <wire/idl/qname.hpp>
#include <sstream>

#include <iostream>

namespace wire {
namespace idl {

//----------------------------------------------------------------------------
//    builtin class implementation
//----------------------------------------------------------------------------
struct builtin_type : public type {
    builtin_type(::std::string name)
        : type(name)
    {
    }
};

namespace {

#define MAKE_TYPE_PAIR(t, x) {#x, ::std::make_shared<t>(#x)}

type_list const&
builtin_types()
{
    static type_list builtins_{
        MAKE_TYPE_PAIR(builtin_type, bool),
        MAKE_TYPE_PAIR(builtin_type, char),
        MAKE_TYPE_PAIR(builtin_type, byte),
        MAKE_TYPE_PAIR(builtin_type, int32),
        MAKE_TYPE_PAIR(builtin_type, int64),
        MAKE_TYPE_PAIR(builtin_type, octet),
        MAKE_TYPE_PAIR(builtin_type, uint32),
        MAKE_TYPE_PAIR(builtin_type, uint64),
        MAKE_TYPE_PAIR(builtin_type, float),
        MAKE_TYPE_PAIR(builtin_type, double),
        MAKE_TYPE_PAIR(builtin_type, string),
        MAKE_TYPE_PAIR(builtin_type, uuid),

        // FIXME Replace with parametrized type implementation
        MAKE_TYPE_PAIR(builtin_type, variant),
        MAKE_TYPE_PAIR(builtin_type, sequence),
        MAKE_TYPE_PAIR(builtin_type, array),
        MAKE_TYPE_PAIR(builtin_type, dictionary),
        MAKE_TYPE_PAIR(builtin_type, optional),
    };
    return builtins_;
}

}  /* namespace  */

//----------------------------------------------------------------------------
//    entity class implementation
//----------------------------------------------------------------------------
entity::entity()
    : scope_{}, name_{}
{
}

entity::entity(::std::string const& name)
    : name_{name}
{
    if (name.empty()) {
        throw std::runtime_error("Name is empty");
    }
}

entity::entity(scope_ptr sc, ::std::string const& name)
    : scope_{sc}, name_{name}
{
    if (!sc) {
        throw std::runtime_error("Scope pointer is empty");
    }
    if (name.empty()) {
        throw std::runtime_error("Name is empty");
    }
}


qname
entity::get_qualified_name() const
{
    scope_ptr sc = parent();
    qname qn;
    if (sc) {
        qn = sc->get_qualified_name();
    } else {
        qn.fully = true;
    }
    qn.components.push_back(name_);
    return qn;
}

//----------------------------------------------------------------------------
//    type class implementation
//----------------------------------------------------------------------------
bool
type::is_biult_in(qname const& qn)
{
    type_list const& bt = builtin_types();
    auto f = bt.find(qn.name());
    return f != bt.end();
}

//----------------------------------------------------------------------------
//    constant class implementation
//----------------------------------------------------------------------------
constant::constant(scope_ptr sc, ::std::string const& type,
            ::std::string const& name, ::std::string const& literal)
    : entity{sc, name}, literal_{literal}
{
}

//----------------------------------------------------------------------------
//    scope class implementation
//----------------------------------------------------------------------------
entity_ptr
scope::find_name(qname const& name) const
{
    if (name.components.size() == 1) {
        // check for built-in types
    }
    return find_name(
        qname_search{
            name.fully,
            name.components.begin(),
            name.components.end() });
}

entity_ptr
scope::find_name(qname_search const& search) const
{
    if (search.begin == search.end) {
        throw ::std::runtime_error{"Empty entity name"};
    }

    if (search.fully) {
        return namespace_::global()->find_name(search);
    }
    // search local
    entity_ptr f;
    // search types
    auto t = types_.find(*search.begin);
    if (t != types_.end()) {
        auto b = search.begin + 1;
        if (b != search.end) {
            scope_ptr sc = ::std::dynamic_pointer_cast<scope>(t->second);
            if (sc) {
                f = sc->find_name( qname_search{ false, b, search.end } );
            } else {
                throw ::std::runtime_error("Type cannot have nested names");
            }
        } else {
            f = t->second;
        }
    }
    if (!f) {
        // search constants
    }
    return f;
}
//----------------------------------------------------------------------------
//    name_space class implementation
//----------------------------------------------------------------------------

class global_namespace : public namespace_ {
public:
    global_namespace()
        : namespace_{}
    {
    }
};

namespace_ptr
namespace_::global()
{
    static namespace_ptr gns_(std::make_shared<global_namespace>());
    return gns_;
}

void
namespace_::clear_global()
{
    global()->types_.clear();
    global()->constants_.clear();
    global()->nested_.clear();
}

namespace_ptr
namespace_::add_namespace(qname const& qn)
{
    if (qn.fully && parent()) {
        throw ::std::runtime_error("Cannot add a globally scoped namespace");
    }
    return add_namespace(qname_search{false, qn.begin(), qn.end()});
}

namespace_ptr
namespace_::add_namespace(qname_search const& s)
{
    auto f = nested_.find(*s.begin);
    if (f == nested_.end()) {
        auto ns = ::std::make_shared< namespace_ >(
                shared_this< namespace_ >(), *s.begin );
        f = nested_.emplace( *s.begin, ns ).first;
    }
    auto n = s.begin + 1;
    if (n != s.end) {
        return f->second->add_namespace(qname_search{false, n, s.end});
    }
    return f->second;
}

entity_ptr
namespace_::find_name(qname_search const& s) const
{
    if (s.fully && !is_global()) {
        return global()->find_name(s);
    }
    std::cerr << "Lookup name " << *s.begin
        << " in " << name() << "\n";
    auto f = nested_.find(*s.begin);
    if (f != nested_.end()) {
        auto n = s.next(false);
        if (!n.empty()) {
            return f->second->find_name(n);
        }
        return f->second;
    }
    throw ::std::runtime_error("Name not found");
}

bool
namespace_::is_global() const
{
    return name().empty();
}

}  // namespace ast
}  // namespace wire



