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

template < typename Impl >
struct impl_traits {
	typedef buffer::pointer					pointer;
	typedef buffer::iterator					iterator_type;
};
template <typename Impl>
struct impl_traits<const Impl> {
	typedef buffer::const_pointer		pointer;
	typedef buffer::const_iterator	iterator_type;
};

struct buffer::impl {
	struct encaps_state {
		buffer* out_;
		size_type		size_before_;
		size_type		buffer_before_;
		uint32_t		encoding_major = ENCODING_MAJOR;
		uint32_t		encoding_minor = ENCODING_MINOR;

		encaps_state(buffer* o)
			: out_(o),
			  size_before_(out_ ? out_->size() : 0),
			  buffer_before_(out_ ? out_->pimpl_->buffers_.size() - 1 : 0)
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
				buffer_type& b = out_->pimpl_->buffers_[buffer_before_];
				auto o = std::back_inserter(b);
				write(o, encoding_major);
				write(o, encoding_minor);
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

	buffer*				container_;
	message::message_flags	flags_;
	buffer_sequence_type	buffers_;
	encapsulation_stack		encapsulations_;

	impl(buffer* out)
		: container_(out),
		  flags_(message::request),
		  buffers_(2, buffer_type{}),
		  encapsulations_({encaps_state{ nullptr }})
	{
	}
	impl(buffer* out, message::message_flags flags)
		: container_(out),
		  flags_(flags),
		  buffers_(2, buffer_type{}),
		  encapsulations_({encaps_state{ nullptr }})
	{
	}
	impl(impl const& rhs)
		: container_(rhs.container_),
		  flags_(rhs.flags_),
		  buffers_(rhs.buffers_),
		  encapsulations_(rhs.encapsulations_)
	{
	}
	impl(buffer* out, impl const& rhs)
		: container_(out),
		  flags_(rhs.flags_),
		  buffers_(rhs.buffers_),
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
	back_buffer()
	{
		return buffers_.back();
	}

	buffer_type&
	message_header_buffer()
	{
		return buffers_.front();
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
	index_of(buffer_iterator< _pointer > const& iter) const
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
		if (buffers_.back().empty() && buffers_.size() > 2) {
			buffers_.pop_back();
		}
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
	insert_encaps(buffer&& encaps)
	{
		encapsulations_.emplace_back(container_);
		buffer_sequence_type buffers = std::move(encaps.pimpl_->buffers_);
		for( auto p = buffers.begin() + 1; p != buffers.end(); ++p) {
			if (p->size() > 0) {
				buffers_.push_back( std::move(*p) );
			}
		}
		encapsulations_.pop_back();
		start_buffer();
	}
};

buffer::buffer()
	: pimpl_(std::make_shared<impl>(this))
{
}
buffer::buffer(message::message_flags flags)
	: pimpl_(std::make_shared<impl>(this, flags))
{
}

buffer::buffer(buffer const& rhs)
	: pimpl_(std::make_shared<impl>(this, *rhs.pimpl_))
{
}

buffer::buffer(buffer&& rhs)
	: pimpl_(std::move(rhs.pimpl_))
{
	pimpl_->container_ = this;
}

void
buffer::swap(buffer& rhs)
{
	using std::swap;
	swap(pimpl_, rhs.pimpl_);
	swap(pimpl_->container_, rhs.pimpl_->container_);
}

buffer&
buffer::operator =(buffer const& rhs)
{
	buffer tmp(rhs);
	swap(tmp);
	return *this;
}

buffer&
buffer::operator =(buffer&& rhs)
{
	pimpl_ = std::move(rhs.pimpl_);
	rhs.pimpl_.reset();
	return *this;
}

buffer::size_type
buffer::size() const
{
	return pimpl_->size();
}

bool
buffer::empty() const
{
	return pimpl_->empty();
}

buffer::iterator
buffer::begin()
{
	return pimpl_->begin();
}
buffer::const_iterator
buffer::cbegin() const
{
	return pimpl_->cbegin();
}

buffer::iterator
buffer::end()
{
	return pimpl_->end();
}
buffer::const_iterator
buffer::cend() const
{
	return pimpl_->cend();
}

buffer::reverse_iterator
buffer::rbegin()
{
	return pimpl_->rbegin();
}
buffer::const_reverse_iterator
buffer::crbegin() const
{
	return pimpl_->crbegin();
}

buffer::reverse_iterator
buffer::rend()
{
	return pimpl_->rend();
}
buffer::const_reverse_iterator
buffer::crend() const
{
	return pimpl_->crend();
}

void
buffer::advance(iterator& iter, difference_type diff) const
{
	pimpl_->advance(iter, diff);
}

void
buffer::advance(const_iterator& iter, difference_type diff) const
{
	pimpl_->advance(iter, diff);
}

buffer::difference_type
buffer::difference(iterator const& a, iterator const& b) const
{
	return pimpl_->difference(a, b);
}
buffer::difference_type
buffer::difference(const_iterator const& a, const_iterator const& b) const
{
	return pimpl_->cdifference(a, b);
}

void
buffer::push_back(value_type v)
{
	pimpl_->push_back(v);
}

void
buffer::pop_back()
{
	pimpl_->pop_back();
}

buffer::asio_buffers
buffer::to_buffers() const
{
	return std::move(pimpl_->to_buffers());
}

void
buffer::insert_encapsulation(buffer&& encaps)
{
	pimpl_->insert_encaps(std::move(encaps));
}

buffer::encapsulation
buffer::begin_encapsulation()
{
	return std::move(encapsulation{this});
}

//----------------------------------------------------------------------------
//	encapsulation implementation
//----------------------------------------------------------------------------
struct buffer::encapsulation::impl {
	typedef buffer::impl::encaps_state		encaps_state;
	typedef buffer::impl::encaps_iterator		encaps_iter;

	buffer*		out_;
	encaps_iter		encaps_iter_;

	impl(buffer* o) :
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

buffer::encapsulation::encapsulation(buffer* o)
	: pimpl_(std::make_shared<impl>(o))
{
}

buffer::encapsulation::encapsulation(encapsulation&& rhs)
	: pimpl_(rhs.pimpl_)
{
	rhs.pimpl_.reset();
}

buffer::encapsulation::~encapsulation()
{
}

buffer::size_type
buffer::encapsulation::size() const
{
	return pimpl_->size();
}

}  // namespace encoding
}  // namespace wire
