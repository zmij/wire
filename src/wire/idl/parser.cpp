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

//----------------------------------------------------------------------------
parser_state::parser_state()
    : stack_
      {
        ::std::make_shared< namespace_scope >( *this, ast::namespace_::global() )
      }
{
    ast::namespace_::clear_global();
}

void
parser_state::process_token(source_location const& source_loc, token_value_type const& tkn)
{
    current_scope().process_token(source_loc, tkn);
}

parser_scope&
parser_state::current_scope()
{
    return *stack_.back();
}

void
parser_state::pop_scope(source_location const& loc)
{
    if (stack_.size() > 1)
        stack_.pop_back();
    else
        throw syntax_error(loc, "Unexpected end of scope");
}
//----------------------------------------------------------------------------
struct expect_semicolon_after_block;
struct type_name_expect_scope;
struct type_name_expect_identifier;
struct type_name_template_args;
struct expect_type_name;

//----------------------------------------------------------------------------
struct skip_leading_whitespace : phrase_parse {
    skip_leading_whitespace(parser_scope& sc, phrase_parse_ptr nxt)
        : phrase_parse(sc, rule::skip_whitespace), next(nxt)
    {
    }
    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_eol:
            case lexer::token_whitespace:
                break;
            default:
                return next->process_token(loc, tkn);
        }
        return shared_from_this();
    }
    phrase_parse_ptr next;
};

//----------------------------------------------------------------------------
struct expect_identifier : phrase_parse {
    expect_identifier(parser_scope& sc, ::std::string& id)
        : phrase_parse(sc, rule::identifier), identifier(id)
    {
    }

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_identifier:
                identifier = ::std::string{ tkn.value().begin(), tkn.value().end() };
                break;
            case lexer::token_eol:
            case lexer::token_whitespace:
                return shared_from_this();
            default:
                throw syntax_error(loc, "Identifier expected");
        }
        return phrase_parse_ptr{};
    }

    ::std::string& identifier;
};

//----------------------------------------------------------------------------
struct block_name_parse : phrase_parse {
    block_name_parse(parser_scope& sc, rule t)
        : phrase_parse(sc, t) {}
    virtual ~block_name_parse() {};

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        if (tkn.id() == lexer::token_whitespace)
            return shared_from_this();
        if (identifier.empty()) {
            if (tkn.id() != lexer::token_identifier)
                throw syntax_error(loc, "Identifier expected");
            identifier = ::std::string{tkn.value().begin(), tkn.value().end()};
        } else {
            // Expect a block open token or a statement end (for forward declaration)
            switch (tkn.id()) {
                case lexer::token_semicolon:
                    if (!current_phrase.process_token(loc, tkn)) {
                        if (type == rule::namespace_name) {
                            throw syntax_error(loc, "Cannot forward declare a namespace");
                        }
                        if (!ancestors.empty())
                            throw syntax_error(loc, "Cannot declare bases in a forward declaration");
                        // notify scope about forward declaration
                        scope.forward_declare(loc, type, identifier);
                        return phrase_parse_ptr{};
                    }
                    break;
                case lexer::token_block_start:
                    if (!current_phrase.process_token(loc, tkn)) {
                        // notify scope about scope open
                        scope.open_scope(loc, type, identifier, ancestors);
                        if (type == rule::namespace_name)
                            return phrase_parse_ptr{}; // Don't want a semicolon here
                        return next_phrase<expect_semicolon_after_block>();
                    }
                    break;
                case lexer::token_colon: {
                    if (!current_phrase.process_token(loc, tkn)) {
                        switch (type) {
                            case rule::namespace_name:
                            case rule::structure_name:
                                throw syntax_error(loc, "Cannot derive a namespace or structure");
                            default:
                                break;
                        }
                        current_phrase.set_current_phrase< expect_type_name >(scope,
                                [&](ast::type_ptr t) { ancestors.push_back(t); }
                        );
                    }
                    break;
                }
                case lexer::token_comma:
                    if (!current_phrase.process_token(loc, tkn)) {
                        switch (type) {
                            case rule::namespace_name:
                            case rule::structure_name:
                                throw syntax_error(loc, "Cannot derive a namespace or structure");
                            case rule::exception_name:
                                if (ancestors.size() > 0) {
                                    throw syntax_error(loc, "An exception cannot have more than one base");
                                }
                                break;
                            default:
                                break;
                        }
                        current_phrase.set_current_phrase< expect_type_name >(scope,
                                [&](ast::type_ptr t) { ancestors.push_back(t); }
                        );
                    }
                    break;
                case lexer::token_whitespace:
                    if (!current_phrase.process_token(loc, tkn)) {
                    }
                    break;
                default:
                    if (!current_phrase.process_token(loc, tkn)) {
                        throw syntax_error(loc, "Unexpected token (block name)");
                    }
            }
        }
        return shared_from_this();
    }

    phrase_parser   current_phrase;
    ::std::string   identifier;
    ast::type_list  ancestors;
};

