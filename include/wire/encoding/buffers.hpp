/*
 * buffers.hpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_BUFFERS_HPP_
#define TIP_WIRE_BUFFERS_HPP_

#include <wire/encoding/wire_io.hpp>
#include <wire/encoding/message.hpp>
#include <wire/asio_config.hpp>
#include <wire/encoding/detail/buffer_traits.hpp>

#include <memory>
#include <iterator>
#include <vector>

namespace wire {
namespace encoding {

class buffer {
public:
	/** Internal buffers storage type */
	typedef std::vector<uint8_t>			buffer_type;
	/** Sequence of internal buffers */
	typedef std::vector<buffer_type>		buffer_sequence_type;
	typedef std::vector< ASIO_NS::const_buffer > asio_buffers;
	//@{
	/**
	 * @name Container concept
	 * http://en.cppreference.com/w/cpp/concept/Container
	 */
	typedef buffer_type::value_type			value_type;
	typedef buffer_type::reference			reference;
	typedef buffer_type::const_reference	const_reference;
	typedef buffer_type::pointer			pointer;
	typedef buffer_type::const_pointer		const_pointer;
	typedef buffer_type::difference_type	difference_type;
	typedef buffer_type::size_type			size_type;
	//@}
private:
	struct impl;
	enum iter_position {
		normal,
		after_end,
		before_begin
	};
public:
	/**
	 * Outgoing buffer iterator template
	 */
	template < typename _pointer >
	class buffer_iterator : public std::iterator< std::random_access_iterator_tag,
		value_type, difference_type,
		typename detail::buffer_traits<_pointer>::pointer,
		typename detail::buffer_traits<_pointer>::reference > {
	public:
		typedef detail::buffer_traits<_pointer>					out_buffer_traits;
		typedef typename out_buffer_traits::container_pointer		container_pointer;
		typedef typename out_buffer_traits::buffer_iterator_type	buffer_iterator_type;
		typedef typename out_buffer_traits::value_iterator_type		value_iterator_type;
		typedef std::iterator< std::random_access_iterator_tag,
				value_type, difference_type,
				typename out_buffer_traits::pointer,
				typename out_buffer_traits::reference >				iterator_type;
		typedef typename iterator_type::pointer						pointer;
		typedef typename iterator_type::reference					reference;
	public:
		buffer_iterator();
		buffer_iterator(buffer_iterator const&);
		template< typename T >
		buffer_iterator(buffer_iterator<T> const&);

		void
		swap(buffer_iterator&);

		buffer_iterator&
		operator = (buffer_iterator const&);
		template< typename T >
		buffer_iterator&
		operator = (buffer_iterator<T> const&);

		bool
		operator == (buffer_iterator const&) const;
		bool
		operator != (buffer_iterator const&) const;

		//@{
		/** @name Forward iterator requirements */
		reference
		operator *() const;
		pointer
		operator ->() const;

		buffer_iterator&
		operator ++();
		buffer_iterator
		operator ++(int);
		//@}

		//@{
		/** @name Bidirectional iterator requirements */
		buffer_iterator&
		operator --();
		buffer_iterator
		operator --(int);
		//@}

		//@{
		/** @name Random access iterator requirements */
		/** @TODO Random access operator */
		buffer_iterator&
		operator += (difference_type n);
		buffer_iterator
		operator + (difference_type n) const;

		buffer_iterator&
		operator -= (difference_type n);
		buffer_iterator
		operator - (difference_type n) const;
		template < typename _P >
		difference_type
		operator - (buffer_iterator<_P> const&) const;
		//@}
	private:
		friend class impl;
		template < typename T >
		friend class buffer_iterator;

		buffer_iterator(container_pointer c, buffer_iterator_type buff, value_iterator_type curr)
			: container_(c), buffer_(buff), current_(curr), position_(normal) {}
		buffer_iterator(container_pointer c, iter_position pos)
			: container_(c), buffer_(), current_(), position_(pos) {}
	private:
		container_pointer		container_;
		buffer_iterator_type	buffer_;
		value_iterator_type		current_;
		iter_position			position_;
	};

	/** Normal input iterator for output buffer */
	typedef buffer_iterator< pointer >				iterator;
	/** Constant input iterator for output buffer */
	typedef buffer_iterator< const_pointer >			const_iterator;
	/** Reverse input iterator */
	typedef std::reverse_iterator< iterator >			reverse_iterator;
	/** Constant reverse input iterator */
	typedef std::reverse_iterator< const_iterator >		const_reverse_iterator;

	class encapsulation;
public:
	buffer();
	buffer(message::message_flags);
	/** Copy construct */
	buffer(buffer const&);
	/** Move construct */
	buffer(buffer&&);

	/** Swap contents */
	void
	swap(buffer&);

	/** Copy assign */
	buffer&
	operator = (buffer const&);
	/** Move assign */
	buffer&
	operator = (buffer&&);
	//@{
	/**
	 * @name Container concept
	 * http://en.cppreference.com/w/cpp/concept/Container
	 */
	size_type
	size() const;
	size_type
	max_size() const;
	bool
	empty() const;

	iterator
	begin();
	inline const_iterator
	begin() const
	{ return cbegin(); }
	const_iterator
	cbegin() const;

	iterator
	end();
	inline const_iterator
	end() const
	{ return cend(); }

	const_iterator
	cend() const;
	//@}

	//@{
	/** @name ReversibleContainer concept */
	reverse_iterator
	rbegin();
	inline const_reverse_iterator
	rbegin() const
	{ return crbegin(); }
	const_reverse_iterator
	crbegin() const;

	reverse_iterator
	rend();
	inline const_reverse_iterator
	rend() const
	{ return crend(); }
	const_reverse_iterator
	crend() const;
	//@}

	//@{
	/**
	 * @name SequenceContainer concept
	 * http://en.cppreference.com/w/cpp/concept/SequenceContainer
	 */
	reference
	front();
	const_reference
	front() const;

	reference
	back();
	const_reference
	back() const;

	void
	push_back(value_type);
	void
	pop_back();

	reference
	operator[] (size_type);
	const_reference
	operator[] (size_type) const;

	reference
	at(size_type);
	const_reference
	at(size_type) const;
	//@}

	/**
	 * Get asio buffers for output
	 * @return
	 */
	asio_buffers
	to_buffers() const;

	//@{
	/** @name Encapsulated data */
	void
	insert_encapsulation(buffer&&);
	encapsulation
	begin_encapsulation();
	//@}
private:
	template < typename _pointer >
	friend class buffer_iterator;
	friend class encapsulation;

	void
	advance(iterator& iter, difference_type diff) const;
	void
	advance(const_iterator& iter, difference_type diff) const;

	difference_type
	difference(iterator const& a, iterator const& b) const;
	difference_type
	difference(const_iterator const& a, const_iterator const& b) const;
private:
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

typedef std::shared_ptr< buffer > buffer_ptr;

class buffer::encapsulation {
public:
	encapsulation(encapsulation&& rhs);
	~encapsulation();
	encapsulation&
	operator = (encapsulation&& rhs);

	size_type
	size() const;
private:
	friend class buffer;
	encapsulation(buffer* o);
	encapsulation(encapsulation const&) = delete;
	encapsulation&
	operator = (encapsulation const&) = delete;
private:
	struct impl;
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

inline void
swap(buffer& lhs, buffer& rhs)
{
	lhs.swap(rhs);
}

}  // namespace encoding
}  // namespace wire

#include <wire/encoding/buffers.inl>

#endif /* TIP_WIRE_BUFFERS_HPP_ */
