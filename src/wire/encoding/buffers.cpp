/*
 * buffers.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#include <wire/encoding/buffers.hpp>
#include <wire/version.hpp>
#include <wire/encoding/message.hpp>

#include <numeric>
#include <iostream>
#include <list>

namespace wire {
namespace encoding {

struct outgoing::impl : detail::buffer_sequence {
	struct encaps_state {
		outgoing* out_;
		size_type		size_before_;
		size_type		buffer_before_;
		version			encoding_version = version{ ENCODING_MAJOR, ENCODING_MINOR };

		encaps_state(outgoing* o)
			: out_(o),
			  size_before_(out_ ? out_->size() : 0),
			  buffer_before_(out_ ? out_->pimpl_->buffers_size() - 1 : 0)
		{
		}
		encaps_state(encaps_state const& rhs)
			: out_(rhs.out_),
			  size_before_(rhs.size_before_),
			  buffer_before_(rhs.buffer_before_)
		{
		}
		encaps_state(encaps_state&& rhs)
			: out_(rhs.out_),
			  size_before_(rhs.size_before_),
			  buffer_before_(rhs.buffer_before_)
		{
			rhs.out_ = nullptr;
		}
		~encaps_state()
		{
			if (out_) {
				size_type sz = size();
				buffer_type& b = out_->pimpl_->buffer_at(buffer_before_);
				auto o = std::back_inserter(b);
				write(o, encoding_version);
				write(o, sz);
			}
		}

		size_type
		size() const
		{
			if (!out_)
				return 0;
			return out_->size() - size_before_;
		}
	};
	typedef std::list< encaps_state > 		encapsulation_stack;
	typedef encapsulation_stack::iterator	encaps_iterator;

	buffer_type				header_;
	outgoing*				container_;
	message::message_flags	flags_;
	encapsulation_stack		encapsulations_;

	impl(outgoing* out)
		: buffer_sequence{1},
		  container_(out),
		  flags_(message::request),
		  encapsulations_({encaps_state{ nullptr }})
	{
	}
	impl(outgoing* out, message::message_flags flags)
		: buffer_sequence{1},
		  container_(out),
		  flags_(flags),
		  encapsulations_({encaps_state{ nullptr }})
	{
	}
	impl(impl const& rhs)
		: buffer_sequence(rhs),
		  container_(rhs.container_),
		  flags_(rhs.flags_),
		  encapsulations_(rhs.encapsulations_)
	{
	}
	impl(outgoing* out, impl const& rhs)
		: buffer_sequence(rhs),
		  container_(out),
		  flags_(rhs.flags_),
		  encapsulations_({encaps_state{ nullptr }})
	{
		auto start = rhs.encapsulations_.begin();
		std::transform(
			++start, rhs.encapsulations_.end(),
			std::back_inserter(encapsulations_),
			[this](encaps_state const& encaps)
			{
				encaps_state tmp{encaps};
				tmp.out_ = container_;
				return std::move(tmp);
			}
		);
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
	start_buffer()
	{
		buffers_.push_back({});
	}

	encaps_iterator
	begin_encaps()
	{
		//std::cerr << "begin encaps\n";
		encapsulations_.emplace_back(container_);
		start_buffer();
		return --encapsulations_.end();
	}
	void
	end_encaps(encaps_iterator iter)
	{
		if (encapsulations_.end() == ++iter) {
			//std::cerr << "end encaps\n";
			encapsulations_.pop_back();
		}
		start_buffer();
	}
	void
	insert_encaps(outgoing&& encaps)
	{
		encapsulations_.emplace_back(container_);
		buffer_sequence_type buffers = std::move(encaps.pimpl_->buffers_);
		for( auto p = buffers.begin(); p != buffers.end(); ++p) {
			if (p->size() > 0) {
				buffers_.push_back( std::move(*p) );
			}
		}
		encapsulations_.pop_back();
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

outgoing::encapsulation
outgoing::begin_encapsulation()
{
	return std::move(encapsulation{this});
}

//----------------------------------------------------------------------------
//	outgoing::encapsulation implementation
//----------------------------------------------------------------------------
struct outgoing::encapsulation::impl {
	typedef outgoing::impl::encaps_state		encaps_state;
	typedef outgoing::impl::encaps_iterator		encaps_iter;

	outgoing*		out_;
	encaps_iter		encaps_iter_;

	impl(outgoing* o) :
		out_(o),
		encaps_iter_(out_ ? out_->pimpl_->begin_encaps() : encaps_iter{})
	{
	}
	~impl()
	{
		if (out_) {
			out_->pimpl_->end_encaps(encaps_iter_);
		}
	}

	size_type
	size() const
	{
		return out_ ? encaps_iter_->size() : 0;
	}
};

outgoing::encapsulation::encapsulation(outgoing* o)
	: pimpl_(std::make_shared<impl>(o))
{
}

outgoing::encapsulation::encapsulation(encapsulation&& rhs)
	: pimpl_(rhs.pimpl_)
{
	rhs.pimpl_.reset();
}

outgoing::encapsulation::~encapsulation()
{
}

outgoing::size_type
outgoing::encapsulation::size() const
{
	return pimpl_->size();
}

//----------------------------------------------------------------------------
//	incoming implementation
//----------------------------------------------------------------------------
struct incoming::impl : detail::buffer_sequence {
	encoding::message			message_;

	impl(message const& m)
		: buffer_sequence{},
		  message_{m}
	{
	}

	impl(message const& m, buffer_type const& b)
		: buffer_sequence{b},
		  message_{m}
	{
	}

	impl(message const& m, buffer_type&& b)
		: buffer_sequence{std::move(b)},
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
};

incoming::incoming(message const& m)
	: pimpl_(::std::make_shared<impl>(m))
{
}

incoming::incoming(message const& m, buffer_type const& b)
	: pimpl_(::std::make_shared<impl>(m, b))
{
}

incoming::incoming(message const& m, buffer_type&& b)
	: pimpl_(::std::make_shared<impl>( m, std::move(b) ))
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
	::std::make_shared<impl>(m).swap(pimpl_);
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

}  // namespace encoding
}  // namespace wire
