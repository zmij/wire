/*
 * preprocess.hpp
 *
 *  Created on: 16 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_PREPROCESS_HPP_
#define WIRE_IDL_PREPROCESS_HPP_

#include <iosfwd>
#include <vector>
#include <string>
#include <memory>

namespace wire {
namespace idl {

using string_list = ::std::vector< ::std::string >;

struct preprocess_options {
    string_list    include_dirs;
    bool           output_comments;
    bool           verbose;
};

class preprocessor {
public:
    explicit
    preprocessor(::std::string const& file_name,
        preprocess_options const& options = preprocess_options{});

    ::std::istream&
    stream();

    ::std::string
    to_string();
private:
    struct impl;
    using pimpl = ::std::shared_ptr< impl >;
    pimpl pimpl_;
};

}  /* namespace ast */
}  /* namespace wire */



#endif /* WIRE_IDL_PREPROCESS_HPP_ */
