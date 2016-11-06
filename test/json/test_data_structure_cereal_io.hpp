/*
 * test_data_structure_cereal_io.hpp
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef JSON_TEST_DATA_STRUCTURE_CEREAL_IO_HPP_
#define JSON_TEST_DATA_STRUCTURE_CEREAL_IO_HPP_

#include "test_data_structure.hpp"
#include <cereal/cereal.hpp>

namespace wire {
namespace json {
namespace test {

template < typename Archive >
void
CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar, test_structure& v)
{
    ar(
        cereal::make_nvp("str", v.str),
        cereal::make_nvp("ival", v.ival),
        cereal::make_nvp("fval", v.fval)
    );
}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */


#endif /* JSON_TEST_DATA_STRUCTURE_CEREAL_IO_HPP_ */
