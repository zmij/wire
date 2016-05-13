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
#include <stdexcept>

namespace wire {
namespace idl {

/**
 * Object for searching for a partial qname
 */
struct qname_search {
    using string_seq = ::std::vector< ::std::string >;
    using const_iterator = string_seq::const_iterator;

    bool            fully;
    const_iterator  begin;
    const_iterator  end;

    bool
    empty() const
    { return begin == end; }

    ::std::size_t
    size() const
    { return static_cast<::std::size_t>(::std::distance(begin, end)); }

    ::std::string const&
    front() const
    {
        if (empty())
            throw ::std::range_error("Qname search object is emtpy (get front)");
        return *begin;
    }
    ::std::string const&
    back() const
    {
        if (empty())
            throw ::std::range_error("Qname search object is empty (get name)");
        auto back = end - 1;
        return *back;
    }

    qname_search
    next() const
    {
        if (empty())
            throw ::std::range_error("Qname search object is empty (get next)");
        return { false, begin + 1, end };
    }

    qname_search
    last() const
    {
        if (empty())
            throw ::std::range_error("Qname search object is empty (get last)");
        return { false, end - 1, end};
    }
    qname_search
    scope() const
    {
        if (empty())
            throw ::std::range_error("Qname search object is empty (get scope)");
        return {fully, begin, end - 1};
    }

    qname_search&
    operator++()
    {
        fully = false;
        ++begin;
        return *this;
    }
};

/**
 * Qualified name.
 * Can be fully qualified, can be relative.
 */
struct qname {
    using string_seq = qname_search::string_seq;
    using const_iterator = qname_search::const_iterator;

    bool       fully = false;
    string_seq components;

    qname() {}
    qname(bool f) : fully(f) {}
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
    operator == (qname const& rhs) const;
    bool
    operator != (qname const& rhs) const
    {
        return !(*this == rhs);
    }
    qname&
    operator += (qname const&);

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
            throw ::std::runtime_error("Name is empty (qualified name)");
        }
        return components.back();
    }

    qname_search
    search() const
    { return { fully, components.begin(), components.end() }; }
    qname_search
    search( bool f) const
    { return { f, components.begin(), components.end() }; }

    static qname
    parse(::std::string const& name);
};

::std::ostream&
operator << (::std::ostream& os, qname const& val);
::std::ostream&
operator << (::std::ostream& os, qname_search const& val);

inline qname
operator + (qname const& lhs, qname const& rhs)
{
    qname tmp{lhs};
    return tmp += rhs;
}

}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_QNAME_HPP_ */
