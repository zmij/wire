/*
 * timed_cache.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_TIMED_CACHE_HPP_
#define WIRE_UTIL_TIMED_CACHE_HPP_

#include <memory>
#include <chrono>

namespace wire {
namespace util {

namespace detail {

template < typename Clock = ::std::chrono::system_clock >
class timed_cache_base {
public:
    using clock     = Clock;
    using timepoint = typename clock::time_point;
    using duration  = typename clock::duration;

    timed_cache_base(duration const& cache_time = ::std::chrono::seconds{1})
        : tm_{ clock::now() }, max_age_{ cache_time } {}
    timed_cache_base(timed_cache_base const& rhs)
        : tm_{ rhs.tm_ }, max_age_{ rhs.max_age_ } {}
    timed_cache_base(timed_cache_base&& rhs)
        : tm_{ ::std::move(rhs.tm_) }, max_age_{ ::std::move(rhs.max_age_) } {}

    void
    swap(timed_cache_base& rhs) noexcept
    {
        using ::std::swap;
        swap(tm_, rhs.tm_);
        swap(max_age_, rhs.age_);
    }

    timed_cache_base&
    operator = (timed_cache_base const& rhs)
    {
        timed_cache_base tmp{rhs};
        swap(tmp);
        return *this;
    }
    timed_cache_base&
    operator = (timed_cache_base&& rhs)
    {
        timed_cache_base tmp{::std::move(rhs)};
        swap(tmp);
        return *this;
    }


    bool
    stale() const
    {
        return clock::now() - tm_ > max_age_;
    }

    void
    refresh()
    {
        tm_ = clock::now();
    }
private:
    timepoint   tm_;
    duration    max_age_;
};

}  /* namespace detail */

template < typename T, typename Clock = ::std::chrono::system_clock >
class timed_cache : detail::timed_cache_base<Clock> {
public:
    using base_type         = detail::timed_cache_base<Clock>;
    using duration          = typename base_type::duration;
    using value_type        = T;
    using pointer           = T*;
    using const_pointer     = T const*;
    using reference         = T&;
    using const_reference   = T const&;
public:
    timed_cache(duration const& max_age = ::std::chrono::seconds{1})
        : base_type{max_age}, value_{} {}
    explicit
    timed_cache(pointer v,
            duration const& max_age = ::std::chrono::seconds{1})
        : base_type{max_age}, value_{v} {}
    explicit
    timed_cache(const_reference v,
            duration const& max_age = ::std::chrono::seconds{1})
        : base_type{max_age}, value_{ ::std::make_shared<value_type>(v) } {}
    explicit
    timed_cache(::std::shared_ptr<value_type> v,
            duration const& max_age = ::std::chrono::seconds{1})
        : base_type{max_age}, value_{v} {}

    timed_cache(timed_cache const& rhs)
        : base_type{rhs}, value_{rhs.value_}
    {
    }
    timed_cache(timed_cache&& rhs)
        : base_type{::std::move(rhs)}, value_{::std::move(rhs.value_)}
    {
    }

    void
    swap(timed_cache& rhs)
    {
        using ::std::swap;
        base_type::swap(rhs);
        swap(value_, rhs.value_);
    }

    timed_cache&
    operator = (timed_cache const& rhs)
    {
        timed_cache tmp{rhs};
        swap(tmp);
        return *this;
    }
    timed_cache&
    operator = (timed_cache&& rhs)
    {
        timed_cache tmp{::std::move(rhs)};
        swap(tmp);
        return *this;
    }

    timed_cache&
    operator = (pointer v)
    {
        value_.reset(v);
        refresh();
        return *this;
    }

    timed_cache&
    operator = (reference v)
    {
        value_ = ::std::make_shared<value_type>(v);
        refresh();
        return *this;
    }

    timed_cache&
    operator = (::std::shared_ptr<value_type> v)
    {
        value_ = v;
        refresh();
        return *this;
    }

    bool
    operator == (timed_cache const& rhs) const
    {
        return value_ == rhs.value_;
    }
    bool
    operator != (timed_cache const& rhs) const
    {
        return value_ != rhs.value_;
    }

    using base_type::stale;

    operator ::std::shared_ptr<value_type>() const
    {
        if (stale())
            return ::std::shared_ptr<value_type>{};
        return value_;
    }

    operator bool() const
    { return value_ && !stale(); }

    pointer
    get()
    { return value_.get(); }
    const_pointer
    get() const
    { return value_.get(); }

    pointer
    operator ->()
    { return get(); }
    const_pointer
    operator ->() const
    { return get(); }

    reference
    operator *()
    { return *get(); }
    const_reference
    operator *() const
    { return *get(); }
private:
    using base_type::refresh;
private:
    using storage_type  = ::std::shared_ptr<T>;
    storage_type    value_;
};

template < typename T, typename ... Args >
timed_cache<T>
make_timed_cache(Args&& ... args)
{
    return timed_cache<T>(::std::make_shared(::std::forward<Args>(args)...));
}

}  /* namespace util */
}  /* namespace wire */


#endif /* WIRE_UTIL_TIMED_CACHE_HPP_ */
