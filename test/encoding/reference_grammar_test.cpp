/*
 * reference_grammar_test.cpp
 *
 *  Created on: May 5, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <grammar/grammar_parse_test.hpp>

#include <wire/core/grammar/reference_parse.hpp>

namespace wire {
namespace core {
namespace test {

GRAMMAR_TEST(grammar::parse::reference_grammar, Reference,
    ::testing::Values(
        "foo tcp://localhost:5432",
        "foo@bar",
        "test/ba454543-29b8-4661-aee2-3727263e7575[v1] socket:///tmp/.wire",
        "test/ba454543-29b8-4661-aee2-3727263e7575[v1]@adapters/ba454543-29b84661-aee2-3727263e7575"
    ),
    ::testing::Values(
        "tcp://localhost:5432",
        "foo",
        "",
        "a8c834c8-b834-4f41-8f89-3f5fb9e7df00",
        "test/ba454543-29b8-4661-aee2-3727263e7575[v1]"
    )
);

}  /* namespace test */
}  /* namespace core */
}  /* namespace wire */
