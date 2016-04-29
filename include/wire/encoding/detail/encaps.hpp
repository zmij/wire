/*
 * encapsulation.hpp
 *
 *  Created on: Apr 29, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_ENCAPS_HPP_
#define WIRE_ENCODING_DETAIL_ENCAPS_HPP_

#include <memory>
#include <cstdint>
#include <string>

#include <wire/version.hpp>
#include <wire/encoding/detail/buffer_iterator.hpp>

namespace wire {
namespace encoding {

class outgoing;
class incoming;

namespace detail {

class outgoing_encaps {
public:
    outgoing_encaps(outgoing_encaps const&);
    outgoing_encaps(outgoing_encaps&&);

    ::std::size_t
    size() const;
    bool
    empty() const;

    void
    end_encaps();

    //@{
    /** @name Encapsulation operations */
    void
    write_type_name(::std::string const& name);
    //@}
private:
    friend class encoding::outgoing;
    outgoing_encaps(outgoing* o);
    outgoing_encaps&
    operator = (outgoing_encaps const&) = delete;
    outgoing_encaps&
    operator = (outgoing_encaps&&) = delete;
private:
    struct impl;
    using pimpl = std::shared_ptr<impl>;
    pimpl pimpl_;
};

class incoming_encaps {
public:
    /** Normal input iterator for input buffer */
    using iterator                  = buffer_sequence::iterator;
    /** Constant input iterator for input buffer */
    using const_iterator            = buffer_sequence::const_iterator;
    /** Reverse input iterator */
    using reverse_iterator          = buffer_sequence::reverse_iterator;
    /** Constant reverse input iterator */
    using const_reverse_iterator    = buffer_sequence::const_reverse_iterator;
public:
    incoming_encaps(incoming_encaps const&);
    incoming_encaps(incoming_encaps&&);

    ::std::size_t
    size() const;
    bool
    empty() const;

    void
    end_encaps();

    const_iterator
    begin() const;
    const_iterator
    end() const;

    version const&
    encoding_version() const;
private:
    friend class encoding::incoming;
    incoming_encaps(incoming* i);
    incoming_encaps&
    operator = (incoming_encaps const&) = delete;
    incoming_encaps&
    operator = (incoming_encaps&&) = delete;
private:
    struct impl;
    using pimpl = std::shared_ptr<impl>;
    pimpl pimpl_;
};

template< typename EncapsType >
class encaps_guard {
public:
    using encaps_type = EncapsType;
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

}  /* namespace detail */
}  /* namespace encoding */
}  /* namespace wire */


#endif /* WIRE_ENCODING_DETAIL_ENCAPS_HPP_ */
