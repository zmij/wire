/*
 * buffers.hpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_BUFFERS_HPP_
#define TIP_WIRE_BUFFERS_HPP_

#include <wire/asio_config.hpp>
#include <wire/encoding/wire_io.hpp>
#include <wire/encoding/message.hpp>
#include <wire/encoding/detail/buffer_iterator.hpp>
#include <wire/encoding/detail/encaps.hpp>

#include <memory>
#include <iterator>
#include <vector>

namespace wire {
namespace encoding {

class outgoing {
public:
    /** Internal buffers storage type */
    using buffer_type               = detail::buffer_sequence::buffer_type;
    /** Sequence of internal buffers */
    using buffer_sequence_type      = detail::buffer_sequence::buffer_sequence_type;
    using asio_buffers              = ::std::vector< ASIO_NS::const_buffer >;
    //@{
    /**
     * @name Container concept
     * http://en.cppreference.com/w/cpp/concept/Container
     */
    using value_type                = detail::buffer_sequence::value_type;
    using reference                 = detail::buffer_sequence::reference;
    using const_reference           = detail::buffer_sequence::const_reference;
    using pointer                   = detail::buffer_sequence::pointer;
    using const_pointer             = detail::buffer_sequence::const_pointer;
    using difference_type           = detail::buffer_sequence::difference_type;
    using size_type                 = detail::buffer_sequence::size_type;
    //@}
public:
    /** Normal input iterator for output buffer */
    using iterator                  = detail::buffer_sequence::iterator;
    /** Constant input iterator for output buffer */
    using const_iterator            = detail::buffer_sequence::const_iterator;
    /** Reverse input iterator */
    using reverse_iterator          = detail::buffer_sequence::reverse_iterator;
    /** Constant reverse input iterator */
    using const_reverse_iterator    = detail::buffer_sequence::const_reverse_iterator;

    using encapsulation_type        = detail::buffer_sequence::out_encaps;
    using encaps_guard              = detail::encaps_guard<encapsulation_type>;
public:
    outgoing();
    outgoing(message::message_flags);
    /** Copy construct */
    outgoing(outgoing const&);
    /** Move construct */
    outgoing(outgoing&&);

    /** Swap contents */
    void
    swap(outgoing&);

    /** Copy assign */
    outgoing&
    operator = (outgoing const&);
    /** Move assign */
    outgoing&
    operator = (outgoing&&);

    message::message_flags
    type() const;
    //@{
    /**
     * @name Container concept
     * http://en.cppreference.com/w/cpp/concept/Container
     */
    size_type
    size() const;
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

    /**
     * Get asio buffers for output
     * @return
     */
    asio_buffers
    to_buffers() const;

    //@{
    /** @name Encapsulated data */
    void
    insert_encapsulation(outgoing&&);

    encapsulation_type
    begin_encapsulation();

    encapsulation_type
    current_encapsulation();
    //@}
private:
    struct impl;
    using pimpl = ::std::shared_ptr<impl>;
    pimpl pimpl_;
};

using outgoing_ptr = ::std::shared_ptr< outgoing >;

inline void
swap(outgoing& lhs, outgoing& rhs)
{
    lhs.swap(rhs);
}

class incoming {
public:
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
public:
    /** Normal input iterator for input buffer */
    using iterator                  = detail::buffer_sequence::iterator;
    /** Constant input iterator for input buffer */
    using const_iterator            = detail::buffer_sequence::const_iterator;
    /** Reverse input iterator */
    using reverse_iterator          = detail::buffer_sequence::reverse_iterator;
    /** Constant reverse input iterator */
    using const_reverse_iterator    = detail::buffer_sequence::const_reverse_iterator;

    using encapsulation_type        = detail::incoming_encaps;
    using encaps_guard              = detail::encaps_guard<encapsulation_type>;
public:
    incoming(message const&);
    template < typename InputIterator >
    incoming(message const&, InputIterator& begin, InputIterator end);
    incoming(message const&, buffer_type const&);
    incoming(message const&, buffer_type&&);

    message const&
    header() const;
    message::message_flags
    type() const;

    template < typename InputIterator >
    void
    insert_back(InputIterator& begin, InputIterator end);
    void
    insert_back(buffer_type const&);
    void
    insert_back(buffer_type&&);

    /**
     * Check if the message has been read from network buffers.
     * @return
     */
    bool
    complete() const;
    /**
     * How many bytes is left to read from network buffers
     * @return
     */
    size_type
    want_bytes() const;

    //@{
    /**
     * @name Container concept
     * http://en.cppreference.com/w/cpp/concept/Container
     */
    size_type
    size() const;
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
    /** @name Encapsulated data */
    encapsulation_type
    begin_encapsulation(const_iterator);

    encapsulation_type
    current_encapsulation();
    //@}
private:
    friend class detail::incoming_encaps;
    void
    create_pimpl(message const&);

    buffer_type&
    back_buffer();
private:
    struct impl;
    using pimpl = ::std::shared_ptr<impl>;
    pimpl pimpl_;
};

using incoming_ptr = ::std::shared_ptr<incoming>;

// TODO Chage the signature to an encapsulation
using reply_callback
        = ::std::function< void(incoming::const_iterator, incoming::const_iterator) >;
using request_result_callback = ::std::function< void(outgoing&&) >;

}  // namespace encoding
}  // namespace wire

#include <wire/encoding/buffers.inl>

#endif /* TIP_WIRE_BUFFERS_HPP_ */
