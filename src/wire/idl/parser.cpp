/*
 * parser.cpp
 *
 *  Created on: Apr 20, 2016
 *      Author: zmij
 */

#include <wire/idl/parser.hpp>
#include <iostream>
#include <iomanip>
#include <functional>
#include <boost/optional.hpp>

namespace wire {
namespace idl {
namespace parser {

parser::parser(::std::string const& cnt)
    : contents{ cnt }, state_{contents}
{
}

ast::global_namespace_ptr
parser::parse()
{
    namespace qi = ::boost::spirit::qi;

    auto sb     = contents.data();
    auto se     = sb + contents.size();

    tokens_type tokens;
    token_iterator iter = tokens.begin(sb, se);
    token_iterator end = tokens.end();

    grammar_type grammar{ tokens, state_ };

    bool r = qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self]);

    if (!r || iter != end) {
        auto loc = state_.get_location( ::std::distance(contents.data(), sb) );
        throw syntax_error(loc, "Unexpected token");
    }
    return state_.get_tree();
}

//----------------------------------------------------------------------------
parser_state::parser_state(::std::string const& contents,
        include_dir_list const& include_dirs)
    : stream_begin(contents.data()),
      loc_jumps{ {0, source_location{}} },
      global_{ ast::global_namespace::create() },
      scopes_{ ::std::make_shared< namespace_scope >( global_ ) },
      include_dirs_{ include_dirs }
{
}

parser_scope&
parser_state::current()
{
    return *scopes_.back();
}

ast::scope_ptr
parser_state::ast_scope()
{
    return current().scope();
}

void
parser_state::update_location(base_iterator p, source_location const& loc)
{
    loc_jumps[ ::std::distance(stream_begin, p) ] = loc;
    global_->set_current_compilation_unit(loc.file);
}

source_location
parser_state::get_location(::std::size_t pos) const
{
    auto f = --loc_jumps.upper_bound( pos );
    source_location loc = f->second;
    for (auto c = stream_begin + f->first; c != stream_begin + pos; ++c) {
        if (*c == '\n') {
            loc.character = 0;
            ++loc.line;
        } else {
            ++loc.character;
        }
    }

    return loc;
}

void
parser_state::start_namespace(::std::size_t pos, ::std::string const& name)
try {
    scopes_.push_back(current().start_namespace(pos, name));
    attach_annotations(ast_scope());
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare namespace '" << name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}


void
parser_state::start_structure(::std::size_t pos, ::std::string const& name)
try {
    ast::structure_ptr st = ast_scope()->add_type< ast::structure >(pos, name);
    scopes_.push_back( ::std::make_shared< structure_scope >(st) );
    attach_annotations(st);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare structure '" << name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::start_interface(::std::size_t pos, ::std::string const& name,
        optional_type_list const& ancestors_names)
try {
    // check the ancestor list
    ast::interface_list ancestors;
    if (ancestors_names.is_initialized()) {
        auto const& names = *ancestors_names;
        for (auto const& an : names) {
            ast::type_ptr t = ast_scope()->find_type(an, pos);
            if (!t) {
                ::std::ostringstream os;
                os << "Parent data type '" << an << "' for interface '"
                        << name << "' not found";
                throw syntax_error(get_location(pos), os.str());
            }
            ast::interface_ptr ai = ast::dynamic_type_cast< ast::interface >(t);
            if (!ai) {
                ::std::ostringstream os;
                os << "Parent data type '" << an << "' for interface '"
                        << name << "' is not an interface";
                throw syntax_error(get_location(pos), os.str());
            }
            ancestors.push_back(ai);
        }
    }
    ast::interface_ptr iface = ast_scope()->add_type< ast::interface >(pos, name, ancestors);
    scopes_.push_back(::std::make_shared< interface_scope >(iface));
    attach_annotations(iface);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare interface '" << name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}


void
parser_state::start_class(::std::size_t pos, ::std::string const& name,
        optional_type_list const& ancestors_names)
try {
    ast::class_ptr parent;
    ast::interface_list ancestors;
    if (ancestors_names.is_initialized()) {
        auto const& names = *ancestors_names;
        for (auto const& an : names) {
            ast::type_ptr t = ast_scope()->find_type(an, pos);
            if (!t) {
                ::std::ostringstream os;
                os << "Parent data type '" << an << "' for class '"
                        << name << "' not found";
                throw syntax_error(get_location(pos), os.str());
            }
            ast::class_ptr pnt = ast::dynamic_type_cast< ast::class_ >(t);
            if (pnt) {
                if (parent) {
                    ::std::ostringstream os;
                    os << "A class cannot have more than one class ancestor. '"
                            << name << "' class has more: '" << parent->get_type_name()
                            << "' and '" << pnt->get_type_name() << "'";
                    throw syntax_error(get_location(pos), os.str());
                }
                parent = pnt;
            } else {
                ast::interface_ptr ai = ast::dynamic_type_cast< ast::interface >(t);
                if (!ai) {
                    ::std::ostringstream os;
                    os << "Parent data type '" << an << "' for class '"
                            << name << "' is not an interface";
                    throw syntax_error(get_location(pos), os.str());
                }
                ancestors.push_back(ai);
            }
        }
    }
    ast::class_ptr cl = ast_scope()->add_type< ast::class_ >(pos, name, parent, ancestors);
    scopes_.push_back(::std::make_shared< class_scope >(cl));
    attach_annotations(cl);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare class '" << name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::start_exception(::std::size_t pos, ::std::string const& name,
        optional_type const& parent_name)
try {
    // check the ancestor
    ast::exception_ptr parent;
    if (parent_name.is_initialized()) {
        ast::type_ptr t = ast_scope()->find_type( *parent_name, pos );
        if (!t) {
            ::std::ostringstream os;
            os << "Parent data type '" << *parent_name << "' for exception '"
                    << name << "' not found";
            throw syntax_error(get_location(pos), os.str());
        }
        parent = ast::dynamic_type_cast< ast::exception >( t );
        if (!parent) {
            ::std::ostringstream os;
            os << "Parent data type '" << *parent_name << "' for exception '"
                    << name << "' is not an exception";
            throw syntax_error(get_location(pos), os.str());
        }
    }
    ast::exception_ptr ex = ast_scope()->add_type< ast::exception >( pos, name, parent );
    scopes_.push_back(::std::make_shared< exception_scope >(ex));
    attach_annotations(ex);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare exception '" << name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::end_scope(::std::size_t pos)
{
    if (scopes_.size() > 1)
        scopes_.pop_back();
    else
        throw syntax_error( get_location(pos), "Cannot end scope here" );
}

void
parser_state::declare_enum(::std::size_t pos, grammar::enum_decl const& decl)
try {
    ast::enumeration_ptr en = ast_scope()->add_type< ast::enumeration >(pos, decl.name, decl.constrained);

    for (auto const& v : decl.enumerators) {
        en->add_enumerator(pos, v.name, v.init);
    }
    attach_annotations(en);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare type '" << decl.name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::add_type_alias(::std::size_t pos, grammar::type_alias_decl const& decl)
try {
    ast::type_ptr aliased = current().scope()->find_type(decl.second, pos);
    if (!aliased) {
        ::std::ostringstream os;
        os << "Data type '" << decl.second << "' not found in scope "
                << current().scope()->get_qualified_name();
        throw grammar_error{pos, os.str()};
    }
    ast::type_ptr ta =  ast_scope()->add_type< ast::type_alias >(pos, decl.first, aliased);
    attach_annotations(ta);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot declare type alias '" << decl.first << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::forward_declare(::std::size_t pos, grammar::fwd_decl const& decl)
try {
    ast::forward_declaration_ptr fwd =
            ast_scope()->add_type< ast::forward_declaration >(pos, decl.second, decl.first);
    attach_annotations(fwd);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot forward declare type '" << decl.first << " " << decl.second << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::add_constant(::std::size_t pos, grammar::data_member_decl const& decl)
try {
    ast::type_ptr t = ast_scope()->find_type(decl.type, pos);
    if (!t) {
        ::std::ostringstream os;
        os << "Data type '" << decl.type << "' not found in scope "
                << ast_scope()->get_qualified_name();
        throw syntax_error{get_location(pos), os.str()};
    }
    if (!decl.init.is_initialized()) {
        ::std::ostringstream os;
        os << "No initializer specified for constant '" << decl.type << " " << decl.name
                << "' in scope " << ast_scope()->get_qualified_name();
        throw syntax_error{get_location(pos), os.str()};
    }
    // TODO Check compatible init
    // string - quoted literal
    // integral types - integral literals
    // boolean types - bool or integral literals
    // sequences, maps and arrays - initializer lists
    // struct - initializer lists
    ast::constant_ptr var = ast_scope()->add_constant(pos, decl.name, t, *decl.init);
    attach_annotations(var);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot add constant '" << decl.name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::add_data_member(::std::size_t pos, grammar::data_member_decl const& decl)
try {
    ast::variable_ptr var = current().add_data_member(pos, decl);
    attach_annotations(var);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot add data member '" << decl.type << " " << decl.name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::add_func_member(::std::size_t pos, grammar::function_decl const& decl)
try {
    ast::function_ptr func = current().add_func_member(pos, decl);
    attach_annotations(func);
} catch (syntax_error const&) {
    throw;
} catch (ast::entity_conflict const& e) {
    ::std::ostringstream os;
    os << "Cannot add function member '" << decl.name << "'\n"
        << get_location(e.previous->decl_position()) << ": note: Previously declared here";
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), os.str());
} catch (grammar_error const& e) {
    throw syntax_error( get_location(e.pos != 0 ? e.pos : pos), e.what() );
} catch (::std::exception const& e) {
    throw syntax_error( get_location(pos), e.what() );
}

void
parser_state::add_annotations(::std::size_t pos, grammar::annotation_list const& annotations)
{
    current_annotations_.insert(current_annotations_.end(),
            annotations.begin(), annotations.end());
}

void
parser_state::attach_annotations(ast::entity_ptr en)
{
    if (!current_annotations_.empty()) {
        en->add_annotations(current_annotations_);
        current_annotations_.clear();
    }
}

//----------------------------------------------------------------------------
parser_scope::scope_ptr
parser_scope::start_namespace_impl(::std::size_t pos, ::std::string const& name)
{
    ::std::ostringstream os;
    os << "Cannot start a namespace in scope " << scope()->get_qualified_name();
    throw grammar_error{ pos, os.str() };
}

ast::variable_ptr
parser_scope::add_data_member_impl(::std::size_t pos, grammar::data_member_decl const& decl)
{
    ::std::ostringstream os;
    os << "Cannot add a data member in scope " << scope()->get_qualified_name();
    throw grammar_error{ pos, os.str() };
}

ast::function_ptr
parser_scope::add_func_member_impl(::std::size_t pos, grammar::function_decl const& decl)
{
    ::std::ostringstream os;
    os << "Cannot add a function member in scope " << scope()->get_qualified_name();
    throw grammar_error{ pos, os.str() };
}

//----------------------------------------------------------------------------
parser_scope::scope_ptr
namespace_scope::start_namespace_impl(::std::size_t pos, ::std::string const& name)
{
    ast::namespace_ptr ns = scope< ast::namespace_ >()->add_namespace(pos, name);
    return ::std::make_shared< namespace_scope >(ns);
}

//----------------------------------------------------------------------------
ast::variable_ptr
structure_scope::add_data_member_impl(::std::size_t pos, grammar::data_member_decl const& decl)
{
    ast::structure_ptr st = scope< ast::structure >();
    ast::type_ptr t = st->find_type(decl.type, pos);
    if (!t) {
        ::std::ostringstream os;
        os << "Data type '" << decl.type << "' not found in scope " << st->get_qualified_name();
        throw grammar_error{pos, os.str()};
    }
    ast::templated_type_ptr tt = ast::dynamic_type_cast< ast::templated_type >(t);
    if (tt) {
        ::std::ostringstream os;
        os << "Cannot use template type '" << decl.type << "' without parameters";
        throw grammar_error{pos, os.str()};
    }
    return st->add_data_member(pos, decl.name, t);
}

//----------------------------------------------------------------------------
ast::function_ptr
interface_scope::add_func_member_impl(::std::size_t pos, grammar::function_decl const& decl)
{
    ast::interface_ptr iface = scope< ast::interface >();
    ast::type_ptr ret = iface->find_type(decl.return_type, pos);

    if (!ret) {
        ::std::ostringstream os;
        os << "Return data type '" << decl.return_type << "' for function '"
                << decl.name << "' not found in scope " << iface->get_qualified_name();
        throw grammar_error(pos, os.str());
    }
    ast::function::function_params params;
    ast::exception_list throw_spec;
    for (auto const& p : decl.params) {
        auto t = iface->find_type(p.first, pos);
        if (!t) {
            ::std::ostringstream os;
            os << "Data type '" << p.first << "' for function '"
                    << decl.name << "' parameter '" << p.second
                    << "' not found in scope " << iface->get_qualified_name();
            throw grammar_error(pos, os.str());
        }
        params.push_back(::std::make_pair( t, p.second ));
    }

    for (auto const& e : decl.throw_spec) {
        auto t = iface->find_type(e, pos);
        if (!t) {
            ::std::ostringstream os;
            os << "Data type '" << e << "' from function '" << decl.name
                    << "' throw specification not found in scope "
                    << iface->get_qualified_name();
            throw grammar_error(pos, os.str());
        }
        auto ex = ast::dynamic_type_cast< ast::exception >(t);
        if (!ex) {
            ::std::ostringstream os;
            os << "Data type '" << e << "' from function '" << decl.name
                    << "' throw specification is not an exception";
            throw grammar_error(pos, os.str());
        }
        throw_spec.push_back(ex);
    }

    return iface->add_function(pos, decl.name, ret, decl.const_qualified, params, throw_spec);
}

}  // namespace parser
}  // namespace idl
}  // namespace wire
