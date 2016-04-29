/*
 * buffers.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#include <wire/encoding/buffers.hpp>
#include <wire/version.hpp>
#include <wire/encoding/message.hpp>
#include <wire/errors/exceptions.hpp>

#include <numeric>
#include <iostream>
#include <list>

namespace wire {
namespace encoding {

struct outgoing::impl : detail::buffer_sequence {
    buffer_type                header_;
    outgoing*                  container_;
    message::message_flags     flags_;

    impl(outgoing* out)
        : buffer_sequence{1},
          container_(out),
          flags_(message::request)
    {
    }
    impl(outgoing* out, message::message_flags flags)
        : buffer_sequence{1},
          container_(out),
          flags_(flags)
    {
    }
    impl(impl const& rhs)
        : buffer_sequence(rhs),
          container_(rhs.container_),
          flags_(rhs.flags_)
    {
    }
    impl(outgoing* out, impl const& rhs)
        : buffer_sequence(rhs),
          container_(out),
          flags_(rhs.flags_)
    {
    }

    buffer_type&
    message_header_buffer()
    {
        return header_;
    }

    asio_buffers
    to_buffers()
    {
        // TODO close encaps and other stuff
        if (message_header_buffer().empty()) {
            message m { flags_, size() };
            write(std::back_inserter(message_header_buffer()), m);
        }
        asio_buffers buffs;
        buffs.push_back(ASIO_NS::buffer(message_header_buffer()));
        for (auto const& b : buffers_) {
            if (!b.empty()) {
                buffs.push_back(ASIO_NS::buffer(b));
            }
        }
        return std::move(buffs);
    }

    void
    insert_encaps(outgoing&& encaps)
    {
        auto iter = begin_out_encaps();
        buffer_sequence_type buffers = std::move(encaps.pimpl_->buffers_);
        for( auto p = buffers.begin(); p != buffers.end(); ++p) {
            if (p->size() > 0) {
                buffers_.push_back( std::move(*p) );
            }
        }
        end_out_encaps(iter);
        start_buffer();
    }
};

outgoing::outgoing()
    : pimpl_(std::make_shared<impl>(this))
{
}
outgoing::outgoing(message::message_flags flags)
    : pimpl_(std::make_shared<impl>(this, flags))
{
}

outgoing::outgoing(outgoing const& rhs)
    : pimpl_(std::make_shared<impl>(this, *rhs.pimpl_))
{
}

outgoing::outgoing(outgoing&& rhs)
    : pimpl_(std::move(rhs.pimpl_))
{
    pimpl_->container_ = this;
}

void
outgoing::swap(outgoing& rhs)
{
    using std::swap;
    swap(pimpl_, rhs.pimpl_);
    swap(pimpl_->container_, rhs.pimpl_->container_);
}

outgoing&
outgoing::operator =(outgoing const& rhs)
{
    outgoing tmp(rhs);
    swap(tmp);
    return *this;
}

outgoing&
outgoing::operator =(outgoing&& rhs)
{
    pimpl_ = std::move(rhs.pimpl_);
    rhs.pimpl_.reset();
    return *this;
}

message::message_flags
outgoing::type() const
{
    return static_cast<message::message_flags>(pimpl_->flags_ & message::type_bits);
}

outgoing::size_type
outgoing::size() const
{
    return pimpl_->size();
}

bool
outgoing::empty() const
{
    return pimpl_->empty();
}

outgoing::iterator
outgoing::begin()
{
    return pimpl_->begin();
}
outgoing::const_iterator
outgoing::cbegin() const
{
    return pimpl_->cbegin();
}

outgoing::iterator
outgoing::end()
{
    return pimpl_->end();
}
outgoing::const_iterator
outgoing::cend() const
{
    return pimpl_->cend();
}

outgoing::reverse_iterator
outgoing::rbegin()
{
    return pimpl_->rbegin();
}
outgoing::const_reverse_iterator
outgoing::crbegin() const
{
    return pimpl_->crbegin();
}

outgoing::reverse_iterator
outgoing::rend()
{
    return pimpl_->rend();
}
outgoing::const_reverse_iterator
outgoing::crend() const
{
    return pimpl_->crend();
}

void
outgoing::push_back(value_type v)
{
    pimpl_->push_back(v);
}

void
outgoing::pop_back()
{
    pimpl_->pop_back();
}

outgoing::asio_buffers
outgoing::to_buffers() const
{
    return std::move(pimpl_->to_buffers());
}

void
outgoing::insert_encapsulation(outgoing&& encaps)
{
    pimpl_->insert_encaps(std::move(encaps));
}

outgoing::encapsulation_type
outgoing::begin_encapsulation()
{
    return pimpl_->begin_out_encapsulation();
}

outgoing::encapsulation_type
outgoing::current_encapsulation()
{
    return pimpl_->current_out_encapsulation();
}

//----------------------------------------------------------------------------
//    incoming implementation
//----------------------------------------------------------------------------
struct incoming::impl : detail::buffer_sequence {
    struct encaps_state {
        using type_map = ::std::vector< ::std::string >;
        incoming*       in;
        version         encoding_version = version{ ENCODING_MAJOR, ENCODING_MINOR };
        size_type       size_ = 0;

        const_iterator  begin_;
        const_iterator  end_;

        type_map        type_map_;

        encaps_state(incoming* in, const_iterator b)
            : in(in), begin_(b)
        {
            end_ = in->end();
            read(begin_, end_, encoding_version, size_);
            // TODO Check correct encoding version
            end_ = begin_ + size_;
        }

        size_type
        size() const
        {
            return size_;
        }

        bool
        empty() const
        {
            return begin_ == end_;
        }

        void
        read_type_name(const_iterator& iter, ::std::string& name)
        {
            ::std::size_t type_idx;
            read(iter, end_, type_idx);
            if (type_idx == 0) {
                // String type name follows
                read(iter, end_, name);
                type_map_.push_back(name);
            } else {
                if (type_idx > type_map_.size()) {
                    throw errors::unmarshal_error("Invalid type index in encapsulation");
                }
                name = type_map_[ type_idx - 1 ];
            }
        }
    };
    using encapsulation_stack = std::list< encaps_state >;
    using encaps_iterator = encapsulation_stack::iterator;

    incoming*                   container_;
    encoding::message           message_;
    encapsulation_stack         encapsulations_;

    impl(incoming* in, message const& m)
        : container_{in},
          buffer_sequence{},
          message_{m}
    {
    }

    impl(incoming* in, message const& m, buffer_type const& b)
        : container_{in},
          buffer_sequence{b},
          message_{m}
    {
    }

    impl(incoming* in, message const& m, buffer_type&& b)
        : container_{in},
          buffer_sequence{std::move(b)},
          message_{m}
    {
    }

    void
    insert_back(buffer_type const& b)
    {
        if (!b.empty()) {
            buffers_.push_back(b);
        }
    }

    void
    insert_back(buffer_type&& b)
    {
        if (!b.empty()) {
            buffers_.push_back(std::move(b));
        }
    }

    size_type
    want_bytes()
    {
        return message_.size - size();
    }
    bool
    complete()
    {
        return message_.size <= size();
    }

    encaps_iterator
    begin_encaps(const_iterator iter)
    {
        encapsulations_.emplace_back(container_, iter);
        return --encapsulations_.end();
    }

    void
    end_encaps(encaps_iterator iter)
    {
        if (encapsulations_.end() == ++iter) {
            encapsulations_.pop_back();
        }
    }

    encaps_iterator
    current_encaps()
    {
        if (encapsulations_.empty())
            throw errors::unmarshal_error("No current encapsulation");
        return --encapsulations_.end();
    }
};

incoming::incoming(message const& m)
    : pimpl_(::std::make_shared<impl>(this, m))
{
}

incoming::incoming(message const& m, buffer_type const& b)
    : pimpl_(::std::make_shared<impl>(this, m, b))
{
}

incoming::incoming(message const& m, buffer_type&& b)
    : pimpl_(::std::make_shared<impl>(this, m, std::move(b) ))
{
}

message const&
incoming::header() const
{
    return pimpl_->message_;
}

message::message_flags
incoming::type() const
{
    return pimpl_->message_.type();
}

void
incoming::insert_back(buffer_type const& b)
{
    pimpl_->insert_back(b);
}

void
incoming::insert_back(buffer_type&& b)
{
    pimpl_->insert_back(std::move(b));
}

void
incoming::create_pimpl(message const& m)
{
    ::std::make_shared<impl>(this, m).swap(pimpl_);
}

incoming::buffer_type&
incoming::back_buffer()
{
    return pimpl_->back_buffer();
}

bool
incoming::empty() const
{
    return pimpl_->empty();
}

incoming::size_type
incoming::size() const
{
    return pimpl_->size();
}

bool
incoming::complete() const
{
    return pimpl_->complete();
}

incoming::size_type
incoming::want_bytes() const
{
    return pimpl_->want_bytes();
}

incoming::iterator
incoming::begin()
{
    return pimpl_->begin();
}

incoming::const_iterator
incoming::cbegin() const
{
    return pimpl_->cbegin();
}

incoming::iterator
incoming::end()
{
    return pimpl_->end();
}

incoming::const_iterator
incoming::cend() const
{
    return pimpl_->cend();
}

incoming::reverse_iterator
incoming::rbegin()
{
    return pimpl_->rbegin();
}

incoming::const_reverse_iterator
incoming::crbegin() const
{
    return pimpl_->crbegin();
}

incoming::reverse_iterator
incoming::rend()
{
    return pimpl_->rend();
}

incoming::const_reverse_iterator
incoming::crend() const
{
    return pimpl_->crend();
}

incoming::encapsulation_type
incoming::begin_encapsulation(const_iterator b)
{
    pimpl_->begin_encaps(b);
    return encapsulation_type{ this };
}

incoming::encapsulation_type
incoming::current_encapsulation()
{
    return encapsulation_type{ this };
}

//----------------------------------------------------------------------------
//  incoming encaps implementation
//----------------------------------------------------------------------------
namespace detail {

struct incoming_encaps::impl {
    using encaps_state = incoming::impl::encaps_state;
    using encaps_iter = incoming::impl::encaps_iterator;

    incoming*   in_;
    encaps_iter encaps_iter_;

    impl(incoming* in)
        : in_{in},
          encaps_iter_{in_ ? in_->pimpl_->current_encaps() : encaps_iter{}}
    {
    }

    incoming::size_type
    size() const
    {
        return in_ ? encaps_iter_->size() : 0;
    }

    bool
    empty() const
    {
        return in_ ? encaps_iter_->empty() : 0;
    }

    void
    end_encaps()
    {
        if (in_) {
            in_->pimpl_->end_encaps(encaps_iter_);
        }
    }

    const_iterator
    cbegin()
    {
        if (in_) {
            return encaps_iter_->begin_;
        }
        return const_iterator{};
    }

    const_iterator
    cend()
    {
        if (in_) {
            return encaps_iter_->end_;
        }
        return const_iterator{};
    }

    version const&
    ver()
    {
        if (in_) {
            return encaps_iter_->encoding_version;
        }
        throw errors::unmarshal_error("Invalid encapsulation");
    }
};

incoming_encaps::incoming_encaps(incoming* in)
    : pimpl_{ ::std::make_shared< impl >(in)}
{
}

incoming_encaps::incoming_encaps(incoming_encaps const& rhs)
    : pimpl_{ rhs.pimpl_ }
{
}

incoming_encaps::incoming_encaps(incoming_encaps&& rhs)
    : pimpl_{ ::std::move(rhs.pimpl_) }
{
    rhs.pimpl_.reset();
}

::std::size_t
incoming_encaps::size() const
{
    return pimpl_->size();
}

bool
incoming_encaps::empty() const
{
    return pimpl_->empty();
}

void
incoming_encaps::end_encaps()
{
    pimpl_->end_encaps();
}

incoming_encaps::const_iterator
incoming_encaps::begin() const
{
    return pimpl_->cbegin();
}

incoming_encaps::const_iterator
incoming_encaps::end() const
{
    return pimpl_->cend();
}

version const&
incoming_encaps::encoding_version() const
{
    return pimpl_->ver();
}

}  /* namespace detail */

}  // namespace encoding
}  // namespace wire
