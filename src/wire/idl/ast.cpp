/*
 * ast.cpp
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/idl/ast.hpp>
#include <wire/idl/qname.hpp>
#include <wire/idl/syntax_error.hpp>
#include <wire/idl/generator.hpp>
#include <sstream>

#include <iostream>
#include <cassert>

namespace wire {
namespace idl {
namespace ast {

//----------------------------------------------------------------------------
//    compilation unit class implementation
//----------------------------------------------------------------------------

entity_const_set
compilation_unit::dependecies() const
{
    entity_const_set deps;
    for (auto const& e : entities) {
        e->collect_dependencies(deps);
    }
    return deps;
}

entity_const_set
compilation_unit::external_dependencies() const
{
    entity_const_set deps;
    for (auto const& e : entities) {
        e->collect_dependencies(deps,
            [&](entity_const_ptr e) { return e->unit()->name != name; });
    }

    return deps;
}

compilation_unit_const_set
compilation_unit::dependent_units() const
{
    compilation_unit_const_set units;

    entity_const_set deps = external_dependencies();
    ::std::transform(deps.begin(), deps.end(),
        ::std::inserter(units, units.end()),
         [](entity_const_ptr e) { return e->unit(); });

    return units;
}

void
compilation_unit::generate(generator& gen) const
{
    // TODO Topological sort of local entities
    for (auto const& e : entities) {
        if (auto t = dynamic_entity_cast< type >(e)) {
            gen.generate_type_decl(t);
        } else if (auto c = dynamic_entity_cast< constant >(e)) {
            gen.generate_constant(c);
        } else {
            throw grammar_error(e->decl_position(), "Unexpected entity");
        }
    }
}

//----------------------------------------------------------------------------
struct builtin_unit : compilation_unit {
    builtin_unit() : compilation_unit("__BUILTINS__") {}

    bool
    is_builtin() const override
    { return true; }
};

//----------------------------------------------------------------------------
//    builtin class implementation
//----------------------------------------------------------------------------
/**
 * Built-in type class implementation.
 * Implementation is hidden to prohibit unintentional adding of a built-in type
 */
struct builtin_type : public type {
    builtin_type(::std::string name)
        : entity(name), type(name)
    {
    }

    scope_ptr
    owner() const override
    {
        return scope_ptr{};
    }
};

struct templated_type_impl : templated_type {
    templated_type_impl(::std::string const& name, template_param_types&& args)
        : entity(name), templated_type(name, ::std::move(args))
    {
    }
};

parametrized_type_ptr
templated_type::create_parametrized_type(scope_ptr sc, ::std::size_t pos) const
{
    return parametrized_type_ptr{ new parametrized_type(
        sc, pos, name(), shared_this< templated_type >()) };
}

void
parametrized_type::add_parameter(parameter const& param)
{
    if (current == tmpl_type->formal_parameters().end()) {
        ::std::ostringstream os;
        os << "Extra " << (param.which() ? "integral" : "type")
            << " parameter for " << name() << " template on position "
            << params_.size();
        throw grammar_error(os.str());
    }

    if (param.which() == current->kind) {
        params_.push_back(param);
        if (!current->unbound)
            ++current;
    } else {
        if (current->unbound) {
            ++current;
            add_parameter(param);
        } else {
            ::std::ostringstream os;
            os << "Unexpected " << (param.which() ? "integral" : "type")
                << " parameter for " << name() << " template on position "
                << params_.size();
            throw grammar_error(os.str());
        }
    }
}

void
parametrized_type::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    if (pred(tmpl_type))
        deps.insert(tmpl_type);
    for (auto const& p : params_) {
        if (p.which() == template_param_type::type) {
            type_ptr t = ::boost::get< type_ptr >(p);
            if (pred(t))
                deps.insert(t);
            t->collect_dependencies(deps, pred);
        }
    }
}

