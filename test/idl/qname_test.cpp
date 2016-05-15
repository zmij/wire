/*
 * qname_test.cpp
 *
 *  Created on: 19 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <grammar/grammar_parse_test.hpp>
#include <wire/idl/qname_grammar.hpp>

namespace wire {
namespace idl {
namespace ast {
namespace test {

GRAMMAR_TEST(parse::qualified_name_grammar, QualifiedName,
    ::testing::Values("a", "::a", "::a::b"),
    ::testing::Values("", "::1", "::23::b")
);


}  /* namespace test */
}  /* namespace ast */
}  /* namespace idl */
}  /* namespace wire */
