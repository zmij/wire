/*
 * ast_test.cpp
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/idl/ast.hpp>
#include <wire/idl/qname.hpp>

namespace wire {
namespace idl {
namespace ast {
namespace test {

TEST(AST, QualifiedName)
{
    qname qn{"a"};
    EXPECT_FALSE(qn.fully);
    EXPECT_EQ(1, qn.size());
    EXPECT_EQ("a", qn.name());

    qname_search s = qn.search();
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(1, s.size());
    s = s.next();
    EXPECT_FALSE(s.fully);
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(0, s.size());

    qn = qname{"::A"};
    EXPECT_TRUE(qn.fully);
    EXPECT_EQ(1, qn.size());
    EXPECT_EQ("A", qn.name());

    qn = qname{"::N::NS::_bla::A"};
    EXPECT_TRUE(qn.fully);
    EXPECT_EQ(4, qn.size());
    EXPECT_EQ("A", qn.name());

    s = qn.search();
    EXPECT_TRUE(s.fully);
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(4, s.size());

    s = s.scope();
    EXPECT_TRUE(s.fully);
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(3, s.size());

    s = s.next();
    EXPECT_FALSE(s.fully);
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(2, s.size());
}

TEST(AST, BuiltinTypes)
{
    namespace_::clear_global();
    auto glob = namespace_::global();

    ASSERT_TRUE(glob->types().empty());
    ASSERT_TRUE(glob->nested().empty());

    EXPECT_TRUE(type::is_built_in("void"));
    EXPECT_TRUE(type::is_built_in("bool"));
    EXPECT_TRUE(type::is_built_in("char"));
    EXPECT_TRUE(type::is_built_in("byte"));
    EXPECT_TRUE(type::is_built_in("int32"));
    EXPECT_TRUE(type::is_built_in("int64"));
    EXPECT_TRUE(type::is_built_in("octet"));
    EXPECT_TRUE(type::is_built_in("uint32"));
    EXPECT_TRUE(type::is_built_in("uint64"));
    EXPECT_TRUE(type::is_built_in("float"));
    EXPECT_TRUE(type::is_built_in("double"));
    EXPECT_TRUE(type::is_built_in("string"));
    EXPECT_TRUE(type::is_built_in("uuid"));

    EXPECT_TRUE(type::is_built_in("variant"));
    EXPECT_TRUE(type::is_built_in("sequence"));
    EXPECT_TRUE(type::is_built_in("array"));
    EXPECT_TRUE(type::is_built_in("dictionary"));
    EXPECT_TRUE(type::is_built_in("optional"));

    EXPECT_FALSE(type::is_built_in("vector"));
    EXPECT_FALSE(type::is_built_in("int"));
    EXPECT_FALSE(type::is_built_in("list"));

    EXPECT_TRUE(glob->find_type("void").get());
    EXPECT_TRUE(glob->find_type("bool").get());
    EXPECT_TRUE(glob->find_type("char").get());
    EXPECT_TRUE(glob->find_type("byte").get());
    EXPECT_TRUE(glob->find_type("int32").get());
    EXPECT_TRUE(glob->find_type("int64").get());
    EXPECT_TRUE(glob->find_type("octet").get());
    EXPECT_TRUE(glob->find_type("uint32").get());
    EXPECT_TRUE(glob->find_type("uint64").get());
    EXPECT_TRUE(glob->find_type("float").get());
    EXPECT_TRUE(glob->find_type("double").get());
    EXPECT_TRUE(glob->find_type("string").get());
    EXPECT_TRUE(glob->find_type("uuid").get());

    EXPECT_TRUE(glob->find_type("variant").get());
    EXPECT_TRUE(glob->find_type("sequence").get());
    EXPECT_TRUE(glob->find_type("array").get());
    EXPECT_TRUE(glob->find_type("dictionary").get());
    EXPECT_TRUE(glob->find_type("optional").get());

    EXPECT_TRUE(glob->find_entity("void").get());
    EXPECT_TRUE(glob->find_entity("bool").get());
    EXPECT_TRUE(glob->find_entity("char").get());
    EXPECT_TRUE(glob->find_entity("byte").get());
    EXPECT_TRUE(glob->find_entity("int32").get());
    EXPECT_TRUE(glob->find_entity("int64").get());
    EXPECT_TRUE(glob->find_entity("octet").get());
    EXPECT_TRUE(glob->find_entity("uint32").get());
    EXPECT_TRUE(glob->find_entity("uint64").get());
    EXPECT_TRUE(glob->find_entity("float").get());
    EXPECT_TRUE(glob->find_entity("double").get());
    EXPECT_TRUE(glob->find_entity("string").get());
    EXPECT_TRUE(glob->find_entity("uuid").get());

    EXPECT_TRUE(glob->find_entity("variant").get());
    EXPECT_TRUE(glob->find_entity("sequence").get());
    EXPECT_TRUE(glob->find_entity("array").get());
    EXPECT_TRUE(glob->find_entity("dictionary").get());
    EXPECT_TRUE(glob->find_entity("optional").get());
}

TEST(AST, Namespaces)
{
    namespace_::clear_global();
    auto foobar = namespace_::global()->add_namespace(0, "foo::bar");
    EXPECT_TRUE(foobar.get());
    auto foo = namespace_::global()->find_entity<namespace_>(qname{"foo"});
    EXPECT_TRUE(foo.get());
    EXPECT_NO_THROW(foo->find_entity("foo"));
    EXPECT_NO_THROW(foo->find_entity("::foo"));

    EXPECT_TRUE(foo->find_entity("foo").get());
    EXPECT_TRUE(foo->find_entity("::foo").get());

    EXPECT_TRUE(foo->find_namespace("foo").get());
    EXPECT_TRUE(foo->find_namespace("::foo").get());

    EXPECT_EQ(foo, foo->find_entity<namespace_>("::foo"));
    EXPECT_EQ(foo, foobar->find_entity<namespace_>("::foo"));
    EXPECT_EQ(foobar, foo->find_entity<namespace_>("bar"));
}

TEST(AST, ScopeLookup)
{
    /*
     * Create ast
     * namespace outer {
     * namespace inner {
     *
     * struct out_struct {
     *     struct in_struct {
     *     };
     * };
     *
     * }
     * }
     */
    namespace_::clear_global();
    auto glob = namespace_::global();

    ASSERT_TRUE(glob->types().empty());
    ASSERT_TRUE(glob->nested().empty());

    auto inner_ns = glob->add_namespace(0, "outer::inner");
    auto outer_ns = glob->find_namespace("outer");

    structure_ptr out = inner_ns->add_type< structure >(1, "out_struct");
    out->add_type< structure >(2, "in_struct");

    EXPECT_TRUE(glob->find_scope("outer").first.get());
    EXPECT_TRUE(glob->find_scope("outer::inner").first.get());
    EXPECT_TRUE(glob->find_scope("outer::inner::out_struct").first.get());
    EXPECT_TRUE(glob->find_scope("outer::inner::out_struct::in_struct").first.get());

    EXPECT_FALSE(glob->find_scope("inner").first);
    EXPECT_FALSE(glob->find_scope("out_struct").first);
    EXPECT_FALSE(glob->find_scope("in_struct").first);

    EXPECT_TRUE(inner_ns->find_scope("outer").first.get());
    EXPECT_TRUE(inner_ns->find_scope("inner").first.get());
    EXPECT_TRUE(inner_ns->find_scope("outer::inner").first.get());
    EXPECT_TRUE(inner_ns->find_scope("out_struct").first.get());
    EXPECT_TRUE(inner_ns->find_scope("out_struct::in_struct").first.get());
    EXPECT_TRUE(inner_ns->find_scope("inner::out_struct::in_struct").first.get());
    EXPECT_TRUE(inner_ns->find_scope("outer::inner::out_struct::in_struct").first.get());
    EXPECT_TRUE(inner_ns->find_scope("::outer::inner::out_struct::in_struct").first.get());
    EXPECT_FALSE(inner_ns->find_scope("in_struct").first);
}