namespace {

#define MAKE_BUILTIN_WIRE_TYPE(t, x) ::std::make_shared<t>(#x)
#define MAKE_TEMPLATE_WIRE_TYPE(t, x, ...) ::std::make_shared<t>(#x, template_param_types __VA_ARGS__)

type_list const&
builtin_types()
{
    static type_list builtins_{
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, void),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, bool),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, char),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, byte),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, int32),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, int64),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, octet),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, uint32),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, uint64),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, float),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, double),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, string),
        MAKE_BUILTIN_WIRE_TYPE(builtin_type, uuid),

        // FIXME Replace with parametrized type implementation
        MAKE_TEMPLATE_WIRE_TYPE(templated_type_impl, variant,
                {{ template_param_type::type, true }}),
        MAKE_TEMPLATE_WIRE_TYPE(templated_type_impl, sequence,
                {{template_param_type::type, false}}),
        MAKE_TEMPLATE_WIRE_TYPE(templated_type_impl, array,
                {{template_param_type::type, false}, {template_param_type::integral, false}}),
        MAKE_TEMPLATE_WIRE_TYPE(templated_type_impl, dictionary,
                {{template_param_type::type, false}, {template_param_type::type, false}}),
        MAKE_TEMPLATE_WIRE_TYPE(templated_type_impl, optional,
                {{template_param_type::type, false}}),
    };
    return builtins_;
}

type_ptr
find_builtin(::std::string const& name)
{
    type_list const& bt = builtin_types();
    for (auto const& t : bt) {
        if (t->name() == name)
            return t;
    }
    return type_ptr{};
}

compilation_unit_ptr
builtins_unit()
{
    static compilation_unit_ptr builtins_ = ::std::make_shared< builtin_unit >();
    return builtins_;
}

}  /* namespace  */

//----------------------------------------------------------------------------
//    entity class implementation
//----------------------------------------------------------------------------
entity::entity(::std::string const& name)
    : name_{name}, decl_pos_{0}, compilation_unit_{ builtins_unit() }
{
    if (name.empty()) {
        throw std::runtime_error("Name is empty");
    }
}

