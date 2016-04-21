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

//----------------------------------------------------------------------------
struct block_name_parse : phrase_parse {
    block_name_parse(parser_scope& sc, rule t)
        : phrase_parse(sc, t) {}
    virtual ~block_name_parse() {};

    phrase_parse_ptr
    process_token(source_location const& source_loc, token_value_type const& tkn) override
    {
        if (tkn.id() == lexer::token_whitespace)
            return shared_from_this();
        if (identifier.empty()) {
            if (tkn.id() != lexer::token_identifier)
                throw syntax_error(source_loc, "Identifier expected");
            identifier = ::std::string{tkn.value().begin(), tkn.value().end()};
        } else {
            // Expect a block open token or a statement end (for forward declaration)
            switch (tkn.id()) {
                case lexer::token_semicolon:
                    if (type == rule::namespace_name) {
                        throw syntax_error(source_loc, "Cannot forward declare a namespace");
                    }
                    // notify scope about forward declaration
                    scope.forward_declare(source_loc, type, identifier);
                    return phrase_parse_ptr{};
                case lexer::token_block_start:
                    // notify scope about scope open
                    scope.open_scope(source_loc, type, identifier);
                    if (type == rule::namespace_name)
                        return phrase_parse_ptr{}; // Don't want a semicolon here
                    return next_phrase<expect_semicolon_after_block>();
                case lexer::token_whitespace:
                    break;
                default:
                    throw syntax_error(source_loc, "Unexpected token (block name)");
            }
        }
        return shared_from_this();
    }

    ::std::string   identifier;
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
    using qname_set = ::std::function< void(qname_ptr) >;

    type_name_expect_scope(parser_scope& sc,
            ::std::string const& identifier, qname_set f)
        : phrase_parse(sc, rule::type_name),
          qname_{ ::std::make_shared<qname>( identifier ) },
          func{f} {}
    type_name_expect_scope(parser_scope& sc, qname_ptr qn, qname_set f)
        : phrase_parse(sc, rule::type_name),
          qname_(qn), func{f} {}

    virtual ~type_name_expect_scope() {}

    phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const& tkn) override
    {
        switch (tkn.id()) {
            case lexer::token_scope_resolution:
                return next_phrase< type_name_expect_identifier >(qname_, func);
            case lexer::token_whitespace:
                if (func)
                    func(qname_);
                break;
                // TODO angle brackets
            default:
                throw syntax_error(loc, "Unexpected token (type name)");
        }
        return phrase_parse_ptr{};
    }

    qname_ptr qname_;
    qname_set func;
};

struct type_name_expect_identifier : phrase_parse {
    using qname_ptr = ::std::shared_ptr<qname>;
    using qname_set = ::std::function< void(qname_ptr) >;

