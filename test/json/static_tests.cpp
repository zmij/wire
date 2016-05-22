/*
 * static_tests.cpp
 *
 *  Created on: 22 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <wire/json/traits.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace wire {
namespace json {
namespace test {

static_assert( traits::json_type< bool >::value == traits::value_type::BOOL,
        "Correct json value type" );
static_assert( traits::json_type< int >::value == traits::value_type::NUMBER,
        "Correct json value type" );
static_assert( traits::json_type< ::std::string >::value == traits::value_type::STRING,
        "Correct json value type");
static_assert( traits::json_type< ::boost::uuids::uuid >::value == traits::value_type::STRING,
        "Correct json value type");

TEST(Dummy, Dummy)
{

}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */
