/*
 * lua_source_stream.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: zmij
 */

#ifndef WIRE_WIRE2LUA_LUA_SOURCE_STREAM_HPP_
#define WIRE_WIRE2LUA_LUA_SOURCE_STREAM_HPP_

#include <wire/idl/ast.hpp>

#include <wire/wire2lua/mapped_type.hpp>

#include <fstream>
#include <sstream>
#include <deque>

namespace wire {
namespace idl {
namespace lua {

namespace annotations {

::std::string const LUA_FORMAT = "lua_format";

} /* namespace annotations */


class source_stream {
public:
    source_stream(::std::string const& filename);

    bool
    operator !() const
    {
        return !stream_;
    }
    //@{
    /** @name Formatting manipulation */
    void
    write_offset(int temp = 0);
    void
    modify_offset(int delta);
    void
    set_offset(int offset);
    int
    get_offset() const
    { return current_offset_; }
    //@}
private:
    template < typename T >
    friend source_stream&
    operator << (source_stream& os, T const& v);
private:
    ::std::ofstream stream_;

    int             current_offset_;
    int             tab_width_;
};

template < typename T >
source_stream&
operator << (source_stream& os, T const& v)
{
    os.stream_ << v;
    return os;
}

source_stream&
operator << (source_stream& os, mapped_type const&);

//@{
/** @name Stream manipulators */
inline source_stream&
operator << (source_stream& os, void(*f)(source_stream&))
{
    f(os);
    return os;
}

inline void
off(source_stream& ss)
{
    ss.write_offset();
}

struct offset_change {
    int delta;
};

inline source_stream&
operator << (source_stream& os, offset_change const& v)
{
    os.modify_offset(v.delta);
    os.write_offset();
    return os;
}

inline offset_change
mod(int delta)
{
    return {delta};
}

struct temp_offset {
    int delta;
};

inline source_stream&
operator << (source_stream& os, temp_offset const& v)
{
    os.write_offset(v.delta);
    return os;
}

inline temp_offset
off(int delta)
{
    return {delta};
}

inline void
sb(source_stream& ss)
{
    ss << "{";
    ss.modify_offset(+1);
}

inline void
eb(source_stream& ss)
{
    ss.modify_offset(-1);
    ss << off << "},";
}
//@}

} /* namespace lua */
} /* namespace idl */
} /* namespace wire */

#endif /* WIRE_WIRE2LUA_LUA_SOURCE_STREAM_HPP_ */
