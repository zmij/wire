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
#include <string>
#include <map>
#include <list>
#include <wire/version.hpp>
#include <wire/encoding/message.hpp>
#include <wire/encoding/segment.hpp>
#include <wire/errors/exceptions.hpp>

#include <iostream>

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

//----------------------------------------------------------------------------
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
    using encapsulation_type        = typename Container::in_encaps;
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

    //{@
    /** @name Encapsulation access */
    encapsulation_type
    incoming_encapsulation() const
    {
        return container_->current_in_encapsulation();
    }
    //@}
private:
    //using impl = typename Container::impl;
    friend struct buffer_sequence;
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
    container_pointer       container_;
    buffer_iterator_type    buffer_;
    value_iterator_type     current_;
    iter_position           position_;
};

//----------------------------------------------------------------------------
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
    /** @name Encapsulations */
    class out_encaps;
    class in_encaps;

    struct savepoint;
    struct out_encaps_state;
    struct in_encaps_state;
    using out_encapsulation_stack = ::std::list<out_encaps_state>;
    using out_encaps_iterator = out_encapsulation_stack::iterator;

    using in_encapsulation_stack = ::std::list< in_encaps_state >;
    using in_encaps_iterator = in_encapsulation_stack::iterator;
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

    buffer_sequence_type const&
    buffers() const
    { return buffers_; }
    //@}

    //@{
    /** @name Outgoing encapsulation */
    void
    start_buffer()
    { buffers_.push_back({}); }
    void
    pop_empty_buffer()
    {
        if (buffers_.size() > 1 && buffers_.back().empty())
            buffers_.pop_back();
    }

    out_encaps
    begin_out_encapsulation();
    out_encaps
    current_out_encapsulation() const;

    in_encaps
    begin_in_encapsulation(const_iterator);
    in_encaps
    current_in_encapsulation() const;
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
    out_encaps_iterator
    begin_out_encaps();
    out_encaps_iterator
    current_out_encaps();
    void
    end_out_encaps(out_encaps_iterator iter);

    in_encaps_iterator
    begin_in_encaps(const_iterator beg);
    in_encaps_iterator
    current_in_encaps();
    void
    end_in_encaps(in_encaps_iterator iter);
protected:
    buffer_sequence_type        buffers_;
    out_encapsulation_stack     out_encaps_stack_;
    in_encapsulation_stack      in_encaps_stack_;
};

//----------------------------------------------------------------------------
struct buffer_sequence::savepoint {
    buffer_sequence*    seq_;
    size_type           size_before_;
    size_type           buffer_before_;

    explicit
    savepoint(buffer_sequence& s)
        : seq_{&s},
          size_before_{s.size()},
          buffer_before_{ s.buffers_size() - 1 }
    {
    }

    buffer_type&
    buffer()
    { return seq_->buffer_at(buffer_before_); }

    buffer_type&
    back_buffer()
    { return seq_->back_buffer(); }

    size_type
    size() const
    {
        if (!seq_)
            return 0;
        return seq_->size() - size_before_;
    }

    bool
    empty() const
    {
        if (!seq_)
            return true;
        return seq_->size() == size_before_;
    }
};

//----------------------------------------------------------------------------
struct buffer_sequence::out_encaps_state {
    using type_map = ::std::map< ::std::string, size_type >;
    struct segment : segment_header {
        savepoint       sp_;
        size_type       type_idx_;

        segment(buffer_sequence& s, flags_type f, ::std::string const& name, size_type ti)
            : segment_header{f, name, 0}, sp_{s}, type_idx_{ti}
        {
        }
        ~segment();
    };

    using segment_ptr = ::std::unique_ptr<segment>;


    savepoint           sp_;
    version             encoding_version = version{ ENCODING_MAJOR, ENCODING_MINOR };
    type_map            types_;
    segment_ptr         current_segment_;


    out_encaps_state(buffer_sequence& out);
    out_encaps_state(out_encaps_state&& rhs);
    ~out_encaps_state();

    size_type
    size() const
    {
        return sp_.size();
    }

    bool
    empty() const
    {
        return sp_.empty();
    }

    void
    start_segment(segment_header::flags_type flags, ::std::string const& name);
    void
    end_segment();
};

//----------------------------------------------------------------------------
struct buffer_sequence::in_encaps_state {
    using type_map = ::std::vector< ::std::string >;
    buffer_sequence*    seq_;
    version             encoding_version = version{ ENCODING_MAJOR, ENCODING_MINOR };
    size_type           size_ = 0;

