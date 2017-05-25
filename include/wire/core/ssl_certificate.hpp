/*
 * ssl_certificate.hpp
 *
 *  Created on: May 24, 2017
 *      Author: zmij
 */

#ifndef WIRE_CORE_SSL_CERTIFICATE_HPP_
#define WIRE_CORE_SSL_CERTIFICATE_HPP_

#include <openssl/ossl_typ.h>
#include <string>

#include <wire/core/ssl_certificate_fwd.hpp>

namespace wire {
namespace core {

/**
 * Wrapper for OpenSSL X509 structure
 */
class ssl_certificate {
public:
    ssl_certificate(X509* native)
        : native_{native} {}

    ::std::string
    subject_name() const;
private:
    X509* native_;
};

} /* namespace core */
} /* namespace wire */



#endif /* WIRE_CORE_SSL_CERTIFICATE_HPP_ */
