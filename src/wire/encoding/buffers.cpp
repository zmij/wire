/*
 * buffers.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#include <wire/encoding/buffers.hpp>
#include <wire/version.hpp>
#include <numeric>
#include <iostream>

namespace wire {
namespace encoding {

template < typename Impl >
struct impl_traits {
	typedef outgoing::pointer					pointer;
	typedef outgoing::iterator					iterator_type;
};
template <typename Impl>
struct impl_traits<const Impl> {
	typedef outgoing::const_pointer		pointer;
	typedef outgoing::const_iterator	iterator_type;
};

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

	buffer_type&
	back_buffer()
	{
		return buffers_.back();
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
		if (empty())
			return end();
		return iterator{ container_, buffers_.begin(), buffers_.front().begin() };
	}
	const_iterator
	cbegin() const
	{
		if (empty())
			return cend();
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

	reverse_iterator
	rbegin()
	{
		return reverse_iterator{ end() };
	}
	const_reverse_iterator
	crbegin() const
	{
		return const_reverse_iterator{ cend() };
	}

	reverse_iterator
	rend()
	{
		return reverse_iterator{ begin() };
	}
	const_reverse_iterator
	crend() const
	{
		return const_reverse_iterator{ cbegin() };
	}

	template < typename _pointer >
	difference_type
	index_of(out_buff_iterator< _pointer > const& iter) const
	{
		if (iter.position_ == before_begin)
			return -1;
		if (iter.position_ == after_end)
			return size();
		difference_type prev_buffers = std::accumulate(
				buffers_.cbegin(), buffer_sequence_type::const_iterator(iter.buffer_),
				difference_type(0),
				[](difference_type sz, buffer_type const& buff)
				{
					return sz + buff.size();
				});
		return prev_buffers + (iter.current_ - iter.buffer_->begin());
	}

	/**
	 * @pre n is in bounds of [0,size())
	 * @param impl
	 * @param n
	 * @return
	 */
	template < typename Impl >
	static typename impl_traits< Impl >::iterator_type
	iter_at_index( Impl* impl, difference_type n )
	{
		typedef typename impl_traits< Impl >::iterator_type iterator_type;
		typedef typename iterator_type::buffer_iterator_type buffer_iterator_type;
		for ( buffer_iterator_type b = impl->buffers_.begin(); b != impl->buffers_.end() && n >= 0; ++b) {
			if (n < b->size()) {
				return iterator_type{impl->container_, b, b->begin() + n};
			}
			n -= b->size();
		}
		return iterator_type{impl->container_, after_end};
	}

	template < typename Impl >
	static void
	advance( Impl* impl, typename impl_traits< Impl >::iterator_type& iter, difference_type n)
	{
		assert(impl->container_ == iter.container_ && "Iterator belongs to container");
		typedef typename impl_traits< Impl >::iterator_type iter_type;
		if (n != 0) {
			if (iter.position_ == normal) {
				difference_type index = impl->index_of(iter);
				if (n > 0) {
					size_type sz = impl->size();
					if (index + n > sz) {
						iter_type(impl->container_, after_end).swap(iter);
					} else {
						// TODO Check if in same buffer
						iter_at_index(impl, index + n).swap(iter);
					}
				} else { // n < 0
					if (n + index < 0) {
						iter_type(impl->container_, before_begin).swap(iter);
					} else {
						// TODO Check if in same buffer
						iter_at_index(impl, index - n).swap(iter);
					}
				}
			} else if (iter.position_ == after_end && n < 0) {
				difference_type sz = impl->size();
				if (n + sz < 0) {
					iter_type(impl->container_, before_begin).swap(iter);
				} else {
					// Move n-1 from end
					iter_at_index(impl, impl->size() + n).swap(iter);
				}
			} else if (iter.position_ == before_begin && n > 0) {
				// Move n-1 from start
				difference_type sz = impl->size();
				if (n >= sz) {
					iter_type(impl->container_, after_end).swap(iter);
				} else {
					iter_at_index(impl, n).swap(iter);
				}
			}
		} // else do nothing
	}
	void
	advance(iterator& iter, difference_type diff)
	{
		advance(this, iter, diff);
	}

	void
	advance(const_iterator& iter, difference_type diff) const
	{
		advance(this, iter, diff);
	}

	template < typename Impl >
	static difference_type
	difference(Impl* impl, typename impl_traits< Impl >::iterator_type const& a, typename impl_traits< Impl >::iterator_type const& b)
	{
		assert(impl->container_ == a.container_ && "Iterator belongs to container");
		assert(impl->container_ == b.container_ && "Iterator belongs to container");
		if (a.buffer_ == b.buffer_) {
			return a.current_ - b.current_;
		}
		return impl->index_of(a) - impl->index_of(b);
	}

	difference_type
	difference(iterator const& a, iterator const& b)
	{
		return difference(this, a, b);
	}
	difference_type
	cdifference(const_iterator const& a, const_iterator const& b) const
	{
		return difference(this, a, b);
	}

	void
	push_back(value_type v)
	{
		buffers_.back().push_back(v);
	}
	void
	pop_back()
	{
		buffers_.back().pop_back();
		if (buffers_.back().empty() && buffers_.size() > 1) {
			buffers_.pop_back();
		}
	}

	void
	start_buffer()
	{
		buffers_.push_back({});
	}

	void
	begin_encaps()
	{
		start_buffer();
	}
	void
	end_encaps()
	{
		start_buffer();
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
outgoing::advance(iterator& iter, difference_type diff) const
{
	pimpl_->advance(iter, diff);
}

void
outgoing::advance(const_iterator& iter, difference_type diff) const
{
	pimpl_->advance(iter, diff);
}

outgoing::difference_type
outgoing::difference(iterator const& a, iterator const& b) const
{
	return pimpl_->difference(a, b);
}
outgoing::difference_type
outgoing::difference(const_iterator const& a, const_iterator const& b) const
{
	return pimpl_->cdifference(a, b);
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

outgoing::encapsulation
outgoing::begin_encapsulation()
{
	return std::move(encapsulation{this});
}

//----------------------------------------------------------------------------
//	encapsulation implementation
//----------------------------------------------------------------------------
struct outgoing::encapsulation::impl {
	typedef outgoing::buffer_sequence_type::iterator 	buffer_iterator;
	outgoing* out_;
	size_type		size_before_;
	size_type		buffer_before_;

	impl(outgoing* o) :
		out_(o),
		size_before_(out_->size()),
		buffer_before_(out_->pimpl_->buffers_.size())
	{
		out_->pimpl_->begin_encaps();
	}
	~impl()
	{
		if (out_) {
			size_type sz = size();
			buffer_type& b = out_->pimpl_->buffers_[buffer_before_];
			auto o = std::back_inserter(b);
			write(o, ENCODING_MAJOR);
			write(o, ENCODING_MINOR);
			write(o, sz);
			out_->pimpl_->end_encaps();
		}
	}

	size_type
	size() const
	{
		return out_->size() - size_before_;
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

}  // namespace encoding
}  // namespace wire
