/*
 * wildcard.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_WILDCARD_HPP_
#define WIRE_CORE_WILDCARD_HPP_

#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace core {
namespace detail {

struct wildcard {
    static constexpr char symbol = '*';
    bool
    operator == (wildcard const& rhs) const { return true; }
    bool
    operator < (wildcard const& rhs) const { return false; }
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, wildcard const& v)
{
    encoding::write(o, wildcard::symbol);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, wildcard& v)
{
    char c;
    encoding::read(begin, end, c);
    if (c != wildcard::symbol) {
        throw errors::unmarshal_error{"Failed to unmarshal wildcard"};
    }
}

inline constexpr ::std::size_t
hash(wildcard w)
{
    return static_cast<::std::size_t>(wildcard::symbol);
}

}  /* namespace detail */
}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_WILDCARD_HPP_ */
