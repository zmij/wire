/*
 * container_output.hpp
 *
 *  Created on: 5 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_CONTAINER_OUTPUT_HPP_
#define WIRE_JSON_DETAIL_CONTAINER_OUTPUT_HPP_

#include <wire/json/json_stream.hpp>

namespace wire {
namespace json {
namespace detail {
}  /* namespace detail */

template < typename CharT, typename Traits, typename T, typename ... Rest >
basic_json_ostream<CharT, Traits>&
json_write(basic_json_ostream<CharT, Traits>& os, ::std::vector<T, Rest...> const& v)
{
    for (auto const& e : v) {
        os << e;
    }
    return os;
}

}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_CONTAINER_OUTPUT_HPP_ */
