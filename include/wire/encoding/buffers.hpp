/*
 * buffers.hpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_BUFFERS_HPP_
#define TIP_WIRE_BUFFERS_HPP_

#include <wire/encoding/detail/wire_io.hpp>
#include <wire/encoding/detail/out_buffer_traits.hpp>

#include <memory>
#include <iterator>
#include <vector>

namespace wire {
namespace encoding {

class outgoing {
public:
	/** Internal buffers storage type */
	typedef std::vector<uint8_t>			buffer_type;
	/** Sequence of internal buffers */
	typedef std::vector<buffer_type>		buffer_sequence_type;
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
	class out_buff_iterator : public std::iterator< std::random_access_iterator_tag,
		value_type, difference_type,
		typename detail::out_buffer_traits<_pointer>::pointer,
		typename detail::out_buffer_traits<_pointer>::reference > {
	public:
		typedef detail::out_buffer_traits<_pointer>					out_buffer_traits;
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
		out_buff_iterator();
		out_buff_iterator(out_buff_iterator const&);
		template< typename T >
		out_buff_iterator(out_buff_iterator<T> const&);

		void
		swap(out_buff_iterator&);

		out_buff_iterator&
		operator = (out_buff_iterator const&);
		template< typename T >
		out_buff_iterator&
		operator = (out_buff_iterator<T> const&);

		bool
		operator == (out_buff_iterator const&) const;
		bool
		operator != (out_buff_iterator const&) const;

		//@{
		/** @name Forward iterator requirements */
		reference
		operator *() const;
		pointer
		operator ->() const;

		out_buff_iterator&
		operator ++();
		out_buff_iterator
		operator ++(int);
		//@}

		//@{
		/** @name Bidirectional iterator requirements */
		out_buff_iterator&
		operator --();
		out_buff_iterator
		operator --(int);
		//@}

		//@{
		/** @name Random access iterator requirements */
		/** @TODO Random access operator */
		out_buff_iterator&
		operator += (difference_type n);
		out_buff_iterator
		operator + (difference_type n) const;

		out_buff_iterator&
		operator -= (difference_type n);
		out_buff_iterator
		operator - (difference_type n) const;
		//@}
	private:
		friend class impl;
		out_buff_iterator(container_pointer c, buffer_iterator_type buff, value_iterator_type curr)
			: container_(c), buffer_(buff), current_(curr), position_(normal) {}
		out_buff_iterator(container_pointer c, iter_position pos)
			: container_(c), buffer_(), current_(), position_(pos) {}
	private:
		container_pointer		container_;
		buffer_iterator_type	buffer_;
		value_iterator_type		current_;
		iter_position			position_;
	};

	/** Normal input iterator for output buffer */
	typedef out_buff_iterator< pointer >				iterator;
	/** Constant input iterator for output buffer */
	typedef out_buff_iterator< const_pointer >			const_iterator;
	/** Reverse input iterator */
	typedef std::reverse_iterator< iterator >			reverse_iterator;
	/** Constant reverse input iterator */
	typedef std::reverse_iterator< const_iterator >		const_reverse_iterator;

	class encapsulation;
public:
	outgoing();
	/** Copy construct */
	outgoing(outgoing const&);
	/** Move construct */
	outgoing(outgoing&&);

	/** Swap contents */
	void
	swap(outgoing&);

	/** Copy assign */
	outgoing&
	operator = (outgoing const&);
	/** Move assign */
	outgoing&
	operator = (outgoing&&);
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

	//@{
	/** @name Encapsulated data */
	encapsulation&&
	begin_encapsulation();
	//@}
private:
	template < typename _pointer >
	friend class out_buff_iterator;
	friend class encapsulation;

	void
	advance(iterator& iter, difference_type diff) const;
	void
	advance(const_iterator& iter, difference_type diff) const;
private:
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

class outgoing::encapsulation {
public:
	~encapsulation();
	encapsulation(encapsulation&& rhs);
	encapsulation&
	operator = (encapsulation&& rhs);
private:
	friend class outgoing;
	encapsulation(outgoing* o);
	encapsulation(encapsulation const&) = delete;
	encapsulation&
	operator = (encapsulation const&) = delete;
private:
	outgoing* out_;
};

inline void
swap(outgoing& lhs, outgoing& rhs)
{
	lhs.swap(rhs);
}

template < typename InputIterator >
class incoming {
public:
	typedef InputIterator iterator_type;
public:
	incoming(iterator_type begin, iterator_type end);
private:
	incoming(incoming const&) = delete;
	incoming&
	operator = (incoming const&) = delete;
private:
	iterator_type current_;
	iterator_type end_;
};

}  // namespace encoding
}  // namespace wire

#include <wire/encoding/buffers.inl>

#endif /* TIP_WIRE_BUFFERS_HPP_ */