struct expect_semicolon_after_block : phrase_parse {
    expect_semicolon_after_block(parser_scope& sc)
        : phrase_parse(sc, rule::semicolon_after_block) {}
    virtual ~expect_semicolon_after_block() {};

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_whitespace:
                return shared_from_this();
            case lexer::token_semicolon:
                break;
            default:
                throw syntax_error(loc, "Semicolon expected");
        }
        return phrase_parse_ptr{};
    }
};

struct type_name_expect_scope : phrase_parse {
    using qname_ptr = ::std::shared_ptr<qname>;
    using type_set = ::std::function< void(ast::type_ptr) >;

    type_name_expect_scope(parser_scope& sc,
            ::std::string const& identifier, type_set f)
        : phrase_parse(sc, rule::type_name),
          qname_{ ::std::make_shared<qname>( identifier ) },
          func{f} {}
    type_name_expect_scope(parser_scope& sc, qname_ptr qn, type_set f)
        : phrase_parse(sc, rule::type_name),
          qname_{qn}, func{f} {}

    virtual ~type_name_expect_scope() {}

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_scope_resolution:
                return next_phrase< type_name_expect_identifier >(qname_, func);
            case lexer::token_eol:
            case lexer::token_whitespace:
                if (func) {
                    ast::type_ptr ast_type = scope.scope()->find_type(*qname_);
                    if (!ast_type)
                        throw syntax_error(loc, "Type name not found");
                    func(ast_type);
                }
                break;
            case lexer::token_angled_open: {
                ast::type_ptr ast_type = scope.scope()->find_type(*qname_);
                if (!ast_type)
                    throw syntax_error(loc, "Type name not found");
                return next_phrase<type_name_template_args>(loc, ast_type, func);
            }
            default:
                throw syntax_error(loc, "Unexpected token (type name expect scope)");
        }
        return phrase_parse_ptr{};
    }

    bool
    want_token(source_location const& loc, token_value_type const& tkn) const override
    {
        switch(tkn.id()) {
            case lexer::token_semicolon:
            case lexer::token_block_start:
            case lexer::token_angled_close:
            case lexer::token_comma:
            case lexer::token_eol:
            case lexer::token_whitespace: {
                if (func) {
                    ast::type_ptr ast_type = scope.scope()->find_type(*qname_);
                    if (!ast_type)
                        throw syntax_error(loc, "Type name not found");
                    func(ast_type);
                }
                return false;
            }
            default:
                break;
        }
        return true;
    }

    qname_ptr       qname_;
    type_set       func;
};

struct type_name_expect_identifier : phrase_parse {
    using qname_ptr = ::std::shared_ptr<qname>;
    using type_set = ::std::function< void(ast::type_ptr) >;

    type_name_expect_identifier(parser_scope& sc, type_set f)
        : phrase_parse(sc, rule::type_name),
          qname_{ ::std::make_shared<qname>(true) },
          func{f} {}
    type_name_expect_identifier(parser_scope& sc,
            qname_ptr qn, type_set f)
        : phrase_parse(sc, rule::type_name),
          qname_{ qn }, func{f} {}

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_identifier:
                qname_->components.emplace_back( tkn.value().begin(), tkn.value().end() );
                return next_phrase< type_name_expect_scope >( qname_, func );
            default:
                throw syntax_error(loc, "Unexpected token (type name)");
        }
        return phrase_parse_ptr{};
    }
    qname_ptr qname_;
    type_set func;
};

