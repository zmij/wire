/*
 * qname.hpp
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_QNAME_HPP_
#define WIRE_IDL_QNAME_HPP_

#include <string>
#include <vector>

namespace wire {
namespace idl {

/**
 * Qualified name.
 * Can be fully qualified, can be relative.
 */
struct qname {
    using string_seq = ::std::vector< ::std::string >;
    using const_iterator = string_seq::const_iterator;

    bool       fully = false;
    string_seq components;

    qname() {}
    qname(char const* name);
    /**
     * Intentionally implicit
     * @param name Identifier string, optionally scope-resolved
     */
    qname(::std::string const& name);

    qname(qname const&) = default;
    qname(qname&&) = default;

    qname&
    operator = (qname const&) = default;
    qname&
    operator = (qname&&) = default;

    bool
    empty() const
    { return components.empty(); }

    ::std::size_t
    size() const
    { return components.size(); }

    const_iterator
    begin() const
    { return components.begin(); }
    const_iterator
    end() const
    { return components.end(); }

    ::std::string const&
    name() const
    {
        if (empty()) {
            throw ::std::runtime_error("Name is empty");
        }
        return components.back();
    }

    static qname
    parse(::std::string const& name);
};

struct qname_search {
    using string_seq = qname::string_seq;
    using iterator = qname::const_iterator;

    bool fully;
    iterator begin;
    iterator end;

    bool
    empty() const
    { return begin == end; }

    qname_search
    next() const
    { return { fully, begin + 1, end }; }

    qname_search
    next(bool f) const
    { return { f, begin + 1, end }; }
};

}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_QNAME_HPP_ */
