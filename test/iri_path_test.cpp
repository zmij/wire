/*
 * iri_path_test.cpp
 *
 *  Created on: Aug 29, 2015
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <tip/iri.hpp>

TEST(IriPath, ConstructFromString)
{
	using tip::iri::path;
	EXPECT_NO_THROW(path::parse("/foo"));
	EXPECT_ANY_THROW(path::parse("?foo"));
}