struct type_name_template_args : phrase_parse {
    using type_set = ::std::function< void(ast::type_ptr) >;
    using parameter = ast::parametrized_type::parameter;
    using parameter_ptr = ::std::shared_ptr<ast::parametrized_type::parameter>;

    type_name_template_args(parser_scope& sc, source_location const& loc,
            ast::type_ptr t, type_set f)
        : phrase_parse(sc, rule::template_args), func(f)
    {
        ast::templated_type_ptr tmpl =
                ast::dynamic_entity_cast< ast::templated_type >(t);
        if (!tmpl) {
            ::std::ostringstream os;
            os << "Data type " << t->get_qualified_name() << " is not a template";
            throw syntax_error(loc, os.str());
        }
        type_ = tmpl->create_parametrized_type( scope.scope() );
    }

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch(tkn.id()) {
            case lexer::token_scope_resolution:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (current_param_)
                        throw syntax_error(loc, "Unexpected identifier (template params)");
                    // start a type parser
                    current_phrase.set_current_phrase< type_name_expect_identifier >(
                            scope,
                            [&](ast::type_ptr t) mutable
                            {
                                current_param_ =  ::std::make_shared< parameter >(t);
                            }
                    );
                }
                break;
            case lexer::token_identifier:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (current_param_)
                        throw syntax_error(loc, "Unexpected identifier (template params)");
                    // start a type parser
                    current_phrase.set_current_phrase< type_name_expect_scope >(
                            scope,
                            ::std::string{ tkn.value().begin(), tkn.value().end() },
                            [&](ast::type_ptr t) mutable
                            {
                                current_param_ =  ::std::make_shared< parameter >(t);
                            }
                    );
                }
                break;
            case lexer::token_number:
            case lexer::token_hex_number:
            case lexer::token_oct_number:
                if (!current_phrase.process_token(loc, tkn)) {
                    // we got an integral param here
                    if (current_param_)
                        throw syntax_error(loc, "Unexpected numeric literal (template params)");
                    current_param_ = ::std::make_shared< parameter >(
                            ::std::string{ tkn.value().begin(), tkn.value().end() } );
                }
                break;
            case lexer::token_comma:
                if (!current_phrase.process_token(loc, tkn)) {
                    // done with current param
                    if (!current_param_)
                        throw syntax_error(loc, "Unexpected comma in template params");
                    type_->add_parameter(loc, *current_param_);
                    current_param_.reset();
                }
                break;
            case lexer::token_angled_open:
                if (!current_phrase.process_token(loc, tkn)) {
                    // start a nested template parser
                    if (current_param_ && current_param_->which() == ast::template_param_type::type) {
                        current_phrase.set_current_phrase< type_name_template_args >(
                                scope, loc, ::boost::get< ast::type_ptr >(*current_param_),
                                [&](ast::type_ptr t) mutable
                                {
                                    current_param_ =  ::std::make_shared< parameter >(t);
                                }
                        );
                    } else {
                        throw syntax_error(loc, "Unexpected start of template");
                    }
                }
                break;
            case lexer::token_angled_close:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (!current_param_)
                        throw syntax_error(loc, "Unexpected comma in template params");
                    type_->add_parameter(loc, *current_param_);
                    current_param_.reset();
                    // we're done
                    if (func) {
                        func(type_);
                    }
                    return phrase_parse_ptr{};
                }
                break;
            case lexer::token_eol:
            case lexer::token_whitespace:
                current_phrase.process_token(loc, tkn);
                break;
            default:
                throw syntax_error(loc, "Unexpected token (template parameters)");
        }
        return shared_from_this();
    }

    phrase_parser               current_phrase;
    ast::parametrized_type_ptr  type_;
    type_set                    func;
    parameter_ptr               current_param_;
};

