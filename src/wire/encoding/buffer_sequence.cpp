/*
 * buffer_iterator.cpp
 *
 *  Created on: 2 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/encoding/detail/buffer_sequence.hpp>
#include <numeric>
#include <cassert>
#include <wire/errors/exceptions.hpp>
#include <wire/encoding/wire_io.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace wire {
namespace encoding {
namespace detail {

buffer_sequence::buffer_sequence(core::connector_ptr cnctr)
    : buffers_{ 1, buffer_type{}}, connector_{cnctr}
{
}

buffer_sequence::buffer_sequence(core::connector_ptr cnctr, size_type number)
    : buffers_{ number, buffer_type{}}, connector_{cnctr}
{
}

buffer_sequence::buffer_sequence(core::connector_ptr cnctr, buffer_type const& b)
    : buffers_{{b}}, connector_{cnctr}
{
}

buffer_sequence::buffer_sequence(core::connector_ptr cnctr, buffer_type&& b)
    : buffers_{{std::move(b)}}, connector_{cnctr}
{
}

buffer_sequence::buffer_sequence(buffer_sequence const& rhs)
    : buffers_{rhs.buffers_}, connector_{rhs.connector_}
{
    ::std::transform(
        rhs.out_encaps_stack_.begin(), rhs.out_encaps_stack_.end(),
        ::std::back_inserter(out_encaps_stack_),
        [&]( out_encaps_state const& r )
        {
            out_encaps_state res{r};
            res.sp_.seq_ = this;
            if (res.current_segment_) {
                res.current_segment_->sp_.seq_ = this;
            }
            return res;
        });
    // TODO Copy input encaps
}

buffer_sequence::buffer_sequence(buffer_sequence&& rhs)
    : buffers_{std::move(rhs.buffers_)}, connector_{rhs.connector_}
{
    // TODO Close all out encaps
    ::std::transform(
        rhs.out_encaps_stack_.begin(), rhs.out_encaps_stack_.end(),
        ::std::back_inserter(out_encaps_stack_),
        [&]( out_encaps_state const& r )
        {
            out_encaps_state res{r};
            res.sp_.seq_ = this;
            if (res.current_segment_) {
                res.current_segment_->sp_.seq_ = this;
            }
            return res;
        });
    // TODO Copy input encaps
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
    out_encaps_stack_.emplace_back(*this);
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
        throw errors::marshal_error("There is no current outgoing encapsulation");
    return --out_encaps_stack_.end();
}

buffer_sequence::in_encaps_iterator
buffer_sequence::begin_in_encaps(const_iterator beg)
{
    in_encaps_stack_.emplace_back(*this, beg);
    return --in_encaps_stack_.end();
}

void
buffer_sequence::end_in_encaps(in_encaps_iterator iter)
{
    if (in_encaps_stack_.end() == ++iter)
        in_encaps_stack_.pop_back();
}

buffer_sequence::in_encaps_iterator
buffer_sequence::current_in_encaps()
{
    if (in_encaps_stack_.empty())
        throw errors::marshal_error("There is no current incoming encapsulation");
    return --in_encaps_stack_.end();
}

buffer_sequence::out_encaps
buffer_sequence::begin_out_encapsulation()
{
    begin_out_encaps();
    return out_encaps{this};
}

buffer_sequence::out_encaps
buffer_sequence::current_out_encapsulation() const
{
    return out_encaps{const_cast<buffer_sequence*>(this)};
}

void
buffer_sequence::close_out_encapsulations()
{
    while (out_encaps_stack_.size() > 1)
        out_encaps_stack_.pop_back();
    out_encaps_stack_.front().write_object_queue();
}

buffer_sequence::in_encaps
buffer_sequence::begin_in_encapsulation(const_iterator beg)
{
    begin_in_encaps(beg);
    return in_encaps{this};
}

buffer_sequence::in_encaps
buffer_sequence::current_in_encapsulation() const
{
    return in_encaps{const_cast<buffer_sequence*>(this)};
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

buffer_sequence::out_encaps_state::out_encaps_state(buffer_sequence& out, bool is_default)
    : sp_{out}, is_default_{is_default}
{
}

buffer_sequence::out_encaps_state::out_encaps_state(out_encaps_state const& rhs)
    : sp_{rhs.sp_},
      encoding_version{rhs.encoding_version},
      types_{rhs.types_},
    current_segment_{ rhs.current_segment_ ? new segment{*rhs.current_segment_} : nullptr },
      is_default_{rhs.is_default_}
{
}

buffer_sequence::out_encaps_state::out_encaps_state(out_encaps_state&& rhs)
    : sp_{rhs.sp_},
      encoding_version{rhs.encoding_version},
      types_{::std::move(rhs.types_)},
      current_segment_{ ::std::move(rhs.current_segment_) },
      is_default_{rhs.is_default_}
{
    rhs.sp_.seq_ = nullptr;
}

buffer_sequence::out_encaps_state::~out_encaps_state()
{
    if (!is_default_ && sp_.seq_) {
        write_object_queue();
        buffer_type& buff = sp_.buffer();
        write(::std::back_inserter(buff), encoding_version, size());
    }
}

void
buffer_sequence::out_encaps_state::start_segment(segment_header::flags_type flags,
        ::std::string const& name)
{
    if (!sp_.seq_)
        throw errors::marshal_error("Output encapsulation is invalid");

    auto f = types_.find(name);
    size_type ti = 0;
    if (f == types_.end()) {
        types_.insert(::std::make_pair(name, types_.size() + 1));
        flags = static_cast<segment_header::flags_type>(
                (flags & ~segment_header::hash_type_id) | segment_header::string_type_id);
        ti = types_.size();
    } else {
        ti = f->second;
    }

    current_segment_.reset( new segment{ *sp_.seq_, flags, name, ti } );
    sp_.seq_->start_buffer();
}

void
buffer_sequence::out_encaps_state::start_segment(segment_header::flags_type flags,
        hash_value_type const& name_hash)
{
    if (!sp_.seq_)
        throw errors::marshal_error("Output encapsulation is invalid");

    auto f = types_.find(name_hash);
    size_type ti = 0;
    if (f == types_.end()) {
        types_.insert(::std::make_pair(name_hash, types_.size() + 1));
        flags = static_cast<segment_header::flags_type>(
                (flags & ~segment_header::string_type_id) | segment_header::hash_type_id);
        ti = types_.size();
    } else {
        ti = f->second;
    }

    current_segment_.reset( new segment{ *sp_.seq_, flags, name_hash, ti } );
    sp_.seq_->start_buffer();
}

void
buffer_sequence::out_encaps_state::end_segment()
{
    current_segment_.reset();
}

buffer_sequence::out_encaps_state::object_stream_id
buffer_sequence::out_encaps_state::enqueue_object(void const* obj, marshal_func func)
{
    auto f = object_ids_.find(obj);
    if (f == object_ids_.end()) {
        object_stream_id id = object_ids_.size() + 1;
        ::std::cerr << "Enqueue object with id " << id << "\n";
        object_write_queue_.push_back({id, func});
        object_ids_.emplace(obj, id);
        return id;
    }
    ::std::cerr << "Object already enqueued with id " << f->second << "\n";
    return f->second;
}

void
buffer_sequence::out_encaps_state::write_object_queue()
{
    queued_objects queue;
    queue.swap(object_write_queue_);
    write(::std::back_inserter(sp_.back_buffer()), queue.size());
    while (!queue.empty()) {
        ::std::cerr << "Outgoing object queue size: " << queue.size() << "\n";
        for (auto const& o: queue) {
            o.marshal(o.id);
        }
        queue.swap(object_write_queue_);
        object_write_queue_.clear();
        write(::std::back_inserter(sp_.back_buffer()), queue.size());
    }
}

buffer_sequence::out_encaps_state::segment::~segment()
{
    if (sp_.seq_) {
        auto out = ::std::back_inserter(sp_.buffer());
        auto sz = sp_.size();
        write(out, flags);
        if (flags & string_type_id) {
            ::std::cerr << "Write string type id " << type_id << "\n";
            write(out, ::boost::get<::std::string>(type_id));
        } else if (flags & hash_type_id) {
            ::std::cerr << "Write hash type id " << type_id << "\n";
            write(out, ::boost::get<hash_value_type>(type_id));
        } else {
            ::std::cerr << "Write type id index " << type_idx_ << "\n";
            write(out, type_idx_);
        }
        write(out, sz);
        sp_.seq_->pop_empty_buffer();
    }
}

//----------------------------------------------------------------------------
buffer_sequence::in_encaps_state::in_encaps_state(buffer_sequence& seq, const_iterator beg)
    : seq_{&seq}, begin_{beg}
{
    end_ = seq_->end();
    read(begin_, end_, encoding_version, size_);
    end_ = begin_ + size_;
}

buffer_sequence::in_encaps_state::in_encaps_state(buffer_sequence& seq)
    : seq_(&seq), begin_{seq_->begin()}, is_default_{true}
{
    size_ = seq_->size();
    end_ = seq_->end();
}

//----------------------------------------------------------------------------
::std::ostream&
debug_output(::std::ostream& os, buffer_sequence const& bs)
{
    int line_size{0};
    int buff_no{0};
    ::std::ostringstream char_rep;
    for (auto const& b : bs.buffers()) {
        os << "\nBuffer " << buff_no << "\n";
        for (int i = 0; i < line_size; i++) {
            if (i == 8) {
                os << "  ";
                char_rep.put(' ');
            }
            os << "   ";
            char_rep.put(' ');
        }
        for (auto c : b) {
            if (line_size == 8) {
                os << "  ";
                char_rep << " ";
            }
            os << ::std::setw(2) << ::std::setfill('0') << ::std::hex << (int)c << " ";
            if (isprint(c)) {
                char_rep.put(c);
            } else {
                char_rep.put('.');
            }
            line_size++;
            if (line_size == 16) {
                os << "| " << char_rep.str() << "\n";
                char_rep.str("");
                line_size = 0;
            }
        }
        if (line_size > 0) {
            for (int i = line_size; i < 16; ++i) {
                if (i == 8)
                    os << "  ";
                os << "   ";
            }
            os << "| " << char_rep.str() << "\n";
            char_rep.str("");
        }
        ++buff_no;
    }
    return os;
}

}  // namespace detail
}  // namespace encoding
}  // namespace wire

