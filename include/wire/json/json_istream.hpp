/*
 * json_istream.hpp
 *
 *  Created on: 5 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_JSON_ISTREAM_HPP_
#define WIRE_JSON_JSON_ISTREAM_HPP_

#include <string>
#include <locale>
#include <iostream>
#include <stack>
#include <type_traits>
#include <iomanip>

#include <wire/json/json_stream_fwd.hpp>
#include <wire/json/json_io_base.hpp>
#include <wire/json/traits.hpp>

namespace wire {
namespace json {

template < typename CharT, typename Traits >
class basic_json_istream : public json_io_base<CharT, Traits> {
public:
    using base_type     = json_io_base<CharT, Traits>;
    using char_type     = typename base_type::char_type;
    using traits_type   = typename base_type::traits_type;
    using this_type     = basic_json_istream<char_type, traits_type>;
    using stream_type   = typename base_type::istream_type;
    using chars         = typename base_type::chars;
    using size_type     = ::std::size_t;
public:

    this_type&
    start_object()
    {
        return *this;
    }
    this_type&
    end_object()
    {
        return *this;
    }
    this_type&
    start_array()
    {
        return *this;
    }
    this_type&
    end_array()
    {
        return *this;
    }

    ::std::locale
    getloc() const
    { return is_.getloc(); }
    void
    imbue( ::std::locale const& loc )
    { is_.imbue(loc); }
    template < typename Facet >
    void
    add_facet(Facet* fct)
    { is_.imbue(::std::locale{is_.getloc(), fct}); }

    stream_type&
    stream()
    { return is_; }
private:
private:
    stream_type         is_;
};

extern template class basic_json_istream<char>;
extern template class basic_json_istream<wchar_t>;

template < typename CharT, typename Traits >
basic_json_istream<CharT, Traits>&
operator >> (basic_json_istream<CharT, Traits>& is, void(*manip)(basic_json_istream<CharT, Traits>&))
{
    manip(is);
    return is;
}

//----------------------------------------------------------------------------
template < typename CharT, typename Traits >
void
start_object(basic_json_istream<CharT, Traits>& is)
{
    is.start_object();
}

template < typename CharT, typename Traits >
void
end_object(basic_json_istream<CharT, Traits>& is)
{
    is.end_object();
}

template < typename CharT, typename Traits >
void
start_array(basic_json_istream<CharT, Traits>& is)
{
    is.start_array();
}

template < typename CharT, typename Traits >
void
end_array(basic_json_istream<CharT, Traits>& is)
{
    is.end_array();
}

}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_JSON_ISTREAM_HPP_ */