struct member_decl : phrase_parse {
    using qname_ptr = ::std::shared_ptr<qname>;
    enum member_type {
        data,
        function
    };
    member_decl(parser_scope& sc, token_value_type const& tkn)
        : phrase_parse(sc, rule::member_declaration)
    {
        switch (tkn.id()) {
            case lexer::token_const:
                is_const_ = true;
                break;
            case lexer::token_identifier:
                current_phrase.set_current_phrase< type_name_expect_scope >(
                        scope,
                        ::std::string{ tkn.value().begin(), tkn.value().end() },
                        [&](ast::type_ptr qn) mutable { data_type_ = qn; }
                );
                break;
            case lexer::token_scope_resolution:
                current_phrase.set_current_phrase< type_name_expect_identifier >(
                        scope,
                        [&](ast::type_ptr qn) mutable { data_type_ = qn; });
                break;
            default:
                break;
        }

    }

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_const:
                if (is_const_.is_initialized())
                    throw syntax_error(loc, "Extra const qualifier");
                is_const_ = true;
                break;
            case lexer::token_identifier:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (!data_type_) {
                        current_phrase.set_current_phrase< type_name_expect_scope >(
                                scope,
                                ::std::string{ tkn.value().begin(), tkn.value().end() },
                                [&](ast::type_ptr qn) mutable { data_type_ = qn; }
                        );
                    } else {
                        if (!member_name_.empty())
                            throw syntax_error(loc, "Unexpected identifier (parse member)");
                        member_name_ = ::std::string{ tkn.value().begin(), tkn.value().end() };
                    }
                }
                break;
            case lexer::token_comma:
                if (!current_phrase.process_token(loc, tkn)) {
                    throw syntax_error(loc, "Unexpected comma (parse member)");
                }
                break;
            case lexer::token_scope_resolution:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (!data_type_)
                        current_phrase.set_current_phrase< type_name_expect_identifier >(
                                scope,
                                [&](ast::type_ptr qn) mutable { data_type_ = qn; });
                    else
                        throw syntax_error(loc, "Unexpected scope resolution token");
                }
                break;
            case lexer::token_semicolon:
                // Notify owner about a member
                if (type == data) {
                    if (is_const_.is_initialized()) {
                        // Add a constant
                    } else {
                        // Add a data member
                        scope.add_data_member(loc, data_type_, member_name_);
                    }
                } else {

                }
                return phrase_parse_ptr{};
            case lexer::token_angled_open:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (!member_name_.empty())
                        throw syntax_error(loc, "Unexpected template params start");
                    current_phrase.set_current_phrase< type_name_template_args >(
                            scope, loc, data_type_,
                            [&](ast::type_ptr qn) mutable { data_type_ = qn; }
                    );
                }
                break;
            case lexer::token_angled_close:
                if (!current_phrase.process_token(loc, tkn)) {
                    throw syntax_error(loc, "Unexpected template params end");
                }
                break;
            case lexer::token_brace_open:
                type = function;
                break;
            case lexer::token_brace_close:
                break;
            case lexer::token_number:
            case lexer::token_hex_number:
            case lexer::token_oct_number:
                if (!current_phrase.process_token(loc, tkn)) {
                    throw syntax_error(loc, "Unexpected numeric literal (parse member)");
                }
                break;
            case lexer::token_eol:
            case lexer::token_whitespace:
                current_phrase.process_token(loc, tkn);
                break;
            default:
                throw syntax_error(loc, "Unexpected token (parse member)");
        }
        return shared_from_this();
    }

    member_type                 type = data;
    ::boost::optional< bool >   is_const_;
    phrase_parser               current_phrase;
    ast::type_ptr               data_type_;
    ::std::string               member_name_;
};

struct expect_type_name : phrase_parse {
    using type_set = ::std::function< void(ast::type_ptr) >;

