/*
 * context.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_CONTEXT_HPP_
#define WIRE_CORE_CONTEXT_HPP_

#include <string>
#include <map>
#include <memory>
#include <sstream>

#include <wire/encoding/types.hpp>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace core {

//using context_type      = ::std::map< ::std::string, ::std::string >;

class context_type {
    using container_type    = ::std::map< ::std::string, ::std::string >;
    using value_type        = container_type::value_type;
public:
    context_type() : data_{} {}
    context_type(::std::initializer_list<value_type> init) : data_{init} {}

    void
    swap(context_type& rhs) noexcept
    {
        using ::std::swap;
        swap(data_, rhs.data_);
    }

    bool
    empty() const
    { return data_.empty(); }
    ::std::size_t
    size() const
    { return data_.size(); }

    bool
    has(::std::string const& key) const
    {
        return data_.find(key) != data_.end();
    }

    ::std::string const&
    operator[](::std::string const& key) const;

    template < typename T >
    bool
    to(::std::string const& key, T& value) const
    {
        ::std::istringstream is{ (*this)[key] };
        return ((bool) is >> value);
    }
private:
    container_type data_;

    template < typename OutputIterator >
    friend void
    wire_write(OutputIterator o, context_type const& v);
    template < typename InputIterator >
    friend void
    wire_read(InputIterator& begin, InputIterator end, context_type& v);
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, context_type const& v)
{
    encoding::write(o, v.data_);
}
template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, context_type& v)
{
    context_type tmp;
    encoding::read(begin, end, tmp.data_);
    v.swap(tmp);
}


using context_ptr       = ::std::shared_ptr< context_type >;
using context_const_ptr = ::std::shared_ptr< context_type const >;

extern const context_type no_context;

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_CONTEXT_HPP_ */
