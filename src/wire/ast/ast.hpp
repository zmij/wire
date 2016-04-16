/*
 * ast.hpp
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_AST_AST_HPP_
#define WIRE_AST_AST_HPP_

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <iosfwd>

namespace wire {
namespace ast {

class entity;
typedef ::std::shared_ptr< entity > entity_ptr;

class scope;
typedef ::std::shared_ptr<scope> scope_ptr;
typedef ::std::weak_ptr<scope> scope_weak_ptr;

class name_space;
typedef ::std::shared_ptr< name_space > namespace_ptr;
typedef ::std::vector< namespace_ptr > namespace_list;

class location {
    ::std::string file;
    ::std::size_t line;
};

class entity {
public:
    ::std::string const&
    name() const
    { return name_; }

    scope_ptr
    scope() const
    { return scope_.lock(); }

    std::string
    qualified_name() const;

    void
    get_qualified_name(::std::ostream& os) const;

    struct name_compare {
        bool
        operator () (entity const& lhs, entity const& rhs) const
        {
            return lhs.name() < rhs.name();
        }

        bool
        operator () (entity_ptr const& lhs, entity_ptr const& rhs) const
        {
            return lhs->name() < rhs->name();
        }
    };
protected:
    /**
     * Create an entity in the global scope. Can be used for global namespace
     * only.
     */
    entity();
    /**
     * Create an entity in the given scope
     * @param sc
     * @param name
     */
    entity(scope_ptr sc, ::std::string const& name);
private:
    scope_weak_ptr    scope_;
    ::std::string    name_;
};

typedef ::std::set<entity_ptr, entity::name_compare> entity_set;

struct type : virtual entity {
};
typedef ::std::shared_ptr< type >     type_ptr;
typedef ::std::vector< type_ptr >    type_list;

class constant : public entity {
public:
    constant( scope_ptr sc, ::std::string const& type,
            ::std::string const& name, ::std::string const& literal);
private:
    type_ptr        type_;
    ::std::string    literal_;
};

typedef ::std::vector< constant >    constant_list;

class scope : public virtual entity {
public:
    void
    add_type(type_ptr);
    type_ptr
    find_type(::std::string const& name) const;

    void
    add_constant(constant&& c);

    type_list const&
    types() const
    { return types_; }

    constant_list const&
    constants() const
    { return constants_; }
protected:
    type_list        types_;
    constant_list    constants_;
};

class name_space : public scope {
public:
    static name_space&
    global();

    static void
    clear_global();
public:
    void
    add_namespace(namespace_ptr ns);

    namespace_list const&
    nested() const
    { return nested_; }
private:
    namespace_list    nested_;
};

}  // namespace ast
}  // namespace wire



#endif /* WIRE_AST_AST_HPP_ */