    expect_type_name(parser_scope& sc, type_set f)
        : phrase_parse(sc, rule::type_name), func(f)
    {
    }
    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_identifier:
                return next_phrase< type_name_expect_scope >(
                        ::std::string{ tkn.value().begin(), tkn.value().end() },
                        func
                );
                break;
            case lexer::token_scope_resolution:
                return next_phrase< type_name_expect_identifier >(func);
                break;
            case lexer::token_eol:
            case lexer::token_whitespace:
                break;
            default:
                throw syntax_error(loc, "Type name expected");
        }
        return shared_from_this();
    }
    type_set func;
};

struct type_alias_decl : phrase_parse {
    type_alias_decl(parser_scope& sc)
        : phrase_parse(sc, rule::type_alias),
          member_name_{}
    {
        current_phrase.set_current_phrase< skip_leading_whitespace >(
                scope, next_phrase< expect_identifier >(member_name_));
    }
    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_assign:
                if (!current_phrase.process_token(loc, tkn)) {
                    current_phrase.set_current_phrase< expect_type_name >(
                            scope,
                            [&](ast::type_ptr t) mutable { aliased_type = t; }
                    );
                }
                break;
            case lexer::token_angled_open:
                if (!current_phrase.process_token(loc, tkn)) {
                    current_phrase.set_current_phrase< type_name_template_args >(
                            scope, loc, aliased_type,
                            [&](ast::type_ptr t) mutable { aliased_type = t; }
                    );
                }
                break;
            case lexer::token_eol:
            case lexer::token_whitespace:
                current_phrase.process_token(loc, tkn);
                break;
            case lexer::token_semicolon:
                if (!current_phrase.process_token(loc, tkn)) {
                    if (!aliased_type)
                        throw syntax_error(loc, "Aliased type not found");
                    scope.add_type_alias(loc, member_name_, aliased_type);
                    return phrase_parse_ptr{};
                }
            default:
                if (!current_phrase.process_token(loc, tkn)) {
                    throw syntax_error(loc, "Unexpected token (type alias)");
                }
                break;
        }
        return shared_from_this();
    }

    ::std::string       member_name_;
    ast::type_ptr       aliased_type;
    phrase_parser       current_phrase;
};

