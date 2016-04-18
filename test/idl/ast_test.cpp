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
namespace test {

TEST(AST, QualifiedName)
{
    qname qn{"a"};
    EXPECT_FALSE(qn.fully);
    EXPECT_EQ(1, qn.size());
    EXPECT_EQ("a", qn.name());

    qn = qname{"::A"};
    EXPECT_TRUE(qn.fully);
    EXPECT_EQ(1, qn.size());
    EXPECT_EQ("A", qn.name());

    qn = qname{"::N::NS::_bla::A"};
    EXPECT_TRUE(qn.fully);
    EXPECT_EQ(4, qn.size());
    EXPECT_EQ("A", qn.name());
}

TEST(AST, BuiltinTypes)
{
    EXPECT_TRUE(type::is_biult_in("bool"));
    EXPECT_TRUE(type::is_biult_in("char"));
    EXPECT_TRUE(type::is_biult_in("byte"));
    EXPECT_TRUE(type::is_biult_in("int32"));
    EXPECT_TRUE(type::is_biult_in("int64"));
    EXPECT_TRUE(type::is_biult_in("octet"));
    EXPECT_TRUE(type::is_biult_in("uint32"));
    EXPECT_TRUE(type::is_biult_in("uint64"));
    EXPECT_TRUE(type::is_biult_in("float"));
    EXPECT_TRUE(type::is_biult_in("double"));
    EXPECT_TRUE(type::is_biult_in("string"));
    EXPECT_TRUE(type::is_biult_in("uuid"));

    EXPECT_TRUE(type::is_biult_in("variant"));
    EXPECT_TRUE(type::is_biult_in("sequence"));
    EXPECT_TRUE(type::is_biult_in("array"));
    EXPECT_TRUE(type::is_biult_in("dictionary"));
    EXPECT_TRUE(type::is_biult_in("optional"));

    EXPECT_FALSE(type::is_biult_in("vector"));
    EXPECT_FALSE(type::is_biult_in("int"));
    EXPECT_FALSE(type::is_biult_in("list"));
}

TEST(AST, Namespaces)
{
    auto foobar = namespace_::global()->add_namespace("foo::bar");
    EXPECT_TRUE(foobar);
    auto foo = namespace_::global()->find_name<namespace_>(qname{"foo"});
    EXPECT_TRUE(foo);
    EXPECT_THROW(foo->find_name("foo"), ::std::runtime_error);
    EXPECT_NO_THROW(foo->find_name("::foo"));
    EXPECT_EQ(foo, foo->find_name<namespace_>("::foo"));
    EXPECT_EQ(foo, foobar->find_name<namespace_>("::foo"));
    EXPECT_EQ(foobar, foo->find_name<namespace_>("bar"));
}

TEST(AST, TypeAlias)
{

}


} // namespace test
}  /* namespace idl */
}  /* namespace wire */
