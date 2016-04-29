/*
 * buffer_iterator.cpp
 *
 *  Created on: 2 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/encoding/detail/buffer_iterator.hpp>
#include <numeric>
#include <cassert>
#include <wire/errors/exceptions.hpp>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace encoding {
namespace detail {

buffer_sequence::buffer_sequence()
    : buffers_{ 1, buffer_type{}}
{
}

buffer_sequence::buffer_sequence(size_type number)
    : buffers_{ number, buffer_type{}}
{
}

buffer_sequence::buffer_sequence(buffer_type const& b)
    : buffers_{{b}}
{
}

buffer_sequence::buffer_sequence(buffer_type&& b)
    : buffers_{{std::move(b)}}
{
}

buffer_sequence::buffer_sequence(buffer_sequence const& rhs)
    : buffers_{rhs.buffers_}
{
}

buffer_sequence::buffer_sequence(buffer_sequence&& rhs)
    : buffers_{std::move(rhs.buffers_)}
{
}

void
buffer_sequence::swap(buffer_sequence& rhs)
{
    using std::swap;
    swap(buffers_, rhs.buffers_);
}

buffer_sequence&
buffer_sequence::operator =(buffer_sequence const& rhs)
{
    buffer_sequence{rhs}.swap(*this);
    return *this;
}

buffer_sequence&
buffer_sequence::operator =(buffer_sequence&& rhs)
{
    buffer_sequence{std::move(rhs)}.swap(*this);
    return *this;
}

buffer_sequence::size_type
buffer_sequence::size() const
{
    return std::accumulate(
        buffers_.cbegin(), buffers_.cend(), size_type(0),
        [](size_type sz, buffer_type const& buff)
        {
            return sz + buff.size();
        });
}

bool
buffer_sequence::empty() const
{
    for (auto const& b : buffers_) {
        if (!b.empty())
            return false;
    }
    return true;
}

buffer_sequence::iterator
buffer_sequence::begin()
{
    if (empty())
        return end();
    auto b = buffers_.begin();
    for (; b != buffers_.end() && b->empty(); ++b);
    if (b == buffers_.end())
        return end();
    return iterator{ this, b, b->begin() };
}

buffer_sequence::const_iterator
buffer_sequence::cbegin() const
{
    if (empty())
        return cend();
    auto b = buffers_.begin();
    for (; b != buffers_.end() && b->empty(); ++b);
    if (b == buffers_.end())
        return end();
    return const_iterator{ this, b, b->begin() };
}

buffer_sequence::iterator
buffer_sequence::end()
{
    return iterator{ this, after_end };
}

buffer_sequence::const_iterator
buffer_sequence::cend() const
{
    return const_iterator{ this, after_end };
}

buffer_sequence::reverse_iterator
buffer_sequence::rbegin()
{
    return reverse_iterator{ end() };
}

buffer_sequence::const_reverse_iterator
buffer_sequence::crbegin() const
{
    return const_reverse_iterator{ cend() };
}

buffer_sequence::reverse_iterator
buffer_sequence::rend()
{
    return reverse_iterator{ begin() };
}

buffer_sequence::const_reverse_iterator
buffer_sequence::crend() const
{
    return const_reverse_iterator{ cbegin() };
}

buffer_sequence::iterator
buffer_sequence::last()
{
    if (empty())
        return end();
    auto b = buffers_.end() - 1;
    for (; b != buffers_.begin() && b->empty(); --b);
    if (b == buffers_.begin() && b->empty())
        return end();
    return iterator{ this, b, b->end() - 1 };
}

buffer_sequence::const_iterator
buffer_sequence::clast() const
{
    if (empty())
        return end();
    auto b = buffers_.end() - 1;
    for (; b != buffers_.begin() && b->empty(); --b);
    if (b == buffers_.begin() && b->empty())
        return end();
    return const_iterator{ this, b, b->end() - 1 };
}

buffer_sequence::reference
buffer_sequence::front()
{
    return front_buffer().front();
}

buffer_sequence::const_reference
buffer_sequence::front() const
{
    return front_buffer().front();
}

buffer_sequence::reference
buffer_sequence::back()
{
    return back_buffer().back();
}

buffer_sequence::const_reference
buffer_sequence::back() const
{
    return back_buffer().back();
}

void
buffer_sequence::push_back(value_type v)
{
    back_buffer().push_back(v);
}

void
buffer_sequence::pop_back()
{
    back_buffer().pop_back();
}

template < typename P, typename This >
buffer_iterator< typename std::remove_const< This >::type, P >
buffer_sequence::iter_at_index(This* _this, size_type n)
{
    typedef buffer_iterator< typename std::remove_const< This >::type, P > iterator_type;
    for (auto b = _this->buffers_.begin(); b != _this->buffers_.end(); ++b) {
        if (n < b->size()) {
            return iterator_type{ _this, b, b->begin() + n };
        }
        n -= b->size();
    }
    return iterator_type{ _this, after_end };
}

template < typename This, typename P >
void
buffer_sequence::advance(This* _this,
        buffer_iterator< typename std::remove_const< This >::type, P>& iter, difference_type n)
{
    assert(_this == iter.container_ && "Iterator belongs to container");
    typedef buffer_iterator< typename std::remove_const< This >::type, P> iter_type;
    if (n != 0) {
        if (iter.position_ == normal) {
            difference_type bpos = iter.current_ - iter.buffer_->begin();
            if (n > 0) {
                if (bpos + n < iter.buffer_->size()) {
                    // Same buffer
                    iter.current_ += n;
                } else {
                    n -= iter.buffer_->size() - bpos;
                    ++iter.buffer_;
                    for(; iter.buffer_ != _this->buffers_.end()
                        && iter.buffer_->empty(); ++iter.buffer_);
                    if (iter.buffer_ == _this->buffers_.end()) {
                        iter_type{ _this, after_end }.swap(iter);
                    } else {
                        iter.current_ = iter.buffer_->begin();
                        advance(_this, iter, n);
                    }
                }
            } else { // n < 0
                if (bpos + n >= 0) {
                    // Same buffer
                    iter.current_ += n;
                } else {
                    n += bpos;
                    if (iter.buffer_ == _this->buffers_.begin()) {
                        iter_type{ _this, before_begin }.swap(iter);
                    } else {
                        --iter.buffer_;
                        for(;iter.buffer_ != _this->buffers_.begin()
                            && iter.buffer_->empty(); --iter.buffer_);
                        if (iter.buffer_->empty()) {
                            iter_type{ _this, before_begin }.swap(iter);
                        } else {
                            iter.current_ = iter.buffer_->end() - 1;
                            advance(_this, iter, n);
                        }
                    }
                }
            }
        } else if (iter.position_ == after_end && n < 0) {
            difference_type sz = _this->size();
            if (n + sz < 0) {
                iter_type{ _this, before_begin }.swap(iter);
            } else {
                // Move n-1 from end
                iter_at_index<P>(_this, sz + n).swap(iter);
            }
        } else if (iter.position_ == before_begin && n > 0) {
            difference_type sz = _this->size();
            if (n > sz) {
                iter_type{ _this, after_end }.swap(iter);
            } else {
                iter_at_index<P>(_this, n - 1).swap(iter);
            }
        }
    }
}

void
buffer_sequence::advance(iterator& iter, difference_type diff) const
{
    assert(this == iter.container_ && "Iterator belongs to container");
    advance(iter.container_, iter, diff);
}

void
buffer_sequence::advance(const_iterator& iter, difference_type diff) const
{
    advance(this, iter, diff);
}

template < typename This, typename P >
buffer_sequence::difference_type
buffer_sequence::index_of(This* _this, buffer_iterator< typename std::remove_const< This >::type, P> const& iter)
{
    typedef buffer_sequence_type::const_iterator const_buffer_iterator;
    if (iter.position_ == before_begin)
        return -1;
    if (iter.position_ == after_end)
        return _this->size();
    difference_type prev_buffers = std::accumulate(
        _this->buffers_.cbegin(), const_buffer_iterator{ iter.buffer_ },
        difference_type(0),
        [](difference_type sz, buffer_type const& buff)
        {
            return sz + buff.size();
        });
    return prev_buffers + (iter.current_ - iter.buffer_->begin());
}

template < typename This, typename P >
buffer_sequence::difference_type
buffer_sequence::difference(This* _this,
        buffer_iterator< typename std::remove_const< This >::type, P> const& a,
        buffer_iterator< typename std::remove_const< This >::type, P> const& b)
{
    assert(_this == a.container_ && "Iterator belongs to container");
    assert(_this == b.container_ && "Iterator belongs to container");
    if (a.position_ == normal && b.position_ == normal) {
        if (a.buffer_ == b.buffer_ && a.buffer_ != _this->buffers_.end())
            return a.current_ - b.current_;

    }
    return index_of(_this, a) - index_of(_this, b);
}

buffer_sequence::difference_type
buffer_sequence::difference(iterator const& a, iterator const& b) const
{
    assert(this == a.container_ && "Iterator belongs to container");
    return difference(a.container_, a, b);
}

buffer_sequence::difference_type
buffer_sequence::difference(const_iterator const& a, const_iterator const& b) const
{
    //assert(this == a.container_ && "Iterator belongs to container");
    return difference(this, a, b);
}

buffer_sequence::out_encaps_iterator
buffer_sequence::begin_out_encaps()
{
    out_encaps_stack_.emplace_back(this);
    start_buffer();
    return --out_encaps_stack_.end();
}

void
buffer_sequence::end_out_encaps(out_encaps_iterator iter)
{
    if (out_encaps_stack_.end() == ++iter) {
        out_encaps_stack_.pop_back();
    }
    start_buffer();
}

buffer_sequence::out_encaps_iterator
buffer_sequence::current_out_encaps()
{
    if (out_encaps_stack_.empty())
        throw errors::marshal_error("There is no current encapsulation");
    return --out_encaps_stack_.end();
}

buffer_sequence::out_encaps
buffer_sequence::begin_out_encapsulation()
{
    begin_out_encaps();
    return out_encaps{this};
}

buffer_sequence::out_encaps
buffer_sequence::current_out_encapsulation()
{
    return out_encaps{this};
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
buffer_sequence::out_encaps_state::out_encaps_state(buffer_sequence* out)
    : out_{out},
      size_before_{out_->size()},
      buffer_before_{ out_->buffers_size()  - 1 }
{
}

buffer_sequence::out_encaps_state::out_encaps_state(out_encaps_state&& rhs)
    : out_{rhs.out_},
      size_before_{rhs.size_before_},
      buffer_before_{ rhs.buffer_before_ }
{
    rhs.out_ = nullptr;
}

buffer_sequence::out_encaps_state::~out_encaps_state()
{
    if (out_) {
        buffer_type& buff = out_->buffer_at(buffer_before_);
        write(::std::back_inserter(buff), encoding_version, size());
    }
}

void
buffer_sequence::out_encaps_state::write_type_name(::std::string const& name)
{
    auto f = types_.find(name);
    auto& b = out_->back_buffer();
    auto o = ::std::back_inserter(b);
    if (f == types_.end()) {
        types_.insert(::std::make_pair(name, types_.size() + 1));
        write(o, (::std::size_t)0, name);
    } else {
        write(o, f->second);
    }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
buffer_sequence::out_encaps::out_encaps(buffer_sequence* seq)
    : seq_(seq), iter_(seq->current_out_encaps())
{
}

bool
buffer_sequence::out_encaps::empty() const
{
    return iter_->empty();
}

buffer_sequence::size_type
buffer_sequence::out_encaps::size() const
{
    return iter_->size();
}

void
buffer_sequence::out_encaps::end_encaps()
{
    seq_->end_out_encaps(iter_);
}

void
buffer_sequence::out_encaps::write_type_name(::std::string const& name)
{
    iter_->write_type_name(name);
}

}  // namespace detail
}  // namespace encoding
}  // namespace wire

