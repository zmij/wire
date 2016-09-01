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

    impl(core::connector_ptr cnctr, outgoing* out)
        : buffer_sequence{cnctr, 1},
          container_(out),
          flags_(message::request)
    {
        out_encaps_stack_.emplace_back(*this, true);
    }
    impl(core::connector_ptr cnctr, outgoing* out, message::message_flags flags)
        : buffer_sequence{cnctr, 1},
          container_(out),
          flags_(flags)
    {
        out_encaps_stack_.emplace_back(*this, true);
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
        return buffs;
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

outgoing::outgoing(core::connector_ptr cnctr)
    : pimpl_(new impl(cnctr, this))
{
}
outgoing::outgoing(core::connector_ptr cnctr, message::message_flags flags)
    : pimpl_(new impl(cnctr, this, flags))
{
}

//outgoing::outgoing(outgoing const& rhs)
//    : pimpl_(new impl(this, *rhs.pimpl_))
//{
//}

outgoing::outgoing(outgoing&& rhs)
    : pimpl_(std::move(rhs.pimpl_))
{
    pimpl_->container_ = this;
}

outgoing::~outgoing() = default;

void
outgoing::swap(outgoing& rhs)
{
    using std::swap;
    swap(pimpl_, rhs.pimpl_);
    swap(pimpl_->container_, rhs.pimpl_->container_);
}

//outgoing&
//outgoing::operator =(outgoing const& rhs)
//{
//    outgoing tmp(rhs);
//    swap(tmp);
//    return *this;
//}

outgoing&
outgoing::operator =(outgoing&& rhs)
{
    pimpl_ = std::move(rhs.pimpl_);
//    rhs.pimpl_.reset();
    return *this;
}

core::connector_ptr
outgoing::get_connector() const
{
    return pimpl_->get_connector();
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
    return pimpl_->to_buffers();
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

void
outgoing::close_all_encaps()
{
    pimpl_->close_out_encapsulations();
}

void
outgoing::debug_print(::std::ostream& os) const
{
    debug_output(os, *pimpl_);
}

//----------------------------------------------------------------------------
//    incoming implementation
//----------------------------------------------------------------------------
struct incoming::impl : detail::buffer_sequence {

    incoming*                   container_;
    encoding::message           message_;

    impl(core::connector_ptr cnctr, incoming* in, message const& m)
        : buffer_sequence{cnctr},
          container_{in},
          message_{m}
    {
        in_encaps_stack_.emplace_back(*this);
    }

    impl(core::connector_ptr cnctr, incoming* in, message const& m, buffer_type const& b)
        : buffer_sequence{cnctr, b},
          container_{in},
          message_{m}
    {
        in_encaps_stack_.emplace_back(*this);
    }

    impl(core::connector_ptr cnctr, incoming* in, message const& m, buffer_type&& b)
        : buffer_sequence{cnctr, std::move(b)},
          container_{in},
          message_{m}
    {
        in_encaps_stack_.emplace_back(*this);
    }

    impl(incoming* in, message const& m, detail::buffer_sequence&& bs)
        : buffer_sequence(::std::move(bs)),
          container_{in},
          message_{m}
    {
        in_encaps_stack_.emplace_back(*this);
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
};

incoming::incoming(core::connector_ptr cnctr, message const& m)
    : pimpl_(::std::make_shared<impl>(cnctr, this, m))
{
}

incoming::incoming(core::connector_ptr cnctr, message const& m, buffer_type const& b)
    : pimpl_(::std::make_shared<impl>(cnctr, this, m, b))
{
}

incoming::incoming(core::connector_ptr cnctr, message const& m, buffer_type&& b)
    : pimpl_(::std::make_shared<impl>(cnctr, this, m, std::move(b) ))
{
}

incoming::incoming(message const& m, outgoing&& out)
    : pimpl_(::std::make_shared<impl>(this, m, ::std::move(*out.pimpl_)))
{
}

core::connector_ptr
incoming::get_connector() const
{
    return pimpl_->get_connector();
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
incoming::create_pimpl(core::connector_ptr cnctr, message const& m)
{
    ::std::make_shared<impl>(cnctr, this, m).swap(pimpl_);
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
    return pimpl_->begin_in_encapsulation(b);
}

incoming::encapsulation_type
incoming::current_encapsulation()
{
    return pimpl_->current_in_encapsulation();
}

void
incoming::debug_print(::std::ostream& os) const
{
    debug_output(os, *pimpl_);
}

}  // namespace encoding
}  // namespace wire
