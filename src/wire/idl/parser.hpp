/*
 * parser.hpp
 *
 *  Created on: Apr 20, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_PARSER_HPP_
#define WIRE_IDL_PARSER_HPP_

#include <wire/idl/ast.hpp>
#include <wire/idl/token_types.hpp>
#include <wire/idl/syntax_error.hpp>

#include <deque>

namespace wire {
namespace idl {
namespace parser {

struct parser_scope;

struct parser_state {
    using token_value_type = lexer::token<>;
    using lexer_type =
        ::boost::spirit::lex::lexertl::actor_lexer< token_value_type >;

    parser_state();

    void
    process_token(source_location const& source_loc, token_value_type const& tkn);

    parser_scope&
    current_scope();

    template < typename Scope, typename ... Args >
    void
    push_scope(Args&& ... args)
    {
        stack_.push_back(::std::make_shared< Scope >(*this, std::forward<Args>(args) ...));
    }
    void
    pop_scope(source_location const& loc);
private:
    using scope_ptr = ::std::shared_ptr< parser_scope >;
    using scope_stack = ::std::deque< scope_ptr >;

    scope_stack stack_;
};

//----------------------------------------------------------------------------
enum class rule {
    skip_whitespace,
    namespace_name,
    structure_name,
    interface_name,
    class_name,
    exception_name,
    identifier,
    semicolon_after_block,
    member_declaration,
    type_name,
    template_args,
    type_alias,

};
struct phrase_parse : ::std::enable_shared_from_this<phrase_parse> {
    using token_value_type = parser_state::token_value_type;
    using phrase_parse_ptr = ::std::shared_ptr< phrase_parse >;

    phrase_parse(parser_scope& sc, rule t) : scope(sc), type(t) {}
    virtual ~phrase_parse() {}
    virtual phrase_parse_ptr
    process_token(source_location const& loc, token_value_type const&) = 0;
    virtual bool
    want_token(source_location const& loc, token_value_type const&) const
    { return true; }

    template < typename T, typename ... Args >
    phrase_parse_ptr
    next_phrase( Args&& ... args )
    {
        static_assert(::std::is_base_of<phrase_parse, T>::value,
                "Next phrase must descend from phrase_parse type");
        return ::std::make_shared< T >(scope, ::std::forward<Args>(args) ...);
    }

    parser_scope& scope;
    rule        type;
};
using phrase_parse_ptr = ::std::shared_ptr< phrase_parse >;

struct phrase_parser {
    using token_value_type = parser_state::token_value_type;

    operator bool() const
    {
        return current_phrase_.get();
    }

    bool
    process_token(source_location const& loc, token_value_type const& tkn)
    {
        if (current_phrase_) {
            if (current_phrase_->want_token(loc, tkn)) {
                current_phrase_ = current_phrase_->process_token(loc, tkn);
                return true;
            }
            current_phrase_.reset();
        }
        return false;
    }
    template < typename T, typename ... Args >
    void
    set_current_phrase(Args&& ... args)
    {
        current_phrase_ = ::std::make_shared<T>(::std::forward<Args>(args)...);
    }

    phrase_parse_ptr current_phrase_;
};

//----------------------------------------------------------------------------
class parser_scope {
public:
    using token_value_type = parser_state::token_value_type;

    parser_scope( parser_state& ps, ast::scope_ptr sc )
        : state_(ps), scope_(sc) {}
    virtual ~parser_scope() {}

    ast::scope_ptr
    scope() const
    { return scope_; }
    template < typename T >
    ast::shared_entity< T >
    scope()
    { return ast::dynamic_entity_cast< T >(scope_); }

    void
    process_token(source_location const& loc, token_value_type const& tkn);

    void
    open_scope(source_location const&, rule type,
            ::std::string const& identifier, ast::type_list const&);
    void
    close_scope(source_location const& loc);
    void
    forward_declare(source_location const&, rule type,
            ::std::string const& idetifier);

    void
    add_type_alias(source_location const& loc, ::std::string const& identifier,
            ast::type_ptr aliased_type);
    void
    add_data_member(source_location const& loc,
            ast::type_ptr type, ::std::string const& identifier); // TODO default value
protected:
    template < typename T, typename ... Args >
    void
    set_current_phrase(Args&& ... args)
    {
        current_phrase.set_current_phrase<T>(*this, ::std::forward<Args>(args)...);
    }
    virtual void
    open_scope_impl(source_location const&, rule type,
            ::std::string const& identifier, ast::type_list const&);
private:
    virtual void
    start_namespace(source_location const& loc)
    {
        throw syntax_error(loc, "Cannot nest a namespace at this scope");
    }
    virtual void
    add_data_member_impl(source_location const& loc, ast::type_ptr type,
            ::std::string const& identifier)
    {
        throw syntax_error(loc, "Cannot add a data member at this scope");
    }
protected:
    parser_state&     state_;
    ast::scope_ptr    scope_;
    phrase_parser     current_phrase;
};

//----------------------------------------------------------------------------
class namespace_scope : public parser_scope {
public:
    namespace_scope( parser_state& ps, ast::namespace_ptr ns )
        : parser_scope(ps, ns) {}
    virtual ~namespace_scope() {}

private:
    void
    start_namespace(source_location const& source_loc) override;
    void
    open_scope_impl(source_location const&, rule type,
            ::std::string const& identifier, ast::type_list const&) override;
};

//----------------------------------------------------------------------------
class structure_scope : public virtual parser_scope {
public:
    structure_scope( parser_state& ps, ast::structure_ptr s)
        : parser_scope(ps, s) {}
    virtual ~structure_scope() {}
protected:
    void
    add_data_member_impl(source_location const& loc, ast::type_ptr type,
            ::std::string const& identifier) override;
};

//----------------------------------------------------------------------------
class interface_scope : public virtual parser_scope {
public:
    interface_scope( parser_state& ps, ast::interface_ptr s)
        : parser_scope(ps, s) {}
    virtual ~interface_scope() {}
};

//----------------------------------------------------------------------------
class class_scope : public structure_scope, public interface_scope {
public:
    class_scope( parser_state& ps, ast::class_ptr s)
        : parser_scope(ps, s),
          structure_scope(ps, s),
          interface_scope(ps, s) {}
    virtual ~class_scope() {}
};

//----------------------------------------------------------------------------
class exception_scope : public structure_scope {
public:
    exception_scope( parser_state& ps, ast::exception_ptr s)
        : parser_scope(ps, s), structure_scope(ps, s) {}
    virtual ~exception_scope() {}
};


}  // namespace parser
}  // namespace idl
}  // namespace wire

#endif /* WIRE_IDL_PARSER_HPP_ */