    type_name_expect_identifier(parser_scope& sc, qname_set f)
        : phrase_parse(sc, rule::type_name),
          qname_{ ::std::make_shared<qname>(true) },
          func{f} {}
    type_name_expect_identifier(parser_scope& sc,
            qname_ptr qn, qname_set f)
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
    qname_set func;
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
                current_phrase_ = next_phrase< type_name_expect_scope >(
                        ::std::string{ tkn.value().begin(), tkn.value().end() },
                        [&](qname_ptr qn) mutable { qname_ = qn; }
                );
                break;
            case lexer::token_scope_resolution:
                current_phrase_ = next_phrase< type_name_expect_identifier >(
                        [&](qname_ptr qn) mutable { qname_ = qn; });
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
            case lexer::token_scope_resolution:
                if (current_phrase_) {
                    current_phrase_ = current_phrase_->process_token(loc, tkn);
                } else {
                    if (!qname_)
                        current_phrase_ = next_phrase< type_name_expect_identifier >(
                                            [&](qname_ptr qn) mutable { qname_ = qn; });
                    else
                        throw syntax_error(loc, "Unexpected token");
                }
                break;
            case lexer::token_identifier:
                if (current_phrase_) {
                    current_phrase_ = current_phrase_->process_token(loc, tkn);
                } else {
                    if (!qname_) {
                        current_phrase_ = next_phrase< type_name_expect_scope >(
                                ::std::string{ tkn.value().begin(), tkn.value().end() },
                                [&](qname_ptr qn) mutable { qname_ = qn; }
                        );
                    } else {
                        if (!member_name_.empty())
                            throw syntax_error(loc, "Unexpected identifier (parse member)");
                        member_name_ = ::std::string{ tkn.value().begin(), tkn.value().end() };
                    }
                }
                break;
            case lexer::token_semicolon:
                // Notify owner about a member
                if (type == data) {
                    if (is_const_.is_initialized()) {
                        // Add a constant
                    } else {
                        // Add a data member
                        scope.add_data_member(loc, *qname_, member_name_);
                    }
                } else {

                }
                return phrase_parse_ptr{};
            case lexer::token_whitespace:
                if (current_phrase_) {
                    current_phrase_ = current_phrase_->process_token(loc, tkn);
                }
                break;
            default:
                throw syntax_error(loc, "Unexpected token (parse member)");
        }
        return shared_from_this();
    }

    member_type                 type = data;
    ::boost::optional< bool >   is_const_;
    phrase_parse_ptr            current_phrase_;
    ::std::shared_ptr<qname>    qname_;
    ::std::string               member_name_;
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
            if (current_phrase_.get()) {
                throw syntax_error(loc, "Unexpected namespace token");
            }
            start_namespace(loc);
            break;
        case lexer::token_struct:
            if (current_phrase_.get()) {
                throw syntax_error(loc, "Unexpected struct token");
            }
            ::std::cerr << "Structure";
            set_current_phrase< block_name_parse >(rule::structure_name);
            break;
        case lexer::token_class:
            if (current_phrase_.get()) {
                throw syntax_error(loc, "Unexpected class token");
            }
            ::std::cerr << "Class";
            set_current_phrase< block_name_parse >(rule::class_name);
            break;
        case lexer::token_interface:
            if (current_phrase_.get()) {
                throw syntax_error(loc, "Unexpected interface token");
            }
            ::std::cerr << "Interface";
            set_current_phrase< block_name_parse >(rule::interface_name);
            break;
        case lexer::token_exception:
            if (current_phrase_.get()) {
                throw syntax_error(loc, "Unexpected exception token");
            }
            ::std::cerr << "Exception";
            set_current_phrase< block_name_parse >(rule::exception_name);
            break;
        case lexer::token_const:
            ::std::cerr << "const";
            if (current_phrase_) {
                current_phrase_ = current_phrase_->process_token(loc, tkn);
            } else {
                set_current_phrase< member_decl >( tkn );
            }
            break;
        case lexer::token_using:
            ::std::cerr << "using";
            break;

        case lexer::token_comma:
            ::std::cerr << ",";
            break;
        case lexer::token_colon:
            ::std::cerr << ":";
            break;
        case lexer::token_scope_resolution:
            ::std::cerr << ":SCOPE:";
            if (current_phrase_) {
                current_phrase_ = current_phrase_->process_token(loc, tkn);
            } else {
                set_current_phrase< member_decl >( tkn );
            }
             break;
        case lexer::token_semicolon:
            ::std::cerr << ";";
            if (!current_phrase_) {
                throw syntax_error(loc, "Unexpected statement end");
            }
            current_phrase_ = current_phrase_->process_token(loc, tkn);
            break;
        case lexer::token_assign:
            ::std::cerr << "=";
            break;
        case lexer::token_asterisk:
            ::std::cerr << "*";
            break;
        case lexer::token_brace_open:
            ::std::cerr << "(";
            break;
        case lexer::token_brace_close:
            ::std::cerr << ")";
            break;
        case lexer::token_block_start:
            if (!current_phrase_) {
                throw syntax_error(loc, "Unexpected block start");
            }
            current_phrase_ = current_phrase_->process_token(loc, tkn);
            break;
        case lexer::token_block_end:
            if (current_phrase_) {
                throw syntax_error(loc, "Unexpected block end");
            }
            ::std::cerr << "}";
            close_scope(loc);
            break;
        case lexer::token_angled_open:
            ::std::cerr << "<";
            break;
        case lexer::token_angled_close:
            ::std::cerr << ">";
            break;
        case lexer::token_attrib_start:
            ::std::cerr << "[[";
            break;
        case lexer::token_attrib_end:
            ::std::cerr << "]]";
            break;

        case lexer::token_identifier:
            ::std::cerr << "Identifier("
                  << ::std::string{ tkn.value().begin(), tkn.value().end() } << ")";
            if (current_phrase_) {
                current_phrase_ = current_phrase_->process_token(loc, tkn);
            } else {
                set_current_phrase< member_decl >( tkn );
            }
            break;
        case lexer::token_number:
            ::std::cerr << "NUMBER";
            break;
        case lexer::token_oct_number:
            ::std::cerr << "OCTAL";
            break;
        case lexer::token_hex_number:
            ::std::cerr << "HEX";
            break;
        case lexer::token_quoted_string:
            ::std::cerr << "\"string literal\"";
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
            break;
        case lexer::token_whitespace:
            ::std::cerr << *tkn.value().begin();
            if (current_phrase_) {
                current_phrase_ = current_phrase_->process_token(loc, tkn);
            }
            break;
        default:
            ::std::cerr << "{_|_}";
            break;
    }
}

void
parser_scope::open_scope(source_location const& loc, rule type, ::std::string const& identifier)
{
    ::std::cerr << "// Open scope " << identifier << "\n";
    open_scope_impl(loc, type, identifier);
}

void
parser_scope::open_scope_impl(source_location const& loc, rule type, ::std::string const& identifier)
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
            ast::interface_ptr i = scope()->add_type< ast::interface >(identifier);
            state_.push_scope< interface_scope >(i);
            break;
        }
        case rule::class_name: {
            ast::class_ptr c = scope()->add_type< ast::class_ >(identifier);
            state_.push_scope< class_scope >(c);
            break;
        }
        case rule::exception_name: {
            ast::exception_ptr e = scope()->add_type< ast::exception >(identifier);
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
        qname const& type, ::std::string const& identifier)
{
    std::cerr << "// try add a data member " << type << " " << identifier << "\n";
    add_data_member_impl(loc, type, identifier);
}

//----------------------------------------------------------------------------
void
namespace_scope::start_namespace(source_location const& source_loc)
{
    set_current_phrase<block_name_parse>(rule::namespace_name);
}

void
namespace_scope::open_scope_impl(source_location const& loc, rule type, ::std::string const& identifier)
{
    if (type == rule::namespace_name) {
        ast::namespace_ptr ns = scope< ast::namespace_ >()->add_namespace(identifier);
        state_.push_scope< namespace_scope >(ns);
    } else {
        parser_scope::open_scope_impl(loc, type, identifier);
    }
}

//----------------------------------------------------------------------------
void
structure_scope::add_data_member_impl(source_location const& loc, qname const& type,
        ::std::string const& identifier)
{
    ast::structure_ptr st = scope< ast::structure >();
    ast::type_ptr t = st->find_type(type);
    if (!t) {
        ::std::ostringstream os;
        os << "Data type " << type << " not found";
        throw syntax_error(loc, os.str());
    }
    st->add_data_member(identifier, t);
}

}  // namespace parser
}  // namespace idl
}  // namespace wire
