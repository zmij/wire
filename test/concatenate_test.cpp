/*
 * concatenate_test.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/util/concatenate.hpp>

namespace wire {
namespace util {
namespace test {

TEST(Util, Concatenate)
{
	std::string c = concatenate("a", "b", 10, 3.14, "blabla");
	EXPECT_EQ("ab103.14blabla", c);
}

}  // namespace test
}  // namespace util
}  // namespace wire