//----------------------------------------------------------------------------
void
parser_scope::process_token(source_location const& loc, token_value_type const& tkn)
{
    switch (tkn.id()) {
        case lexer::token_locn:
            ::std::cerr << "Update file location\n"
                 << loc.file << "\n"
                 << ::std::setw(3) << loc.line << ": ";
            break;
        case lexer::token_ns:
            ::std::cerr << "Namespace";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected namespace token");
            }
            start_namespace(loc);
            break;
        case lexer::token_struct:
            ::std::cerr << "Structure";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected struct token");
            }
            set_current_phrase< block_name_parse >(rule::structure_name);
            break;
        case lexer::token_class:
            ::std::cerr << "Class";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected class token");
            }
            set_current_phrase< block_name_parse >(rule::class_name);
            break;
        case lexer::token_interface:
            ::std::cerr << "Interface";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected interface token");
            }
            set_current_phrase< block_name_parse >(rule::interface_name);
            break;
        case lexer::token_exception:
            ::std::cerr << "Exception";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected exception token");
            }
            set_current_phrase< block_name_parse >(rule::exception_name);
            break;
        case lexer::token_enum:
            ::std::cerr << "Enumeration";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected enum token");
            }
            break;
        case lexer::token_const:
            ::std::cerr << "const";
            if (!current_phrase.process_token(loc, tkn)) {
                set_current_phrase< member_decl >( tkn );
            }
            break;
        case lexer::token_using:
            ::std::cerr << "using";
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected using token");
            }
            set_current_phrase< type_alias_decl >();
            break;
        case lexer::token_throw:
            ::std::cerr << "throw";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected throw token");
            }
            break;

        case lexer::token_comma:
            ::std::cerr << ",";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected comma");
            }
            break;
        case lexer::token_colon:
            ::std::cerr << ":";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected colon");
            }
            break;
        case lexer::token_scope_resolution:
            ::std::cerr << ":SCOPE:";
            if (!current_phrase.process_token(loc, tkn)) {
                set_current_phrase< member_decl >( tkn );
            }
             break;
        case lexer::token_semicolon:
            ::std::cerr << ";";
            if (!current_phrase) {
                throw syntax_error(loc, "Unexpected statement end");
            }
            current_phrase.process_token(loc, tkn);
            break;
        case lexer::token_assign:
            ::std::cerr << "=";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected assign token");
            }
            break;
        case lexer::token_asterisk:
            ::std::cerr << "*";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected asterisk token");
            }
            break;
        case lexer::token_brace_open:
            ::std::cerr << "(";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected open brace");
            }
            break;
        case lexer::token_brace_close:
            ::std::cerr << ")";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected close brace");
            }
            break;
        case lexer::token_block_start:
            if (!current_phrase) {
                throw syntax_error(loc, "Unexpected block start");
            }
            current_phrase.process_token(loc, tkn);
            break;
        case lexer::token_block_end:
            if (current_phrase) {
                throw syntax_error(loc, "Unexpected block end");
            }
            ::std::cerr << "}";
            close_scope(loc);
            break;
        case lexer::token_angled_open:
            ::std::cerr << "<";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected template parameters start");
            }
            break;
        case lexer::token_angled_close:
            ::std::cerr << ">";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected template parameters end");
            }
            break;
        case lexer::token_attrib_start:
            ::std::cerr << "[[";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected attributes start");
            }
            break;
        case lexer::token_attrib_end:
            ::std::cerr << "]]";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected attributes end");
            }
            break;

        case lexer::token_identifier:
            ::std::cerr << "Identifier("
                  << ::std::string{ tkn.value().begin(), tkn.value().end() } << ")";
            if (!current_phrase.process_token(loc, tkn)) {
                set_current_phrase< member_decl >( tkn );
            }
            break;
        case lexer::token_number:
        case lexer::token_oct_number:
        case lexer::token_hex_number:
            ::std::cerr << "NUMBER";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected numeric literal");
            }
            break;
        case lexer::token_float_literal:
            ::std::cerr << "FLOAT";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected float literal");
            }
            break;
        case lexer::token_quoted_string:
            ::std::cerr << "\"string literal\"";
            if (!current_phrase.process_token(loc, tkn)) {
                throw syntax_error(loc, "Unexpected string literal");
            }
            break;

        case lexer::token_c_comment:
            ::std::cerr << "C Comment "
                 << ::std::string{ tkn.value().begin(), tkn.value().end() } << "\n"
                << ::std::setw(3) << loc.line << ": ";
            break;
        case lexer::token_cpp_comment:
            ::std::cerr << "CPP Comment: "
                 << ::std::string{ tkn.value().begin(), tkn.value().end() }
                << ::std::setw(3) << loc.line << ": ";
            break;
        case lexer::token_eol:
            ::std::cerr << "\n"
                 << ::std::setw(3) << loc.line << ": ";
            current_phrase.process_token(loc, tkn);
            break;
        case lexer::token_whitespace:
            ::std::cerr << *tkn.value().begin();
            current_phrase.process_token(loc, tkn);
            break;
        default:
            ::std::cerr << "{_|_}";
            throw syntax_error(loc, "Unrecognized token");
    }
}

void
parser_scope::open_scope(source_location const& loc, rule type,
        ::std::string const& identifier, ast::type_list const& ancestors)
{
    ::std::cerr << "// Open scope " << identifier << "\n";
    open_scope_impl(loc, type, identifier, ancestors);
}

