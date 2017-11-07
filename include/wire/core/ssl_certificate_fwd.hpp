/*
 * ssl_certificate_fwd.hpp
 *
 *  Created on: May 24, 2017
 *      Author: zmij
 */

#ifndef WIRE_CORE_SSL_CERTIFICATE_FWD_HPP_
#define WIRE_CORE_SSL_CERTIFICATE_FWD_HPP_

#include <functional>

namespace wire {
namespace core {

class ssl_certificate;

using ssl_verify_callback = ::std::function< bool(bool, ssl_certificate const&) >;

} /* namespace core */
} /* namespace wire */



#endif /* WIRE_CORE_SSL_CERTIFICATE_FWD_HPP_ */
