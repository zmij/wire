/*
 * identity_grammar_test.cpp
 *
 *  Created on: May 5, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <gtest/gtest.h>
#include <grammar/grammar_parse_test.hpp>

#include <wire/core/grammar/uuid_parse.hpp>
#include <wire/core/grammar/identity_parse.hpp>

namespace wire {
namespace core {
namespace test {

GRAMMAR_TEST(grammar::parse::uuid_grammar, UUID,
    ::testing::Values(
        "a8c834c8-b834-4f41-8f89-3f5fb9e7df00",
        "ba454543-29b8-4661-aee2-3727263e7575",
        "0e869225-5012-4d64-88d1-24aa2e1bd571",
        "88756f86-c877-46d6-abd7-971ed705a2eb"
    ),
    ::testing::Values(
        "a8c834c8b834-4f41-8f89-3f5fb9e7df00",
        "ba454543-29b84661-aee2-3727263e7575",
        "0e869225-50z2-4d64-88d1-24aa2e1bd571",
        "88756f86-c877-4xd6-abd7-971ed705a2eb"
    )
);

GRAMMAR_TEST(grammar::parse::identity_grammar, Identity,
    ::testing::Values(
        "a8c834c8-b834-4f41-8f89-3f5fb9e7df00",
        "test/ba454543-29b8-4661-aee2-3727263e7575",
        "foo:bar-foo",
        "foo:bar-foo/00.00",
        "foo/*"
    ),
    ::testing::Values(
        ":adfas",
        ".bla",
        "foo/*sd",
        "foo/bar*"
    )
);

}  /* namespace test */
}  /* namespace core */
}  /* namespace wire */

