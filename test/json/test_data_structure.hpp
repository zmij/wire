/*
 * test_datat_structure.hpp
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef JSON_TEST_DATA_STRUCTURE_HPP_
#define JSON_TEST_DATA_STRUCTURE_HPP_

#include <wire/json/json_stream.hpp>

namespace wire {
namespace json {
namespace test {

struct test_structure {
    ::std::string   str;
    int             ival;
    float           fval;
};

inline json::json_ostream&
json_write(json::json_ostream& os, test_structure const& v)
{
    os  << "str" << v.str
        << "ival" << v.ival
        << "fval" << v.fval;
    return os;
}


}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */



#endif /* JSON_TEST_DATA_STRUCTURE_HPP_ */
