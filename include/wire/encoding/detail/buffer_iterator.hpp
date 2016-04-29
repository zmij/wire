/*
 * out_buffer_traits.hpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_HPP_
#define WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_HPP_

#include <vector>
#include <cstdint>

namespace wire {
namespace encoding {
namespace detail {

template < typename Container, typename Pointer >
struct buffer_traits;

template < typename Container >
struct buffer_traits< Container, uint8_t* > {
    using container_type            = Container;
    using container_pointer         = container_type*;
    using buffer_type               = ::std::vector<uint8_t>;
    using value_type                = buffer_type::value_type;
    using buffers_sequence_type     = ::std::vector<buffer_type>;

    using pointer                   = buffer_type::pointer;
    using reference                 = buffer_type::reference;

    using buffer_iterator_type      = buffers_sequence_type::iterator;
    using value_iterator_type       = buffer_type::iterator;
    using difference_type           = buffer_type::difference_type;
};

template < typename Container >
struct buffer_traits< Container, uint8_t const* > {
    using container_type            = Container;
    using container_pointer         = container_type const*;
    using buffer_type               = ::std::vector<uint8_t>;
    using value_type                = buffer_type::value_type;
    using buffers_sequence_type     = ::std::vector<buffer_type>;

    using pointer                   = buffer_type::const_pointer;
    using reference                 = buffer_type::const_reference;
    using buffer_iterator_type      = buffers_sequence_type::const_iterator;
    using value_iterator_type       = buffer_type::const_iterator;
    using difference_type           = buffer_type::difference_type;
};

enum iter_position {
    normal,
    after_end,
    before_begin
};

struct buffer_sequence;

template < typename Container, typename Pointer >
class buffer_iterator : public ::std::iterator< ::std::random_access_iterator_tag,
    typename buffer_traits<Container, Pointer>::value_type,
    typename buffer_traits<Container, Pointer>::difference_type,
    typename buffer_traits<Container, Pointer>::pointer,
    typename buffer_traits<Container, Pointer>::reference > {
public:
    using buffer_traits_type        = buffer_traits<Container, Pointer>;
    using container_pointer         = typename buffer_traits_type::container_pointer;
    using buffer_iterator_type      = typename buffer_traits_type::buffer_iterator_type;
    using value_iterator_type       = typename buffer_traits_type::value_iterator_type;
    using value_type                = typename buffer_traits_type::value_type;
    using difference_type           = typename buffer_traits_type::difference_type;
    using iterator_type             = ::std::iterator< ::std::random_access_iterator_tag,
                                        value_type, difference_type,
                                        typename buffer_traits_type::pointer,
                                        typename buffer_traits_type::reference
                                      >;
    using pointer                   = typename iterator_type::pointer;
    using reference                 = typename iterator_type::reference;
public:
    buffer_iterator();
    buffer_iterator(buffer_iterator const&);
    template< typename T >
    buffer_iterator(buffer_iterator<Container, T> const&);

    void
    swap(buffer_iterator&);

    buffer_iterator&
    operator = (buffer_iterator const&);
    template< typename T >
    buffer_iterator&
    operator = (buffer_iterator<Container, T> const&);

    bool
    operator == (buffer_iterator const&) const;
    bool
    operator != (buffer_iterator const&) const;

    //@{
    /** @name Forward iterator requirements */
    reference
    operator *() const;
    pointer
    operator ->() const;

    buffer_iterator&
    operator ++();
    buffer_iterator
    operator ++(int);
    //@}

    //@{
    /** @name Bidirectional iterator requirements */
    buffer_iterator&
    operator --();
    buffer_iterator
    operator --(int);
    //@}

    //@{
    /** @name Random access iterator requirements */
    /** @TODO Random access operator */
    buffer_iterator&
    operator += (difference_type n);
    buffer_iterator
    operator + (difference_type n) const;

    buffer_iterator&
    operator -= (difference_type n);
    buffer_iterator
    operator - (difference_type n) const;
    template < typename _P >
    difference_type
    operator - (buffer_iterator<Container, _P> const&) const;
    //@}
private:
    //using impl = typename Container::impl;
    friend class buffer_sequence;
    template < typename C, typename T >
    friend class buffer_iterator;

    template < typename C, typename T >
    friend typename C::difference_type
    index_of(typename C::buffer_sequence_type const&, buffer_iterator<C, T> const&);

    buffer_iterator(container_pointer c, buffer_iterator_type buff, value_iterator_type curr)
        : container_(c), buffer_(buff), current_(curr), position_(normal) {}
    buffer_iterator(container_pointer c, iter_position pos)
        : container_(c), buffer_(), current_(), position_(pos) {}
private:
    container_pointer        container_;
    buffer_iterator_type    buffer_;
    value_iterator_type        current_;
    iter_position            position_;
};

