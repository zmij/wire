/*
 * buffer_iterator.inl
 *
 *  Created on: 2 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_INL_
#define WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_INL_

#include <wire/encoding/detail/buffer_iterator.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >::buffer_iterator()
	: container_(nullptr), buffer_(), current_(), position_(after_end)
{
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >::buffer_iterator(buffer_iterator const& rhs)
	: container_(rhs.container_), buffer_(rhs.buffer_),
	  current_(rhs.current_), position_(rhs.position_)
{
}

template < typename Container, typename Pointer >
template < typename T >
buffer_iterator< Container, Pointer >::buffer_iterator(buffer_iterator<Container, T> const& rhs)
	: container_(rhs.container_), buffer_(rhs.buffer_),
	  current_(rhs.current_), position_(rhs.position_)
{
}

template < typename Container, typename Pointer >
void
buffer_iterator< Container, Pointer >::swap(buffer_iterator& rhs)
{
	using std::swap;
	swap(container_, rhs.container_);
	swap(buffer_, rhs.buffer_);
	swap(current_, rhs.current_);
	swap(position_, rhs.position_);
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >&
buffer_iterator< Container, Pointer >::operator =(buffer_iterator const& rhs)
{
	buffer_iterator tmp(rhs);
	swap(tmp);
	return *this;
}

template < typename Container, typename Pointer >
template <typename T >
buffer_iterator< Container, Pointer >&
buffer_iterator< Container, Pointer >::operator =(buffer_iterator<Container, T> const& rhs)
{
	buffer_iterator tmp(rhs);
	swap(tmp);
	return *this;
}

template < typename Container, typename Pointer >
bool
buffer_iterator< Container, Pointer >::operator ==(buffer_iterator const& rhs) const
{
	if (container_ != rhs.container_) {
		return false;
	}
	if (!container_) {
		// Empty iterators
		return true;
	}
	if (position_ != normal || rhs.position_ != normal) {
		return position_ == rhs.position_;
	}

	return buffer_ == rhs.buffer_ && current_ == rhs.current_;
}

template < typename Container, typename Pointer >
bool
buffer_iterator< Container, Pointer >::operator !=(buffer_iterator const& rhs) const
{
	return !(*this == rhs);
}

template < typename Container, typename Pointer >
typename buffer_iterator< Container, Pointer >::reference
buffer_iterator< Container, Pointer >::operator *() const
{
	assert(position_ == normal && container_ && "Iterator is valid");
	return *current_;
}

template < typename Container, typename Pointer >
typename buffer_iterator< Container, Pointer >::pointer
buffer_iterator< Container, Pointer >::operator -> () const
{
	assert(position_ == normal && container_ && "Iterator is valid");
	return current_.operator -> ();
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >&
buffer_iterator< Container, Pointer >::operator ++()
{
	assert(container_ && "Iterator is bound to a container");
	if (position_ == normal) {
		++current_;
		while (current_ == buffer_->end()) {
			++buffer_;
			if (buffer_ == container_->buffers_.end()) {
				container_->end().swap(*this);
				break;
			}
			current_ = buffer_->begin();
		}
	} else if (position_ == before_begin) {
		container_->begin().swap(*this);
	}
	return *this;
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >
buffer_iterator< Container, Pointer >::operator ++(int)
{
	buffer_iterator prev(*this);
	++(*this);
	return prev;
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >&
buffer_iterator< Container, Pointer >::operator --()
{
	assert(container_ && "Iterator is bound to a container");
	if (position_ == normal) {
		if (current_ == buffer_->begin()) {
			if (buffer_ == container_->buffers_.begin()) {
				buffer_iterator{ container_, before_begin }.swap(*this);
			} else {
				--buffer_;
				for (; buffer_ != container_->buffers_.begin() && buffer_->empty(); --buffer_);
				if (buffer_ == container_->buffers_.begin() && buffer_->empty()) {
					buffer_iterator{ container_, before_begin }.swap(*this);
				} else {
					current_ = buffer_->end() - 1;
				}
			}

		} else {
			--current_;
		}
	} else if (position_ == after_end) {
		container_->last().swap(*this);
	}
	return *this;
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >
buffer_iterator< Container, Pointer >::operator --(int)
{
	buffer_iterator prev(*this);
	--(*this);
	return prev;
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >&
buffer_iterator< Container, Pointer >::operator +=(difference_type n)
{
	container_->advance(*this, n);
	return *this;
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >
buffer_iterator< Container, Pointer >::operator + (difference_type n) const
{
	buffer_iterator res(*this);
	return (res += n);
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >&
buffer_iterator< Container, Pointer >::operator -=(difference_type n)
{
	container_->advance(*this, -n);
	return *this;
}

template < typename Container, typename Pointer >
buffer_iterator< Container, Pointer >
buffer_iterator< Container, Pointer >::operator - (difference_type n) const
{
	buffer_iterator res(*this);
	return (res -= n);
}

template < typename Container, typename Pointer >
template < typename T >
typename buffer_iterator< Container, Pointer >::difference_type
buffer_iterator< Container, Pointer >::operator - (buffer_iterator<Container, T> const& rhs) const
{
	return container_->difference(*this, rhs);
}

}  // namespace detail
}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_DETAIL_BUFFER_ITERATOR_INL_ */
