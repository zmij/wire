/*
 * preprocess.hpp
 *
 *  Created on: 16 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_AST_PREPROCESS_HPP_
#define WIRE_AST_PREPROCESS_HPP_

#include <iosfwd>
#include <vector>
#include <string>

namespace wire {
namespace ast {

using string_list = ::std::vector< ::std::string >;

struct preprocess_options {
    string_list    include_dirs;
};

void
preprocess(::std::string const& file_name, ::std::ostream& os,
        preprocess_options const& options = preprocess_options{});

}  /* namespace ast */
}  /* namespace wire */



#endif /* WIRE_AST_PREPROCESS_HPP_ */
