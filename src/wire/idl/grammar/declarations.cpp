/*
 * declarations.cpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#include <wire/idl/grammar/declarations.hpp>

namespace wire {
namespace idl {
namespace grammar {

annotation_list::const_iterator
find(annotation_list const& list, ::std::string const& name)
{
    for (auto p = list.begin(); p != list.end(); ++p) {
        if (p->name == name)
            return p;
    }
    return list.end();
}

}  /* namespace grammar */
}  /* namespace idl */
namespace hash {

struct data_init_hash_32_calc : ::boost::static_visitor<::std::uint32_t> {
    using result_type = murmur_hash_calc<idl::grammar::data_initializer, 32>::result_type;

    result_type
    operator()(::std::string const& v) const noexcept
    {
        return murmur_hash_32(v);
    }

    result_type
    operator()(idl::grammar::data_initializer::initializer_list const& l) const noexcept
    {
        result_type h{0};
        for (auto init : l) {
            h = combine(*init, h);
        }
        return h;
    }
};

struct data_init_hash_calc : ::boost::static_visitor<::std::uint64_t> {
    using result_type = murmur_hash_calc<idl::grammar::data_initializer, 64>::result_type;

    result_type
    operator()(::std::string const& v) const noexcept
    {
        return murmur_hash_64(v);
    }

    result_type
    operator()(idl::grammar::data_initializer::initializer_list const& l) const noexcept
    {
        result_type h{0};
        for (auto init : l) {
            h = combine(*init, h);
        }
        return h;
    }
};

murmur_hash_calc<idl::grammar::data_initializer, 32>::result_type
murmur_hash_calc<idl::grammar::data_initializer, 32>::operator ()(argument_type arg) const noexcept
{
    return ::boost::apply_visitor(data_init_hash_32_calc{}, arg.value);
}

murmur_hash_calc<idl::grammar::data_initializer, 64>::result_type
murmur_hash_calc<idl::grammar::data_initializer, 64>::operator ()(argument_type arg) const noexcept
{
    return ::boost::apply_visitor(data_init_hash_calc{}, arg.value);
}

}  /* namespace hash */
}  /* namespace wire */