entity::entity(scope_ptr sc, ::std::size_t pos, ::std::string const& name)
    : owner_{sc}, name_{name}, decl_pos_{pos}
{
    if (!sc) {
        throw std::runtime_error("Scope pointer is empty");
    }
    if (name.empty()) {
        throw std::runtime_error("Name is empty");
    }
    compilation_unit_ = get_global()->current_compilation_unit();
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

type_name
entity::get_type_name() const
{
    return type_name { get_qualified_name(), type_name::template_params{}, false };
}

global_namespace_ptr
entity::get_global() const
{
    scope_ptr sc = owner();
    if (sc)
        return sc->get_global();
    return const_cast< entity* >(this)->shared_this< global_namespace >();
}

void
entity::collect_elements(entity_const_set& elems, entity_predicate pred) const
{
    auto shared = shared_from_this();
    if (pred(shared))
        elems.insert(shared);
}

//----------------------------------------------------------------------------
//    type class implementation
//----------------------------------------------------------------------------
bool
type::is_built_in(qname const& qn)
{
    type_ptr t = find_builtin(qn.name());
    return t.get();
}

//----------------------------------------------------------------------------
//    forward_declaration class implementation
//----------------------------------------------------------------------------

forward_declaration::forward_kind
forward_declaration::parse_forward_type(::std::string const& what)
{
    if (what == "struct")
        return forward_declaration::structure;
    if (what == "interface")
        return forward_declaration::interface;
    if (what == "class")
        return forward_declaration::class_;
    if (what == "exception")
        return forward_declaration::exception;
    return forward_declaration::unknown;
}

forward_declaration::forward_declaration(scope_ptr sc, ::std::size_t pos,
            ::std::string const& name, ::std::string const& what)
    : entity(sc, pos, name), type(sc, pos, name), fw_(parse_forward_type(what))
{
    if (fw_ == unknown) {
        ::std::ostringstream os;
        os << "Cannot forward declare " << what;
        throw grammar_error(pos, os.str());
    }
}

bool
forward_declaration::is_resolved() const
{
    if (!resolved_) {
        type_ptr t = owner()->find_type(name());
        if (t.get() != this && !dynamic_entity_cast<forward_declaration>(t)) {
            resolved_ = t;
        }
    }
    return resolved_.get();
}

type_ptr
forward_declaration::forwarded_type() const
{
    if (!is_resolved()) {
        // create a dummy

    }
    return resolved_;
}

bool
forward_declaration::is_compatible(entity_ptr en) const
{
    if (name() == en->name()) {
        if (auto fwd = dynamic_entity_cast< forward_declaration >(en)) {
            return fwd->fw_ == fw_;
        }
        if (dynamic_entity_cast< ast::exception >(en)) {
            return fw_ == exception;
        }
        if (dynamic_entity_cast< ast::class_ >(en)) {
            return fw_ == class_;
        }
        if (dynamic_entity_cast< ast::interface >(en)) {
            return fw_ == interface;
        }
        if (dynamic_entity_cast< ast::structure >(en)) {
            return fw_ == structure;
        }
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
//    constant class implementation
//----------------------------------------------------------------------------
constant::constant(scope_ptr sc, ::std::size_t pos, ::std::string const& name,
        type_ptr t, grammar::data_initializer const& init)
    : entity{sc, pos, name}, type_(t), init_(init)
{
}

//----------------------------------------------------------------------------
//    scope class implementation
//----------------------------------------------------------------------------
::std::pair<scope_ptr, scope_ptr>
scope::find_scope(qname const& qn) const
{
    if (qn.empty())
        return ::std::make_pair(get_global(), scope_ptr{});
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
    if (!sc.first && type::is_built_in(search.back())) {
        return get_global();
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
    entity_ptr en;
    scope_ptr sc = find_scope_of(search);
    if (sc) {
        en =  sc->local_entity_search(search);
    }
    if (!en && search.size() == 1) {
        type_ptr t = find_builtin(search.back());
        if (t)
            return t;
    }
    return en;
}

type_ptr
scope::find_type(type_name const& tn, ::std::size_t pos) const
{
    type_ptr t = find_type(tn.name);
    if (!t)
        return t;
    if (!tn.params.empty()) {
        templated_type_ptr tt = dynamic_type_cast< templated_type >(t);
        if (!tt) {
            ::std::ostringstream os;
            os << "Type " << t->get_type_name() << " cannot be parameterized";
            throw grammar_error{os.str()};
        }
        parametrized_type_ptr pt = tt->create_parametrized_type(
                const_cast< scope* >(this)->shared_this< scope >(), pos);
        for (auto const& param : tn.params) {
            switch (param.which()) {
                case 0: {// type_name_ptr
                    auto const& ptn = *::boost::get< type_name::type_name_ptr >(param);
                    type_ptr tp = find_type( ptn , pos);
                    if (!tp) {
                        ::std::ostringstream os;
                        os << "Template '" << tn.name << "' type parameter '" << ptn << "' not found";
                        throw grammar_error{os.str()};
                    }
                    pt->add_parameter(tp);
                    break;
                }
                case 1: // std::string
                    pt->add_parameter(::boost::get< ::std::string >(param));
                    break;
                default:
                    break;
            }
        }
        t = pt;
    }
    if (tn.is_reference) {
        interface_ptr iface = dynamic_type_cast< interface >(t);
        if (!iface) {
            forward_declaration_ptr fwd = dynamic_type_cast< forward_declaration >(t);
            if (!(fwd && fwd->kind() == forward_declaration::interface)) {
                ::std::ostringstream os;
                os << "Cannot create a proxy to a non-interface type " << t->get_type_name();
                throw grammar_error{os.str()};
            }
        }
        t = ::std::make_shared< reference >(t);
    }
    return t;
}

type_ptr
scope::find_type(qname const& qn) const
{
    if (qn.fully) {
        return get_global()->find_type(qn.search());
    }
    return find_type(qn.search(false));
}

type_ptr
scope::find_type(qname_search const& search) const
{
    if (search.empty()) {
        return type_ptr{};
    }
    if (search.fully && owner().get())
        return get_global()->find_type(search);

    auto sc = find_scope(search.scope());
    if (sc.first) {
        type_ptr t = sc.first->local_type_search(search);
        if (t)
            return t;
    }
    if (sc.first && sc.second->owner()) { // Proceed to parent scope only if not searching from global namespace
        sc.first = sc.first->owner();
        if (sc.first) {
            return sc.first->find_type(search);
        }
    }

    return find_builtin(search.back());;
}

type_ptr
scope::local_type_search(qname_search const& search) const
{
    if (search.empty())
        return type_ptr{};

    for (auto const t : types_) {
        if (t->name() == search.back())
            return t;
    }
    for (auto const t : forwards_) {
        if (t->name() == search.back())
            return t;
    }
    return type_ptr{};
}

scope_ptr
scope::local_scope_search(qname_search const& search) const
{
    if (search.empty()) {
        return const_cast< scope* >(this)->shared_this<scope>();
    }
    for (auto const t : types_) {
        if (t->name() == search.front())
            return dynamic_type_cast<scope>(t);
    }
    return scope_ptr{};
}

entity_ptr
scope::local_entity_search(qname_search const& search) const
{
    if (!search.empty()) {
        entity_ptr ent = local_type_search(search);
        if (ent)
            return ent;
        for (auto const c : constants_) {
            if (c->name() == search.back()) {
                return c;
            }
        }
    }
    return entity_ptr{};
}

constant_ptr
scope::add_constant(::std::size_t pos, ::std::string const& name, type_ptr t,
        grammar::data_initializer const& init)
{
    constant_ptr cn = ::std::make_shared< constant >(
            shared_this< scope >(), pos, name, t, init);
    constants_.push_back(cn);
    on_add_entity(cn);
    return cn;
}

void
scope::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    for (auto const& t : types_) {
        t->collect_dependencies(deps, pred);
    }
    for (auto const& c : constants_) {
        c->collect_dependencies(deps, pred);
    }
}

void
scope::collect_elements(entity_const_set& elems, entity_predicate pred) const
{
    entity::collect_elements(elems, pred);
    for (auto const& t : types_) {
        if (pred(t))
            elems.insert(t);
        t->collect_elements(elems, pred);
    }
    for (auto const& c : constants_) {
        if (pred(c))
            elems.insert(c);
    }
}

//----------------------------------------------------------------------------
//    function class implementation
//----------------------------------------------------------------------------
function::function(interface_ptr sc, ::std::size_t pos, ::std::string const& name,
        type_ptr ret, bool is_const,
        function_params const& params,
        exception_list const& t_spec)
    : entity(sc, pos, name),
      ret_type_{ ret },
      parameters_{ params }, is_const_(is_const), throw_spec_(t_spec)
{
    if (!ret_type_) {
        ret_type_ = get_global()->find_type("void");
    }
}

bool
function::is_void() const
{
    return ret_type_->name() == "void";
}

void
function::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    if (pred(ret_type_))
        deps.insert(ret_type_);
    for (auto const& p : parameters_) {
        if (pred(p.first))
            deps.insert(p.first);
        p.first->collect_dependencies(deps, pred);
    }
    for (auto const& e : throw_spec_) {
        if (pred(e))
            deps.insert(e);
    }
}
//----------------------------------------------------------------------------
//    namespace_ class implementation
//----------------------------------------------------------------------------

namespace_ptr
namespace_::add_namespace(::std::size_t pos, qname const& qn)
{
    if (qn.fully && owner()) {
        throw ::std::runtime_error("Cannot add a globally scoped namespace");
    }
    return add_namespace(pos, qname_search{false, qn.begin(), qn.end()});
}

namespace_ptr
namespace_::add_namespace(::std::size_t pos, qname_search const& s)
{
    auto f = nested_.find(*s.begin);
    if (f == nested_.end()) {
        entity_ptr en = local_scope_search(qname{*s.begin}.search());
        if (en) {
            throw entity_conflict{ en, pos };
        }
        auto ns = ::std::make_shared< namespace_ >(
                shared_this< namespace_ >(), pos, *s.begin );
        f = nested_.emplace( *s.begin, ns ).first;
    }
    auto n = s.begin + 1;
    if (n != s.end) {
        return f->second->add_namespace(pos, qname_search{false, n, s.end});
    }
    return f->second;
}

scope_ptr
namespace_::local_scope_search(qname_search const& search) const
{
    if (search.fully && !is_global()) {
        return get_global()->local_scope_search(search);
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
        return get_global()->find_namespace(search);
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

void
namespace_::on_add_entity(entity_ptr en)
{
    get_global()->on_add_entity(en);
}

//----------------------------------------------------------------------------
global_namespace_ptr
global_namespace::create()
{
    return global_namespace_ptr{ new global_namespace{} };
}

compilation_unit_ptr
global_namespace::current_compilation_unit()
{
    return current_;
}

void
global_namespace::set_current_compilation_unit(::std::string const& name)
{
    auto f = units_.find(name);
    if (f == units_.end()) {
        f = units_.insert(::std::make_pair( name, ::std::make_shared< compilation_unit >( name ) )).first;
    }
    current_ = f->second;
    assert(current_.get() && "Current compilation unit");
}

void
global_namespace::on_add_entity(entity_ptr en)
{
    if (current_) {
        current_->entities.push_back(en);
    }
}

//----------------------------------------------------------------------------
//    enumeration class implementation
//----------------------------------------------------------------------------
void
enumeration::add_enumerator(::std::size_t pos, ::std::string const& name, optional_value val)
{
    // check for entity in enclosing scope to avoid clashes
    if (!constrained_) {
        auto en = owner()->local_entity_search(qname{name}.search());
        if (en) {
            throw entity_conflict(en, pos);
        }
    }
    for (auto const& e : enumerators_) {
        if (e.first == name)
            throw ::std::runtime_error("Duplicate enumerator identifier");
    }
    enumerators_.emplace_back(name, val);
    // TODO add entities to enclosing scope if not constrained
}
//----------------------------------------------------------------------------
//    structure class implementation
//----------------------------------------------------------------------------

variable_ptr
structure::add_data_member(::std::size_t pos, ::std::string const& name, type_ptr t)
{
    entity_ptr en = local_entity_search(qname{name}.search());
    if (en) {
        throw entity_conflict{ en, pos };
    }
    variable_ptr member =
            ::std::make_shared< variable >( shared_this<structure>(), pos, name, t );
    data_members_.push_back( member );
    return member;
}

variable_ptr
structure::find_member(::std::string const& name) const
{
    for (auto const& dm : data_members_) {
        if (dm->name() == name)
            return dm;
    }
    return variable_ptr{};
}

entity_ptr
structure::local_entity_search(qname_search const& search) const
{
    entity_ptr ent = scope::local_entity_search(search);
    if (!ent) {
        ent = find_member(search.back());
    }
    return ent;
}

void
structure::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    scope::collect_dependencies(deps, pred);
    for (auto const& m : data_members_) {
        m->collect_dependencies(deps, pred);
    }
}

void
structure::collect_elements(entity_const_set& elems, entity_predicate pred) const
{
    scope::collect_elements(elems, pred);
    for (auto const& m : data_members_) {
        if (pred(m))
            elems.insert(m);
    }
}
//----------------------------------------------------------------------------
//    interface class implementation
//----------------------------------------------------------------------------
function_ptr
interface::add_function(::std::size_t pos, ::std::string const& name,
        type_ptr t, bool is_const,
        function_params const& params, exception_list const& t_spec)
{
    entity_ptr en = local_scope_search(qname{name}.search());
    if (en) {
        throw entity_conflict{ en, pos };
    }
    function_ptr func = ::std::make_shared<function>(
            shared_this<interface>(), pos, name, t, is_const, params, t_spec
        );
    functions_.push_back(func);
    return func;
}

function_ptr
interface::find_function(::std::string const& name) const
{
    for (auto f : functions_) {
        if (f->name() == name)
            return f;
    }
    return function_ptr{};
}

entity_ptr
interface::ancestors_entity_search(qname_search const& search) const
{
    entity_ptr ent{};
    // search ancestors
    for (auto const& anc : ancestors_) {
        ent = anc->local_entity_search(search);
        if (ent)
            break;
    }
    return ent;
}
entity_ptr
interface::local_entity_search(qname_search const& search) const
{
    entity_ptr ent = scope::local_entity_search(search);
    if (!ent) {
        ent = find_function(search.back());
    }
    if (!ent) {
        ent = ancestors_entity_search(search);
    }
    return ent;
}

type_ptr
interface::ancestors_type_search(qname_search const& search) const
{
    type_ptr t{};
    for (auto const& anc : ancestors_) {
        t = anc->local_type_search(search);
        if (t)
            break;
    }
    return t;
}

type_ptr
interface::local_type_search(qname_search const& search) const
{
    type_ptr t = scope::local_type_search(search);
    if (!t) {
        t = ancestors_type_search(search);
    }
    return t;
}

void
interface::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    scope::collect_dependencies(deps, pred);
    for (auto const& a : ancestors_) {
        if (pred(a))
            deps.insert(a);
    }
    for (auto const& f : functions_) {
        f->collect_dependencies(deps, pred);
    }
}

void
interface::collect_elements(entity_const_set& elems, entity_predicate pred) const
{
    scope::collect_elements(elems, pred);
    for (auto const& f : functions_) {
        if (pred(f))
            elems.insert(f);
    }
}

void
interface::collect_ancestors(interface_list& ifaces, entity_predicate pred) const
{
    // first the grand-ancestors
    for (auto a : ancestors_) {
        a->collect_ancestors(ifaces, pred);
    }
    // then the ancestors
    for (auto a : ancestors_) {
        if (pred(a)) {
            bool already_there = false;
            for (auto i : ifaces) {
                if (i == a) {
                    already_there = true;
                    break;
                }
            }
            if (!already_there)
                ifaces.push_back(a);
        }
    }
}

//----------------------------------------------------------------------------
//    reference class implementation
//----------------------------------------------------------------------------
reference::reference(type_ptr iface)
    : entity(iface->owner(), 0, iface->name()),
      type(iface->owner(), 0, iface->name())
{
}

void
reference::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    auto ref_t = owner()->find_type(name());
    if (ref_t) {
        if (pred(ref_t))
            deps.insert(ref_t);
    }
}


//----------------------------------------------------------------------------
//    class class implementation
//----------------------------------------------------------------------------
entity_ptr
class_::local_entity_search(qname_search const& search) const
{
    entity_ptr ent = structure::local_entity_search(search);
    if (!ent) {
        ent = interface::local_entity_search(search);
        if (!ent && parent_) {
            ent = parent_->local_entity_search(search);
        }
    }
    return ent;
}

type_ptr
class_::local_type_search(qname_search const& search) const
{
    type_ptr t = structure::local_type_search(search);
    if (!t) {
        t = interface::local_type_search(search);
        if (!t && parent_) {
            t = parent_->local_type_search(search);
        }
    }
    return t;
}

void
class_::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    scope::collect_dependencies(deps, pred);
    for (auto const& m : data_members_) {
        m->collect_dependencies(deps, pred);
    }
    for (auto const& a : ancestors_) {
        if (pred(a))
            deps.insert(a);
    }
    for (auto const& f : functions_) {
        f->collect_dependencies(deps, pred);
    }
    if (parent_ && pred(parent_))
        deps.insert(parent_);
}

void
class_::collect_elements(entity_const_set& elems, entity_predicate pred) const
{
    scope::collect_elements(elems, pred);
    for (auto const& m : data_members_) {
        if (pred(m))
            elems.insert(m);
    }
    for (auto const& f : functions_) {
        if (pred(f))
            elems.insert(f);
    }
}
//----------------------------------------------------------------------------
//    exception class implementation
//----------------------------------------------------------------------------
entity_ptr
exception::local_entity_search(qname_search const& search) const
{
    entity_ptr ent = structure::local_entity_search(search);
    if (!ent) {
        if (parent_) {
            ent = parent_->local_entity_search(search);
        }
    }
    return ent;
}

type_ptr
exception::local_type_search(qname_search const& search) const
{
    type_ptr t = scope::local_type_search(search);
    if (!t) {
        if (parent_) {
            t = parent_->local_type_search(search);
        }
    }
    return t;
}

void
exception::collect_dependencies(entity_const_set& deps, entity_predicate pred) const
{
    structure::collect_dependencies(deps, pred);
    if (parent_ && pred(parent_))
        deps.insert(parent_);
}

}  // namespace ast
}  // namespace idl
}  // namespace wire



