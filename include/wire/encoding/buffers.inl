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
outgoing::out_buff_iterator< _pointer >::out_buff_iterator()
	: container_(nullptr), buffer_(), current_(), position_(after_end)
{
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >::out_buff_iterator(out_buff_iterator const& rhs)
	: container_(rhs.container_), buffer_(rhs.buffer_),
	  current_(rhs.current_), position_(rhs.position_)
{
}

template < typename _pointer >
template < typename T >
outgoing::out_buff_iterator< _pointer >::out_buff_iterator(out_buff_iterator<T> const& rhs)
	: container_(rhs.container_), buffer_(rhs.buffer_),
	  current_(rhs.current_), position_(rhs.position_)
{
}

template < typename _pointer >
void
outgoing::out_buff_iterator< _pointer >::swap(out_buff_iterator& rhs)
{
	using std::swap;
	swap(container_, rhs.container_);
	swap(buffer_, rhs.buffer_);
	swap(current_, rhs.current_);
	swap(position_, rhs.position_);
}

template <typename _pointer >
outgoing::out_buff_iterator< _pointer >&
outgoing::out_buff_iterator< _pointer >::operator =(out_buff_iterator const& rhs)
{
	out_buff_iterator tmp(rhs);
	swap(tmp);
	return *this;
}

template <typename _pointer >
template <typename T >
outgoing::out_buff_iterator< _pointer >&
outgoing::out_buff_iterator< _pointer >::operator =(out_buff_iterator<T> const& rhs)
{
	out_buff_iterator tmp(rhs);
	swap(tmp);
	return *this;
}

template < typename _pointer >
bool
outgoing::out_buff_iterator< _pointer >::operator ==(out_buff_iterator const& rhs) const
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
outgoing::out_buff_iterator< _pointer >::operator !=(out_buff_iterator const& rhs) const
{
	return !(*this == rhs);
}

template < typename _pointer >
typename outgoing::out_buff_iterator< _pointer >::reference
outgoing::out_buff_iterator< _pointer >::operator *() const
{
	return *current_;
}

template < typename _pointer >
typename outgoing::out_buff_iterator< _pointer >::pointer
outgoing::out_buff_iterator< _pointer >::operator -> () const
{
	return current_.operator -> ();
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >&
outgoing::out_buff_iterator< _pointer >::operator ++()
{
	container_->advance(buffer_, current_, 1);
	return *this;
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >
outgoing::out_buff_iterator< _pointer >::operator ++(int)
{
	out_buff_iterator prev(*this);
	++(*this);
	return prev;
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >&
outgoing::out_buff_iterator< _pointer >::operator --()
{
	container_->advance(buffer_, current_, -1);
	return *this;
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >
outgoing::out_buff_iterator< _pointer >::operator --(int)
{
	out_buff_iterator prev(*this);
	--(*this);
	return prev;
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >&
outgoing::out_buff_iterator< _pointer >::operator +=( difference_type n )
{
	container_->advance(buffer_, current_, n);
	return *this;
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >
outgoing::out_buff_iterator< _pointer >::operator + ( difference_type n ) const
{
	out_buff_iterator res(*this);
	return (res += n);
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >&
outgoing::out_buff_iterator< _pointer >::operator -=( difference_type n )
{
	container_->advance(buffer_, current_, -n);
	return *this;
}

template < typename _pointer >
outgoing::out_buff_iterator< _pointer >
outgoing::out_buff_iterator< _pointer >::operator - ( difference_type n ) const
{
	out_buff_iterator res(*this);
	return (res -= n);
}

}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_BUFFERS_INL_ */
