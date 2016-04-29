/*
 * buffers.inl
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_BUFFERS_INL_
#define WIRE_ENCODING_BUFFERS_INL_

#include <wire/encoding/buffers.hpp>
#include <wire/encoding/detail/helpers.hpp>

namespace wire {
namespace encoding {

template < typename InputIterator >
incoming::incoming(message const& m, InputIterator& begin, InputIterator end)
{
    create_pimpl(m);
    insert_back(begin, end);
}

template < typename InputIterator >
void
incoming::insert_back(InputIterator& begin, InputIterator end)
{
    size_type sz = size();
    if (header().size > sz) {
        detail::copy_max(begin, end, std::back_inserter(back_buffer()),
                header().size - sz);
        std::copy(begin, end, std::back_inserter(back_buffer()));
    }
}

}  // namespace encoding
}  // namespace wire

namespace std {
template <>
class back_insert_iterator<::wire::encoding::outgoing>
    : public iterator< output_iterator_tag, void, void, void, void >{
public:
    using container_type        = ::wire::encoding::outgoing;
protected:
    container_type* container;
public:
    explicit
    back_insert_iterator(container_type& __x) : container(&__x) {}

    back_insert_iterator&
    operator=(container_type::value_type const& __value )
    {
        container->push_back(__value);
        return *this;
    }

    back_insert_iterator&
    operator=(container_type::value_type&& __value )
    {
        container->push_back(::std::move(__value));
        return *this;
    }

    back_insert_iterator&
    operator*()
    { return *this; }

    back_insert_iterator&
    operator++()
    { return *this; }

    back_insert_iterator
    operator++(int)
    { return *this; }

    ::wire::encoding::detail::outgoing_encaps
    encapsulation()
    {
        return container->current_encapsulation();
    }
};

}  /* namespace std */

#endif /* WIRE_ENCODING_BUFFERS_INL_ */