    const_iterator      begin_;
    const_iterator      end_;

    type_map            type_map_;

    in_encaps_state(buffer_sequence* seq, const_iterator beg);

    size_type
    size() const
    { return size_; }

    bool
    empty() const
    { return begin_ == end_; }

    template< typename InputIterator >
    void
    read_segment_header(InputIterator& begin, InputIterator& end, segment_header& sh)
    {
        read(begin, end_, sh.flags);
        if (sh.flags & segment_header::string_type_id) {
            read(begin, end_, sh.type_id);
            type_map_.push_back(sh.type_id);
            ::std::cerr << "Read string type id " << sh.type_id << "\n";
        } else {
            size_type type_idx;
            read(begin, end_, type_idx);
            if (type_idx > type_map_.size()) {
                throw errors::unmarshal_error("Invalid type index in encapsulation");
            }
            sh.type_id = type_map_[ type_idx - 1 ];
            ::std::cerr << "Read type index " << type_idx << " ("
                    << sh.type_id << ")\n";
        }
        read(begin, end_, sh.size);
        end = begin + sh.size;
    }
};

//----------------------------------------------------------------------------
class buffer_sequence::out_encaps {
public:
    out_encaps(out_encaps const&) = default;
    out_encaps(out_encaps&&) = default;

    bool
    empty() const
    { return iter_->empty(); }
    size_type
    size() const
    { return iter_->size(); }

    void
    end_encaps()
    { seq_->end_out_encaps(iter_); }

    void
    start_segment(::std::string const& name,
            segment_header::flags_type flags = segment_header::none)
    {
        iter_->start_segment(flags, name);
    }
    void
    end_segment()
    {
        iter_->end_segment();
    }
private:
    friend struct buffer_sequence;
    out_encaps(buffer_sequence* seq)
        : seq_(seq), iter_(seq->current_out_encaps()) {}
private:
    buffer_sequence*    seq_;
    out_encaps_iterator iter_;
};

//----------------------------------------------------------------------------
class buffer_sequence::in_encaps {
public:
    in_encaps(in_encaps const&) = default;
    in_encaps(in_encaps&&) = default;

    bool
    empty() const
    { return iter_->empty(); }
    size_type
    size() const
    { return iter_->size(); }

    void
    end_encaps()
    { seq_->end_in_encaps(iter_); }

    template< typename InputIterator >
    void
    read_segment_header(InputIterator& begin, InputIterator& end, segment_header& sh)
    {
        iter_->read_segment_header(begin, end, sh);
    }

    const_iterator
    begin() const
    { return iter_->begin_; }
    const_iterator
    end() const
    { return iter_->end_; }

    version const&
    encoding_version() const
    { return iter_->encoding_version; }
private:
    friend struct buffer_sequence;
    in_encaps(buffer_sequence* seq)
        : seq_{seq}, iter_{seq->current_in_encaps()} {}
private:
    buffer_sequence*    seq_;
    in_encaps_iterator  iter_;
};

//----------------------------------------------------------------------------
template< typename EncapsType >
class encaps_guard {
public:
    using encaps_type       = EncapsType;
    using pointer           = encaps_type*;
    using const_pointer     = encaps_type const*;
    using reference         = encaps_type&;
    using const_reference   = encaps_type const&;
public:
    explicit
    encaps_guard(encaps_type const& e) : encaps_{e} {}
    ~encaps_guard() { encaps_.end_encaps(); }

    encaps_type&
    encaps()
    { return encaps_; }

    encaps_type const&
    encaps() const
    { return encaps_; }

    ::std::size_t
    size() const
    { return encaps_.size(); }
    bool
    empty() const
    { return encaps_.empty(); }

    pointer
    operator->()
    { return &encaps_; }
    const_pointer
    operator->() const
    { return &encaps_; }

    reference
    operator*()
    { return encaps_; }
    const_reference
    operator*() const
    { return encaps_; }
private:
    encaps_guard(encaps_guard const&) = delete;
    encaps_guard(encaps_guard&&) = delete;
    encaps_guard&
    operator = (encaps_guard const&) = delete;
    encaps_guard&
    operator = (encaps_guard&&) = delete;
private:
    encaps_type encaps_;
};

::std::ostream&
debug_output(::std::ostream&, buffer_sequence const&);
}  // namespace detail
}  // namespace encoding
}  // namespace wire

#include <wire/encoding/detail/buffer_iterator.inl>

#endif /* WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_HPP_ */
