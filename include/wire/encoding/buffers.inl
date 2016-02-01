/*
 * buffers.inl
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_BUFFERS_INL_
#define WIRE_ENCODING_BUFFERS_INL_

#include <wire/encoding/buffers.hpp>

namespace wire {
namespace encoding {

template < typename _pointer >
outgoing::buffer_iterator< _pointer >::buffer_iterator()
	: container_(nullptr), buffer_(), current_(), position_(after_end)
{
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >::buffer_iterator(buffer_iterator const& rhs)
	: container_(rhs.container_), buffer_(rhs.buffer_),
	  current_(rhs.current_), position_(rhs.position_)
{
}

template < typename _pointer >
template < typename T >
outgoing::buffer_iterator< _pointer >::buffer_iterator(buffer_iterator<T> const& rhs)
	: container_(rhs.container_), buffer_(rhs.buffer_),
	  current_(rhs.current_), position_(rhs.position_)
{
}

template < typename _pointer >
void
outgoing::buffer_iterator< _pointer >::swap(buffer_iterator& rhs)
{
	using std::swap;
	swap(container_, rhs.container_);
	swap(buffer_, rhs.buffer_);
	swap(current_, rhs.current_);
	swap(position_, rhs.position_);
}

template <typename _pointer >
outgoing::buffer_iterator< _pointer >&
outgoing::buffer_iterator< _pointer >::operator =(buffer_iterator const& rhs)
{
	buffer_iterator tmp(rhs);
	swap(tmp);
	return *this;
}

template <typename _pointer >
template <typename T >
outgoing::buffer_iterator< _pointer >&
outgoing::buffer_iterator< _pointer >::operator =(buffer_iterator<T> const& rhs)
{
	buffer_iterator tmp(rhs);
	swap(tmp);
	return *this;
}

template < typename _pointer >
bool
outgoing::buffer_iterator< _pointer >::operator ==(buffer_iterator const& rhs) const
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

template < typename _pointer >
bool
outgoing::buffer_iterator< _pointer >::operator !=(buffer_iterator const& rhs) const
{
	return !(*this == rhs);
}

template < typename _pointer >
typename outgoing::buffer_iterator< _pointer >::reference
outgoing::buffer_iterator< _pointer >::operator *() const
{
	assert(position_ == normal && container_ && "Iterator is valid");
	return *current_;
}

template < typename _pointer >
typename outgoing::buffer_iterator< _pointer >::pointer
outgoing::buffer_iterator< _pointer >::operator -> () const
{
	assert(position_ == normal && container_ && "Iterator is valid");
	return current_.operator -> ();
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >&
outgoing::buffer_iterator< _pointer >::operator ++()
{
	container_->advance(*this, 1);
	return *this;
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >
outgoing::buffer_iterator< _pointer >::operator ++(int)
{
	buffer_iterator prev(*this);
	++(*this);
	return prev;
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >&
outgoing::buffer_iterator< _pointer >::operator --()
{
	container_->advance(*this, -1);
	return *this;
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >
outgoing::buffer_iterator< _pointer >::operator --(int)
{
	buffer_iterator prev(*this);
	--(*this);
	return prev;
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >&
outgoing::buffer_iterator< _pointer >::operator +=( difference_type n )
{
	container_->advance(*this, n);
	return *this;
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >
outgoing::buffer_iterator< _pointer >::operator + ( difference_type n ) const
{
	buffer_iterator res(*this);
	return (res += n);
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >&
outgoing::buffer_iterator< _pointer >::operator -=( difference_type n )
{
	container_->advance(*this, -n);
	return *this;
}

template < typename _pointer >
outgoing::buffer_iterator< _pointer >
outgoing::buffer_iterator< _pointer >::operator - ( difference_type n ) const
{
	buffer_iterator res(*this);
	return (res -= n);
}

template < typename _pointer >
template < typename _P >
outgoing::difference_type
outgoing::buffer_iterator< _pointer >::operator -(buffer_iterator<_P> const& rhs) const
{
	return container_->difference(*this, rhs);
}

}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_BUFFERS_INL_ */
