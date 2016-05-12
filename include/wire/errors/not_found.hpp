/*
 * not_found.hpp
 *
 *  Created on: May 12, 2016
 *      Author: zmij
 */

#ifndef WIRE_ERRORS_NOT_FOUND_HPP_
#define WIRE_ERRORS_NOT_FOUND_HPP_

#include <wire/errors/exceptions.hpp>
#include <wire/core/identity.hpp>

namespace wire {
namespace errors {

class not_found : public runtime_error {
public:
    enum subject { object, facet, operation };
public:
    not_found(subject s, core::identity const& obj_id,
            ::std::string const& fct, ::std::string const& op)
        : runtime_error(format_message(s, obj_id, fct, op)),
          subj_{s}, object_id_{obj_id}, facet_{fct}, operation_{op}
    {}

    subject
    subj() const
    { return subj_; }

    core::identity const&
    object_id() const
    { return object_id_; }

    ::std::string const&
    get_facet() const
    { return facet_; }

    ::std::string const&
    get_operation() const
    {
        return operation_;
    }

    static ::std::string
    format_message(subject s, core::identity const& obj_id,
            ::std::string const& facet, ::std::string const& operation);
private:
    subject         subj_;
    core::identity  object_id_;
    ::std::string   facet_;
    ::std::string   operation_;
};


class no_object : public not_found {
public:
    no_object(core::identity const& obj_id,
            ::std::string const& fct, ::std::string const& op)
        : not_found{ object, obj_id, fct, op } {}
};

class no_operation : public not_found {
public:
    no_operation(core::identity const& obj_id,
            ::std::string const& fct, ::std::string const& op)
        : not_found{ operation, obj_id, fct, op } {}
};

class no_facet : public not_found {
public:
    no_facet(core::identity const& obj_id,
            ::std::string const& fct, ::std::string const& op)
        : not_found{ facet, obj_id, fct, op } {}
};

}  /* namespace errors */
}  /* namespace wire */



#endif /* WIRE_ERRORS_NOT_FOUND_HPP_ */