void
parser_scope::open_scope_impl(source_location const& loc, rule type,
        ::std::string const& identifier, ast::type_list const& ancestors)
{
    if (type == rule::namespace_name) {
        throw syntax_error(loc, "Cannot open a namespace at this scope");
    }
    // FIXME Check if identifier already taken
    switch (type) {
        case rule::structure_name: {
            ast::structure_ptr s = scope()->add_type< ast::structure >(identifier);
            state_.push_scope< structure_scope >(s);
            break;
        }
        case rule::interface_name: {
            ast::interface_list implements;
            if (!ancestors.empty()) {
                for (auto const& a : ancestors) {
                    ast::interface_ptr i = ast::dynamic_type_cast< ast::interface >(a);
                    if (!i) {
                        throw syntax_error(loc,
                                "Interface can be derived from an interface only");
                    }
                    implements.push_back(i);
                }
            }
            ast::interface_ptr i = scope()->add_type< ast::interface >(identifier, implements);
            state_.push_scope< interface_scope >(i);
            break;
        }
        case rule::class_name: {
            ast::class_ptr parent;
            ast::interface_list implements;
            if (!ancestors.empty()) {
                for (auto const& a : ancestors) {
                    ast::class_ptr c = ast::dynamic_type_cast<ast::class_>(a);
                    if (c) {
                        if (parent)
                            throw syntax_error(loc, "Class can have no more than one parent class");
                        parent = c;
                    } else {
                        ast::interface_ptr i = ast::dynamic_type_cast< ast::interface >(a);
                        if (!i) {
                            throw syntax_error(loc,
                                    "Interface can be derived from an interface only");
                        }
                        implements.push_back(i);
                    }
                }
            }
            ast::class_ptr c = scope()->add_type< ast::class_ >(identifier, parent, implements);
            state_.push_scope< class_scope >(c);
            break;
        }
        case rule::exception_name: {
            ast::exception_ptr parent;
            if (!ancestors.empty()) {
                if (ancestors.size() > 1) {
                    throw syntax_error(loc, "An exception can have no more than one ancestor");
                }
                parent = ast::dynamic_type_cast< ast::exception >(ancestors.front());
                if (!parent)
                    throw syntax_error(loc, "An exception can be derived from an exception only");
            }
            ast::exception_ptr e = scope()->add_type< ast::exception >(identifier, parent);
            state_.push_scope< exception_scope >(e);
            break;
        }
        default:
            break;
    }
}

void
parser_scope::close_scope(source_location const& loc)
{
    ::std::cerr << "// Close scope " << scope()->get_qualified_name() << "\n";
    state_.pop_scope(loc);
}

void
parser_scope::forward_declare(source_location const& loc, rule type, ::std::string const& identifier)
{
    throw syntax_error(loc, "Forward declaration is not implemented yet");
}

void
parser_scope::add_data_member(source_location const& loc,
        ast::type_ptr type, ::std::string const& identifier)
{
    if (ast::dynamic_entity_cast< ast::templated_type >(type)) {
        ::std::ostringstream os;
        os << "Cannot use template type " << type->get_qualified_name() << " without parameters";
        throw syntax_error(loc, os.str());
    }
    std::cerr << "// try add a data member " << type->get_qualified_name() << " " << identifier << "\n";
    add_data_member_impl(loc, type, identifier);
}

void
parser_scope::add_type_alias(source_location const& loc, ::std::string const& identifier,
        ast::type_ptr aliased_type)
{
    std::cerr << "// Add alias " << identifier << " for " << aliased_type->get_qualified_name() << "\n";
    scope()->add_type< ast::type_alias >( identifier, aliased_type);
}

//----------------------------------------------------------------------------
void
namespace_scope::start_namespace(source_location const& source_loc)
{
    set_current_phrase<block_name_parse>(rule::namespace_name);
}

void
namespace_scope::open_scope_impl(source_location const& loc, rule type,
        ::std::string const& identifier, ast::type_list const& ancestors)
{
    if (type == rule::namespace_name) {
        ast::namespace_ptr ns = scope< ast::namespace_ >()->add_namespace(identifier);
        state_.push_scope< namespace_scope >(ns);
    } else {
        parser_scope::open_scope_impl(loc, type, identifier, ancestors);
    }
}

//----------------------------------------------------------------------------
void
structure_scope::add_data_member_impl(source_location const& loc, ast::type_ptr type,
        ::std::string const& identifier)
{
    ast::structure_ptr st = scope< ast::structure >();
    st->add_data_member(identifier, type);
}

}  // namespace parser
}  // namespace idl
}  // namespace wire
