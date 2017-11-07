/*
 * options.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef TRANSPORT_OPTIONS_HPP_
#define TRANSPORT_OPTIONS_HPP_

#include <string>

namespace wire {
namespace test {
namespace sparring {

class options {
public:
    static options&
    instance();
public:
    std::string sparring_partner;
protected:
    options() {}
private:
    options(options const&) = delete;
    options&
    operator = (options const&) = delete;
};

} /* namespace transport */
} /* namespace test */
} /* namespace wire */

#endif /* TRANSPORT_OPTIONS_HPP_ */
