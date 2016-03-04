/*
 * transport.inl
 *
 *  Created on: Feb 8, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_TRANSPORT_INL_
#define WIRE_CORE_TRANSPORT_INL_

#include <wire/core/transport.inl>

namespace wire {
namespace core {

template < typename Session, transport_type Type >
transport_listener< Session, Type >::transport_listener(
		asio_config::io_service_ptr svc, session_factory factory)
	: io_service_{svc}, acceptor_{*svc}, factory_(factory)
{
}

template < typename Session, transport_type Type >
void
transport_listener< Session, Type >::open(endpoint const& ep, bool rp)
{
	endpoint_type proto_ep = traits::create_endpoint(io_service_, ep);
	acceptor_.open(proto_ep.protocol());
	acceptor_.set_option( typename acceptor_type::reuse_address(true) );
	if (rp) {
		acceptor_.set_option( reuse_port(true) );
	}
	acceptor_.bind(proto_ep);
	acceptor_.listen();

	start_accept();
}

template < typename Session, transport_type Type >
typename transport_listener<Session, Type>::session_ptr
transport_listener<Session, Type>::create_session()
{
	return factory_( io_service_ );
}

template < typename Session, transport_type Type >
endpoint
transport_listener<Session, Type>::local_endpoint() const
{
	return traits::get_endpoint_data(acceptor_.local_endpoint());
}

template < typename Session, transport_type Type >
void
transport_listener<Session, Type>::start_accept()
{
	session_ptr session = create_session();
	acceptor_.async_accept(session->socket(),
			::std::bind(&transport_listener::handle_accept, this,
				session, ::std::placeholders::_1));
}

template < typename Session, transport_type Type >
void
transport_listener<Session, Type>::handle_accept(session_ptr session, asio_config::error_code const& ec)
{
	if (!ec) {
		session->start();
	}
	start_accept();
}

}  // namespace core
}  // namespace wire


#endif /* WIRE_CORE_TRANSPORT_INL_ */
