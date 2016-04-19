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
#include <cassert>

namespace wire {
namespace idl {
namespace ast {


//----------------------------------------------------------------------------
//    builtin class implementation
//----------------------------------------------------------------------------
/**
 * Built-in type class implementation.
 * Implementation is hidden to prohibit unintentional adding of a built-in type
 */
struct builtin_type : public type {
    builtin_type(::std::string name)
        : type(name)
    {
    }

    scope_ptr
    owner() const override
    {
        return namespace_::global();
    }
};

namespace {

#define MAKE_TYPE_PAIR(t, x) {#x, ::std::make_shared<t>(#x)}

type_list const&
builtin_types()
{
    static type_list builtins_{
        MAKE_TYPE_PAIR(builtin_type, void),
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

type_ptr
find_builtin(::std::string const& name)
{
    type_list const& bt = builtin_types();
    auto f = bt.find(name);
    if (f != bt.end()) {
        return f->second;
    }
    return type_ptr{};
}

}  /* namespace  */

//----------------------------------------------------------------------------
//    entity class implementation
//----------------------------------------------------------------------------
entity::entity(::std::string const& name)
    : name_{name}
{
    if (name.empty()) {
        throw std::runtime_error("Name is empty");
    }
}

entity::entity(scope_ptr sc, ::std::string const& name)
    : owner_{sc}, name_{name}
{
    if (!sc) {
        throw std::runtime_error("Scope pointer is empty");
    }
    if (name.empty()) {
        throw std::runtime_error("Name is empty");
    }
}

::std::string const&
entity::name() const
{
    static ::std::string global_ns = "__GLOBAL__";
    return name_.empty() ? global_ns : name_;
}

qname
entity::get_qualified_name() const
{
    scope_ptr sc = owner();
    qname qn;
    if (sc) {
        qn = sc->get_qualified_name();
    } else {
        qn.fully = true;
    }
    if (!name_.empty())
        qn.components.push_back(name_);
    return qn;
}

//----------------------------------------------------------------------------
//    type class implementation
//----------------------------------------------------------------------------
bool
type::is_built_in(qname const& qn)
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
::std::pair<scope_ptr, scope_ptr>
scope::find_scope(qname const& qn) const
{
    if (qn.empty())
        return ::std::make_pair(namespace_::global(), scope_ptr{});
    return find_scope(qn.search());
}

::std::pair<scope_ptr, scope_ptr>
scope::find_scope(qname_search const& search) const
{
    scope_ptr sc = local_scope_search(search);
    if (sc && !search.empty()) {
        auto next = search.next();
        if (!next.empty()) {
            sc = sc->find_scope(next).first;
        }
    }

    if (!sc) {
        // Fallback to parent
        if (auto own = owner()) {
            return own->find_scope(search);
        }
    }

    return ::std::make_pair(sc, const_cast< scope* >(this)->shared_this<scope>());
}

scope_ptr
scope::find_scope_of(qname const& qn) const
{
    return find_scope_of(qn.search());
}

scope_ptr
scope::find_scope_of(qname_search const& search) const
{
    if (search.empty()) {
        return scope_ptr{};
    }
    if (type::is_built_in(search.back())) {
        return namespace_::global();
    }
    auto sc = find_scope(search.scope());
    if (sc.first && !sc.first->local_entity_search(search)) {
        sc.first.reset();
    }
    if (!sc.first) {
        scope_ptr own = owner();
        if (own) {
            return own->find_scope_of(search);
        }
    }
    return sc.first;
}

entity_ptr
scope::find_entity(qname const& qn) const
{
    return find_entity(qn.search());
}

entity_ptr
scope::find_entity(qname_search const& search) const
{
    if (search.empty())
        return entity_ptr{};
    type_ptr t = find_builtin(search.back());
    if (t)
        return t;
    scope_ptr sc = find_scope_of(search);
    if (sc) {
        return sc->local_entity_search(search);
    }
    return entity_ptr{};
}

type_ptr
scope::find_type(qname const& qn) const
{
    if (qn.fully) {
        return namespace_::global()->find_type(qn.search());
    }
    return find_type(qn.search(false));
}

type_ptr
scope::find_type(qname_search const& search) const
{
    if (search.empty()) {
        return type_ptr{};
    }
    type_ptr t = find_builtin(search.back());
    if (t)
        return t;
    if (search.fully && owner().get())
        return namespace_::global()->find_type(search);

    auto sc = find_scope(search.scope());
    if (sc.first) {
        t = sc.first->local_type_search(search);
        if (t)
            return t;
    }
    if (sc.first && sc.second->owner()) { // Proceed to parent scope only if not searching from global namespace
        sc.first = sc.first->owner();
        if (sc.first) {
            return sc.first->find_type(search);
        }
    }

    return type_ptr{};
}

type_ptr
scope::local_type_search(qname_search const& search) const
{
    if (search.empty())
        return type_ptr{};

    auto ft = types_.find(search.back());
    if (ft != types_.end()) {
        return ft->second;
    }
    return type_ptr{};
}

scope_ptr
scope::local_scope_search(qname_search const& search) const
{
    if (search.empty()) {
        return const_cast< scope* >(this)->shared_this<scope>();
    }
    auto ft = types_.find(search.front());
    if (ft != types_.end()) {
        return dynamic_type_cast<scope>(ft->second);
    }
    return scope_ptr{};
}

entity_ptr
scope::local_entity_search(qname_search const& search) const
{
    if (!search.empty()) {
        auto ft = types_.find(search.back());
        if (ft != types_.end()) {
            return ft->second;
        }
        // TODO Search for constants
    }
    return entity_ptr{};
}

//----------------------------------------------------------------------------
//    namespace_ class implementation
//----------------------------------------------------------------------------

class global_namespace : public namespace_ {
public:
    global_namespace()
        : namespace_()
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
    if (qn.fully && owner()) {
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

scope_ptr
namespace_::local_scope_search(qname_search const& search) const
{
    if (search.fully && !is_global()) {
        return global()->local_scope_search(search);
    }
    if (search.empty()) {
        return const_cast<namespace_*>(this)->shared_this< namespace_ >();
    }
    scope_ptr sc = scope::local_scope_search(search);
    if (!sc) {
        auto fn = nested_.find(search.front());
        if (fn != nested_.end()) {
            return fn->second;
        }
    }
    return sc;
}

entity_ptr
namespace_::local_entity_search(qname_search const& search) const
{
    if (search.empty()) {
        return entity_ptr{};
    }
    entity_ptr en = scope::local_entity_search(search);
    if (!en) {
        auto fn = nested_.find(search.back());
        if (fn != nested_.end()) {
            return fn->second;
        }
    }
    return en;
}

namespace_ptr
namespace_::find_namespace(qname const& qn) const
{
    return find_namespace(qn.search());
}

namespace_ptr
namespace_::find_namespace(qname_search const& search) const
{
    if (search.fully && !is_global()) {
        return global()->find_namespace(search);
    }
    auto f = nested_.find(search.front());
    if (f != nested_.end()) {
        auto n = search.next();
        if (!n.empty()) {
            return f->second->find_namespace(n);
        }
        return f->second;
    }
    if (auto own = owner()) {
        return dynamic_entity_cast< namespace_ >(own)->find_namespace(search);
    }
    return namespace_ptr{};
}

bool
namespace_::is_global() const
{
    return !owner();
}

//----------------------------------------------------------------------------
//    interface class implementation
//----------------------------------------------------------------------------
entity_ptr
interface::local_entity_search(qname_search const& search) const
{
    entity_ptr ent = scope::local_entity_search(search);
    if (!ent) {
        // search ancestors
        for (auto const& anc : ancestors_) {
            ent = anc.second->local_entity_search(search);
            if (ent)
                break;
        }
    }
    return ent;
}

type_ptr
interface::local_type_search(qname_search const& search) const
{
    type_ptr t = scope::local_type_search(search);
    if (!t) {
        for (auto const& anc : ancestors_) {
            t = anc.second->local_type_search(search);
            if (t)
                break;
        }
    }
    return t;
}

//----------------------------------------------------------------------------
//    class class implementation
//----------------------------------------------------------------------------
entity_ptr
class_::local_entity_search(qname_search const& search) const
{
    entity_ptr ent = scope::local_entity_search(search);
    if (!ent) {
        if (parent_) {
            ent = parent_->local_entity_search(search);
        }
        if (!ent) {
            // search ancestors
            for (auto const& anc : ancestors_) {
                ent = anc.second->local_entity_search(search);
                if (ent)
                    break;
            }
        }
    }
    return ent;
}

type_ptr
class_::local_type_search(qname_search const& search) const
{
    type_ptr t = scope::local_type_search(search);
    if (!t) {
        if (parent_) {
            t = parent_->local_type_search(search);
        }
        if (!t) {
            for (auto const& anc : ancestors_) {
                t = anc.second->local_type_search(search);
                if (t)
                    break;
            }
        }
    }
    return t;
}

}  // namespace ast
}  // namespace idl
}  // namespace wire



