/*
 * json_test_config.in.hpp
 *
 *  Created on: Nov 8, 2016
 *      Author: zmij
 */

#ifndef JSON_JSON_TEST_CONFIG_IN_HPP_
#define JSON_JSON_TEST_CONFIG_IN_HPP_

#include <string>

namespace wire {
namespace json {
namespace test {

const ::std::string JSON_TEST_DATA_ROOT = "@JSON_TEST_DATA@";
const ::std::vector<::std::string> EXPECTED_PASS = {@EXPECTED_PASS@};
const ::std::vector<::std::string> EXPECTED_FAIL = {@EXPECTED_FAIL@};
const ::std::vector<::std::string> UNSURE_RESULT = {@UNSURE_RESULT@};

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */



#endif /* JSON_JSON_TEST_CONFIG_IN_HPP_ */
