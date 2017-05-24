/*
 * exceptions.cpp
 *
 *  Created on: May 12, 2016
 *      Author: zmij
 */

#include <wire/errors/exceptions.hpp>
#include <wire/errors/not_found.hpp>

#include <iostream>
#include <sstream>

namespace wire {
namespace errors {

namespace detail {

struct op_id {
    ::wire::encoding::operation_specs::operation_id value;
};

struct op_id_printer {
    using result_type = void;
    ::std::ostream& os;
    void
    operator()( ::wire::encoding::operation_specs::hash_type v ) const
    {
        os << "0x" << ::std::hex << v.value;
    }
    void
    operator()(::std::string const& str) const
    {
        os << str;
    }
};

::std::ostream&
operator << (::std::ostream& os, op_id const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        op_id_printer vis{os};
        val.value.apply_visitor(vis);
    }
    return os;
}

}  /* namespace detail */

char const*
runtime_error::what() const noexcept
{
    if (message_.empty()) {
        // call stream message
        ::std::ostringstream os;
        stream_message(os);
        message_ = os.str();
    }
    return message_.c_str();
}

void
runtime_error::stream_message(::std::ostream& os) const
{
    os << ::std::runtime_error::what();
}

void
not_found::stream_message(::std::ostream& os) const
{
    os << format_message(subj_, object_id_, facet_, operation_);
}

::std::string
not_found::format_message(subject s, core::identity const& obj_id,
        ::std::string const& f, operation_id const& op)
{
    ::std::ostringstream os;

    switch (s) {
        case object:
            os << "Object ";
            break;
        case facet:
            os << "Facet ";
            break;
        case operation:
            os << "Operation ";
            break;
        default:
            break;
    }

    os << "not found. Id: '" << obj_id << "' Facet: '" << f
            << "' Operation: " << detail::op_id{ op };

    return os.str();
}

}  /* namespace errors */
}  /* namespace wire */