TEST(AST, ScopeOfLookup)
{
    /*
     * Create structure
     * namespace outer {
     * namespace inner {
     *
     * struct out_struct {
     *     struct in_struct {
     *     };
     * };
     *
     * }
     * }
     */
    namespace_::clear_global();
    auto glob = namespace_::global();
    ASSERT_TRUE(glob->types().empty());
    ASSERT_TRUE(glob->nested().empty());

    auto ns = glob->add_namespace(0, "outer::inner");

    structure_ptr out = ns->add_type< structure >(1, "out_struct");
    out->add_type< structure >(2, "in_struct");

    EXPECT_TRUE(glob->find_scope_of("outer").get());
    EXPECT_TRUE(glob->find_scope_of("outer::inner").get());
    EXPECT_TRUE(glob->find_scope_of("outer::inner::out_struct").get());
    EXPECT_TRUE(glob->find_scope_of("outer::inner::out_struct::in_struct").get());
}

TEST(AST, EntityLookup)
{
    /*
     * Create ast
     * namespace outer {
     * namespace inner {
     *
     * struct out_struct {
     *     struct in_struct {
     *     };
     * };
     *
     * interface out_interface {
     *     struct internal {
     *     };
     *     void
     *     func();
     * };
     *
     * interface derived : out_interface {
     * };
     *
     * }
     * }
     */
    namespace_::clear_global();
    auto glob = namespace_::global();

    ASSERT_TRUE(glob->types().empty());
    ASSERT_TRUE(glob->nested().empty());

    auto inner_ns = glob->add_namespace(0, "outer::inner");
    auto outer_ns = glob->find_namespace("outer");

    structure_ptr out = inner_ns->add_type< structure >(1, "out_struct");
    out->add_type< structure >(2, "in_struct");

    interface_ptr iface = inner_ns->add_type< interface >(3, "out_iterface" );
    iface->add_function(4, "func");
    structure_ptr intl = iface->add_type< structure >(5, "internal");
    interface_ptr dface = inner_ns->add_type< interface >(6, "derived", interface_list{ iface } );

    EXPECT_TRUE(glob->find_entity("outer::inner::out_struct").get());
    EXPECT_TRUE(glob->find_entity("::outer::inner::out_struct").get());
    EXPECT_TRUE(glob->find_entity("outer::inner::out_struct::in_struct").get());
    EXPECT_TRUE(glob->find_entity("::outer::inner::out_struct::in_struct").get());

    EXPECT_TRUE(inner_ns->find_entity("void").get());
    EXPECT_TRUE(inner_ns->find_entity("out_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("inner::out_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("outer::inner::out_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("::outer::inner::out_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("out_struct::in_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("inner::out_struct::in_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("outer::inner::out_struct::in_struct").get());
    EXPECT_TRUE(inner_ns->find_entity("::outer::inner::out_struct::in_struct").get());

    EXPECT_FALSE(inner_ns->find_entity("__not_there_"));
    EXPECT_FALSE(inner_ns->find_entity("in_struct"));

    EXPECT_TRUE(outer_ns->find_entity("void").get());
    EXPECT_FALSE(outer_ns->find_entity("out_struct").get());
    EXPECT_TRUE(outer_ns->find_entity("inner::out_struct").get());
    EXPECT_TRUE(outer_ns->find_entity("outer::inner::out_struct").get());
    EXPECT_TRUE(outer_ns->find_entity("::outer::inner::out_struct").get());
    EXPECT_FALSE(outer_ns->find_entity("out_struct::in_struct").get());
    EXPECT_TRUE(outer_ns->find_entity("inner::out_struct::in_struct").get());
    EXPECT_TRUE(outer_ns->find_entity("outer::inner::out_struct::in_struct").get());
    EXPECT_TRUE(outer_ns->find_entity("::outer::inner::out_struct::in_struct").get());

    EXPECT_FALSE(outer_ns->find_entity("__not_there_"));
    EXPECT_FALSE(outer_ns->find_entity("in_struct"));

    EXPECT_TRUE(glob->find_entity("outer::inner::out_iterface").get());
    EXPECT_TRUE(glob->find_entity("outer::inner::out_iterface::internal").get());

    ASSERT_TRUE(iface.get());
    ASSERT_TRUE(intl.get());
    ASSERT_TRUE(dface.get());
    EXPECT_TRUE(iface->find_entity("internal").get());
    EXPECT_TRUE(iface->find_entity("func").get());
    EXPECT_TRUE(glob->find_entity("outer::inner::derived").get());
    EXPECT_TRUE(dface->find_entity("internal").get());
    EXPECT_TRUE(dface->find_entity("func").get());
    EXPECT_TRUE(glob->find_entity("outer::inner::derived::internal").get());

    EXPECT_FALSE(inner_ns->find_entity("::outer::inner::out_struct::__not_there_").get());
    EXPECT_FALSE(inner_ns->find_entity("outer::inner::out_struct::__not_there_").get());
    EXPECT_FALSE(glob->find_entity("outer::inner::derived::__not_there_").get());
}


TEST(AST, TypeLookup)
{
    /*
     * Create ast
     * namespace outer {
     * namespace inner {
     *
     * struct out_struct {
     *     struct in_struct {
     *     };
     * };
     *
     * interface out_interface {
     *     struct internal {
     *     };
     * };
     *
     * interface derived : out_interface {
     * };
     *
     * class base : derived {
     * };
     *
     * class child : derived {
     *     class sneaky {
     *     };
     * };
     *
     * using child_alias = child;
     *
     * }
     * }
     */
    namespace_::clear_global();
    auto glob = namespace_::global();

    ASSERT_TRUE(glob->types().empty());
    ASSERT_TRUE(glob->nested().empty());

    auto inner_ns = glob->add_namespace(0, "outer::inner");
    auto outer_ns = glob->find_namespace("outer");

    structure_ptr out = inner_ns->add_type< structure >(1, "out_struct");
    out->add_type< structure >(2, "in_struct");

    interface_ptr iface = inner_ns->add_type< interface >(3, "out_iterface" );
    structure_ptr intl = iface->add_type< structure >(4, "internal");
    interface_ptr dface = inner_ns->add_type< interface >(5, "derived", interface_list{ iface } );

    class_ptr base = inner_ns->add_type<class_>(6, "base", class_ptr{}, interface_list{ dface });
    class_ptr child = inner_ns->add_type<class_>(7, "child", base);
    child->add_type<class_>(8, "sneaky");
    inner_ns->add_type< type_alias >(9, "child_alias", child);

    EXPECT_TRUE(glob->find_type("outer::inner::out_struct").get());
    EXPECT_TRUE(glob->find_type("::outer::inner::out_struct").get());
    EXPECT_TRUE(glob->find_type("outer::inner::out_struct::in_struct").get());
    EXPECT_TRUE(glob->find_type("::outer::inner::out_struct::in_struct").get());

    EXPECT_TRUE(inner_ns->find_type("void").get());
    EXPECT_TRUE(inner_ns->find_type("out_struct").get());
    EXPECT_TRUE(inner_ns->find_type("inner::out_struct").get());
    EXPECT_TRUE(inner_ns->find_type("outer::inner::out_struct").get());
    EXPECT_TRUE(inner_ns->find_type("::outer::inner::out_struct").get());
    EXPECT_TRUE(inner_ns->find_type("out_struct::in_struct").get());
    EXPECT_TRUE(inner_ns->find_type("inner::out_struct::in_struct").get());
    EXPECT_TRUE(inner_ns->find_type("outer::inner::out_struct::in_struct").get());
    EXPECT_TRUE(inner_ns->find_type("::outer::inner::out_struct::in_struct").get());

    EXPECT_FALSE(inner_ns->find_type("__not_there_"));
    EXPECT_FALSE(inner_ns->find_type("in_struct"));

    EXPECT_TRUE(outer_ns->find_type("void").get());
    EXPECT_FALSE(outer_ns->find_type("out_struct").get());
    EXPECT_TRUE(outer_ns->find_type("inner::out_struct").get());
    EXPECT_TRUE(outer_ns->find_type("outer::inner::out_struct").get());
    EXPECT_TRUE(outer_ns->find_type("::outer::inner::out_struct").get());
    EXPECT_FALSE(outer_ns->find_type("out_struct::in_struct").get());
    EXPECT_TRUE(outer_ns->find_type("inner::out_struct::in_struct").get());
    EXPECT_TRUE(outer_ns->find_type("outer::inner::out_struct::in_struct").get());
    EXPECT_TRUE(outer_ns->find_type("::outer::inner::out_struct::in_struct").get());

    EXPECT_FALSE(outer_ns->find_type("__not_there_"));
    EXPECT_FALSE(outer_ns->find_type("in_struct"));

    EXPECT_TRUE(glob->find_type("outer::inner::out_iterface").get());
    EXPECT_TRUE(glob->find_type("outer::inner::out_iterface::internal").get());

    ASSERT_TRUE(iface.get());
    ASSERT_TRUE(intl.get());
    ASSERT_TRUE(dface.get());
    EXPECT_TRUE(iface->find_type("internal").get());
    EXPECT_TRUE(glob->find_type("outer::inner::derived").get());
    EXPECT_TRUE(dface->find_type("internal").get());
    EXPECT_TRUE(glob->find_type("outer::inner::derived::internal").get());

    EXPECT_FALSE(inner_ns->find_type("::outer::inner::out_struct::__not_there_").get());
    EXPECT_FALSE(inner_ns->find_type("outer::inner::out_struct::__not_there_").get());
    EXPECT_FALSE(glob->find_type("outer::inner::derived::__not_there_").get());

    ASSERT_TRUE(base.get());
    ASSERT_TRUE(child.get());

    EXPECT_TRUE(inner_ns->find_type("base").get());
    EXPECT_TRUE(inner_ns->find_type("child").get());
    EXPECT_TRUE(glob->find_type("outer::inner::base::internal").get());
    EXPECT_TRUE(glob->find_type("outer::inner::child::internal").get());
    EXPECT_FALSE(glob->find_type("outer::inner::base::__not_there_").get());
    EXPECT_TRUE(glob->find_type("outer::inner::child::sneaky").get());
    EXPECT_FALSE(glob->find_type("outer::inner::base::sneaky").get());

    EXPECT_TRUE(inner_ns->find_type("child_alias").get());
    EXPECT_TRUE(glob->find_type("outer::inner::child_alias").get());
    EXPECT_TRUE(glob->find_type("outer::inner::child_alias::internal").get());
}

}  /* namespace test */
}  /* namespace ast */
}  /* namespace idl */
}  /* namespace wire */
