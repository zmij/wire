/*
 * ssl_certificate.cpp
 *
 *  Created on: May 24, 2017
 *      Author: zmij
 */

#include <wire/core/ssl_certificate.hpp>
#include <openssl/x509.h>

namespace wire {
namespace core {

::std::string
ssl_certificate::subject_name() const
{
    char sn[256];
    X509_NAME_oneline(X509_get_subject_name(native_), sn, 256);
    return ::std::string{sn};
}

} /* namespace core */
} /* namespace wire */


