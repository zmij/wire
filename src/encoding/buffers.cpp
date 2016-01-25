/*
 * buffers.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#include <wire/encoding/buffers.hpp>

namespace wire {
namespace encoding {

struct outgoing::impl {
	outgoing*				container_;
	buffer_sequence_type	buffers_;

	impl(outgoing* out) : container_(out), buffers_(1, buffer_type{})
	{
	}

	impl(impl const& rhs) : container_(rhs.container_), buffers_(rhs.buffers_)
	{
	}
	impl(outgoing* out, impl const& rhs) : container_(out), buffers_(rhs.buffers_)
	{
	}

	buffer_sequence_type::iterator
	back_buffer()
	{
		return buffers_.begin() + (buffers_.size() - 1);
	}

	size_type
	size() const
	{
		return std::accumulate(buffers_.begin(), buffers_.end(), size_type(0),
			[](size_type sz, buffer_type const& buff)
			{
				return sz + buff.size();
			});
	}

	bool
	empty() const
	{
		return std::accumulate(buffers_.begin(), buffers_.end(), true,
			[](bool empty, buffer_type const& buff)
			{
				return empty && buff.empty();
			});
	}

	iterator
	begin()
	{
		return iterator{ container_, buffers_.begin(), buffers_.front().begin() };
	}
	const_iterator
	cbegin() const
	{
		return const_iterator{ container_, buffers_.cbegin(), buffers_.front().cbegin() };
	}

	iterator
	end()
	{
		return iterator{ container_, after_end };
	}
	const_iterator
	cend() const
	{
		return const_iterator{ container_, after_end };
	}

	template < typename _pointer >
	size_type
	index_of(out_buff_iterator< _pointer > const& iter) const
	{
		size_type prev_buffers = std::accumulate(
				buffers_.cbegin(), buffer_sequence_type::const_iterator(iter.buffer_),
				size_type(0),
				[](size_type sz, buffer_type const& buff)
				{
					return sz + buff.size();
				});
		return prev_buffers + (iter.current_ - iter.buffer_->begin());
	}

	template < typename _pointer >
	void
	do_advance( out_buff_iterator< _pointer >& iter, difference_type n) const
	{
		typedef out_buff_iterator< _pointer > iter_type;
		if (n != 0) {
			if (iter.position_ == normal) {
				size_type index = index_of(iter);
				if (n > 0) {
					size_type sz = size();
					if (index + n > sz) {
						iter_type(container_, after_end).swap(iter);
					} else {

					}
				} else { // n < 0
					if (n + index < 0) {
						iter_type(container_, before_begin).swap(iter);
					} else {

					}
				}
			} else if (iter.position_ == after_end && n < 0) {
				// Move n-1 from end
			} else if (iter.position_ == before_begin && n > 0) {
				// Move n-1 from start
			}
		} // else do nothing
	}
	void
	advance(iterator& iter, difference_type diff) const
	{
		do_advance(iter, diff);
	}

	void
	advance(const_iterator& iter, difference_type diff) const
	{
		do_advance(iter, diff);
	}
};

outgoing::outgoing()
	: pimpl_(std::make_shared<impl>(this))
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

outgoing::encapsulation&&
outgoing::begin_encapsulation()
{
	return std::move(encapsulation{this});
}

void
outgoing::advance(iterator& iter, difference_type diff) const
{
	pimpl_->advance(iter, diff);
}

void
outgoing::advance(const_iterator& iter, difference_type diff) const
{
	pimpl_->advance(iter, diff);
}

outgoing::encapsulation::encapsulation(outgoing* o)
	: out_(o)
{
	// Start encapsulation here
}

outgoing::encapsulation::encapsulation(encapsulation&& rhs)
	: out_(rhs.out_)
{
	rhs.out_ = nullptr;
}

outgoing::encapsulation::~encapsulation()
{
	if (out_) {
		// finish encapsulation here
	}
}

}  // namespace encoding
}  // namespace wire