struct buffer_sequence {
    /** Internal buffers storage type */
    using buffer_type               = ::std::vector<uint8_t>;
    /** Sequence of internal buffers */
    using buffer_sequence_type      = ::std::vector<buffer_type>;
    //@{
    /**
     * @name Container concept
     * http://en.cppreference.com/w/cpp/concept/Container
     */
    using value_type                = buffer_type::value_type;
    using reference                 = buffer_type::reference;
    using const_reference           = buffer_type::const_reference;
    using pointer                   = buffer_type::pointer;
    using const_pointer             = buffer_type::const_pointer;
    using difference_type           = buffer_type::difference_type;
    using size_type                 = buffer_type::size_type;
    //@}
    //@{
    /** @name Iterator types */
    using iterator                  = buffer_iterator< buffer_sequence, pointer >;
    using const_iterator            = buffer_iterator< buffer_sequence, const_pointer >;
    using reverse_iterator          = ::std::reverse_iterator< iterator >;
    using const_reverse_iterator    = ::std::reverse_iterator< const_iterator >;
    //@}

    //@{
    /** @name Constructors */
    buffer_sequence();
    buffer_sequence(size_type number);
    buffer_sequence(buffer_type const& b);
    buffer_sequence(buffer_type&& b);

    buffer_sequence(buffer_sequence const&);
    buffer_sequence(buffer_sequence&&);
    //@}

    void
    swap(buffer_sequence&);

    //@{
    /** @name Assignment */
    buffer_sequence&
    operator = (buffer_sequence const&);
    buffer_sequence&
    operator = (buffer_sequence&&);
    //@}

    //@{
    /**
     * @name Container concept
     * http://en.cppreference.com/w/cpp/concept/Container
     */
    size_type
    size() const;
//    size_type
//    max_size() const;
    bool
    empty() const;

    iterator
    begin();
    inline const_iterator
    begin() const
    { return cbegin(); }
    const_iterator
    cbegin() const;

    iterator
    end();
    inline const_iterator
    end() const
    { return cend(); }

    const_iterator
    cend() const;
    //@}
    //@{
    /** @name ReversibleContainer concept */
    reverse_iterator
    rbegin();
    inline const_reverse_iterator
    rbegin() const
    { return crbegin(); }
    const_reverse_iterator
    crbegin() const;

    reverse_iterator
    rend();
    inline const_reverse_iterator
    rend() const
    { return crend(); }
    const_reverse_iterator
    crend() const;
    //@}
    //@{
    /**
     * @name SequenceContainer concept
     * http://en.cppreference.com/w/cpp/concept/SequenceContainer
     */
    reference
    front();
    const_reference
    front() const;

    reference
    back();
    const_reference
    back() const;

    void
    push_back(value_type);
    void
    pop_back();

    reference
    operator[] (size_type);
    const_reference
    operator[] (size_type) const;

    reference
    at(size_type);
    const_reference
    at(size_type) const;
    //@}

    //@{
    iterator
    last();
    inline const_iterator
    last() const
    { return clast(); };
    const_iterator
    clast() const;
    //@}
    //@{
    /** @name Buffers */
    inline size_type
    buffers_size() const
    { return buffers_.size(); }

    inline buffer_type&
    front_buffer()
    { return buffers_.front(); }
    inline buffer_type const&
    front_buffer() const
    { return buffers_.front(); }

    inline buffer_type&
    back_buffer()
    { return buffers_.back(); }
    inline buffer_type const&
    back_buffer() const
    { return buffers_.back(); }

    inline buffer_type&
    buffer_at(size_type index)
    { return buffers_[index]; }
    inline buffer_type const&
    buffer_at(size_type index) const
    { return buffers_[index]; }
    //@}
private:
    friend class buffer_iterator<buffer_sequence, pointer>;
    friend class buffer_iterator<buffer_sequence, const_pointer>;

    template < typename P, typename This >
    static buffer_iterator< typename ::std::remove_const< This >::type, P >
    iter_at_index(This* _this, size_type n);

    template < typename This, typename P >
    static void
    advance(This* _this,
            buffer_iterator< typename ::std::remove_const< This >::type, P>& iter, difference_type n);
    void
    advance(iterator& iter, difference_type diff) const;
    void
    advance(const_iterator& iter, difference_type diff) const;

    template < typename This, typename P >
    static difference_type
    index_of(This* _this, buffer_iterator< typename ::std::remove_const< This >::type, P> const&);

    template < typename This, typename P >
    static difference_type
    difference(This* _this,
            buffer_iterator< typename ::std::remove_const< This >::type, P> const&,
            buffer_iterator< typename ::std::remove_const< This >::type, P> const&);
    difference_type
    difference(iterator const& a, iterator const& b) const;
    difference_type
    difference(const_iterator const& a, const_iterator const& b) const;
protected:
    buffer_sequence_type buffers_;
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire

#include <wire/encoding/detail/buffer_iterator.inl>

#endif /* WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_HPP_ */
