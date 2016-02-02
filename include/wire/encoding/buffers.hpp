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
#include <memory>
#include <iterator>
#include <vector>
#include "detail/buffer_iterator.hpp"

namespace wire {
namespace encoding {

class outgoing {
public:
	/** Internal buffers storage type */
	typedef detail::buffer_sequence::buffer_type			buffer_type;
	/** Sequence of internal buffers */
	typedef detail::buffer_sequence::buffer_sequence_type	buffer_sequence_type;
	typedef std::vector< ASIO_NS::const_buffer > asio_buffers;
	//@{
	/**
	 * @name Container concept
	 * http://en.cppreference.com/w/cpp/concept/Container
	 */
	typedef detail::buffer_sequence::value_type			value_type;
	typedef detail::buffer_sequence::reference			reference;
	typedef detail::buffer_sequence::const_reference	const_reference;
	typedef detail::buffer_sequence::pointer			pointer;
	typedef detail::buffer_sequence::const_pointer		const_pointer;
	typedef detail::buffer_sequence::difference_type	difference_type;
	typedef detail::buffer_sequence::size_type			size_type;
	//@}
public:
	/** Normal input iterator for output buffer */
	typedef detail::buffer_sequence::iterator			iterator;
	/** Constant input iterator for output buffer */
	typedef detail::buffer_sequence::const_iterator		const_iterator;
	/** Reverse input iterator */
	typedef std::reverse_iterator< iterator >					reverse_iterator;
	/** Constant reverse input iterator */
	typedef std::reverse_iterator< const_iterator >				const_reverse_iterator;

	class encapsulation;
public:
	outgoing();
	outgoing(message::message_flags);
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

	/**
	 * Get asio buffers for output
	 * @return
	 */
	asio_buffers
	to_buffers() const;

	//@{
	/** @name Encapsulated data */
	void
	insert_encapsulation(outgoing&&);
	encapsulation
	begin_encapsulation();
	//@}
private:
	friend class encapsulation;
private:
	struct impl;
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

typedef std::shared_ptr< outgoing > outgoing_ptr;

class outgoing::encapsulation {
public:
	encapsulation(encapsulation&& rhs);
	~encapsulation();
	encapsulation&
	operator = (encapsulation&& rhs);

	size_type
	size() const;
private:
	friend class outgoing;
	encapsulation(outgoing* o);
	encapsulation(encapsulation const&) = delete;
	encapsulation&
	operator = (encapsulation const&) = delete;
private:
	struct impl;
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

inline void
swap(outgoing& lhs, outgoing& rhs)
{
	lhs.swap(rhs);
}

class incoming {
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
public:
	incoming(message const&);
	template < typename InputIterator >
	incoming(message const&, InputIterator begin, InputIterator end);
	incoming(message const&, buffer_type const&);
	incoming(message const&, buffer_type&&);

	template < typename InputIterator >
	void
	insert_back(InputIterator begin, InputIterator end);
	void
	insert_back(buffer_type const&);
	void
	insert_back(buffer_type&&);

	bool
	empty() const;
	size_type
	size() const;
private:
	void
	create_pimpl(message const&);

	buffer_type&
	back_buffer();
private:
	struct impl;
	typedef std::shared_ptr<impl> pimpl;
	pimpl pimpl_;
};

}  // namespace encoding
}  // namespace wire

#include <wire/encoding/buffers.inl>

#endif /* TIP_WIRE_BUFFERS_HPP_ */
