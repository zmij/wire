/*
 * message.hpp
 *
 *  Created on: Jan 21, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_MESSAGE_HPP_
#define WIRE_ENCODING_MESSAGE_HPP_

#include <wire/encoding/types.hpp>
#include <wire/encoding/wire_io.hpp>
#include <wire/core/identity.hpp>
#include <wire/core/context.hpp>
#include <wire/version.hpp>

namespace wire {
namespace encoding {

struct version {
    uint32_t    major;
    uint32_t    minor;

    bool
    operator == (version const& rhs) const
    {
        return major == rhs.major && minor == rhs.minor;
    }

    bool
    operator != (version const& rhs) const
    {
        return !(*this == rhs);
    }

    bool
    operator < (version const& rhs) const
    {
        return major < rhs.major || (major == rhs.major && minor < rhs.minor);
    }
    bool
    operator <= (version const& rhs) const
    {
        return major <= rhs.major || (major == rhs.major && minor <= rhs.minor);
    }

    bool
    operator > (version const& rhs) const
    {
        return rhs < *this;
    }
    bool
    operator >= (version const& rhs) const
    {
        return rhs <= *this;
    }


    void
    swap(version& rhs) noexcept
    {
        using ::std::swap;
        swap(major, rhs.major);
        swap(minor, rhs.minor);
    }
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, version const& v)
{
    write(o, v.major, v.minor);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, version& v)
{
    version tmp;
    read(begin, end, tmp.major, tmp.minor);
    v.swap(tmp);
}

template < typename InputIterator >
bool
try_read(InputIterator& start, InputIterator end, version& v)
{
    using reader = detail::reader<decltype(v.major)>;
    version tmp;
    auto begin = start;
    if (!reader::try_input(begin, end, tmp.major))
        return false;
    if (!reader::try_input(begin, end, tmp.minor))
        return false;
    v.swap(tmp);
    start = begin;
    return true;
}


struct message {
    using size_type = uint64_t;
    enum message_flags {
        // Types
        request             = 0,
        reply               = 2,
        validate            = 3,
        close               = 4,
        // Flags
        protocol            = 8,        /**< Message header contains protocol version */
        encoding            = 0x10,     /**< Message header contains encoding version */

        // Combinations
        validate_flags      = validate | protocol | encoding,
        // Masks
        type_bits           = reply | validate | close,
        flag_bits           = ~type_bits,
    };
    static constexpr uint32_t MAGIC_NUMBER =
            ('w') | ('i' << 8) | ('r' << 16) | ('e' << 24);
    static constexpr uint32_t magic_number_size = sizeof(uint32_t);
    /** 4 bytes for magic number, 1 for flags, 1 for size */
    static constexpr uint32_t min_header_size = 6;
    /** 4 bytes for magic number, 1 for flags,  2 for protocol version
     * 2 for encoding version, 9 - for maximum possible size
     */
    static constexpr uint32_t max_header_size = 18;

    version         protocol_version    = version{ ::wire::PROTOCOL_MAJOR, ::wire::PROTOCOL_MINOR };
    version         encoding_version    = version{ ::wire::ENCODING_MAJOR, ::wire::ENCODING_MINOR };

    message_flags   flags               = request;
    size_type       size                = 0;

    message() = default;
    message(message_flags flags, size_type size)
        : flags(flags), size(size)
    {}

    bool
    operator == (message const& rhs) const
    {
        return protocol_version == rhs.protocol_version
                && encoding_version == rhs.encoding_version
                && flags == rhs.flags
                && size == rhs.size;
    }

    void
    swap(message& rhs) noexcept
    {
        using ::std::swap;
        swap(protocol_version, rhs.protocol_version);
        swap(encoding_version, rhs.encoding_version);
        swap(flags, rhs.flags);
        swap(size, rhs.size);
    }

    message_flags
    type() const
    {
        return static_cast<message_flags>(flags & type_bits);
    }
};

::std::ostream&
operator << (::std::ostream& out, message::message_flags val);

}  // namespace encoding
}  // namespace wire

namespace std {
inline void
swap(wire::encoding::message& lhs, wire::encoding::message& rhs)
{
    lhs.swap(rhs);
}

}

namespace wire {
namespace encoding {

template < typename OutputIterator >
void
wire_write(OutputIterator o, message const& v)
{
    write(o, int32_fixed_t(message::MAGIC_NUMBER));
    write(o, v.flags);
    // write this info depending on flags
    if (v.flags & message::protocol)
        write(o, PROTOCOL_MAJOR, PROTOCOL_MINOR);
    if (v.flags & message::encoding)
        write(o, v.encoding_version);

    write(o, v.size);
}

template < typename InputIterator >
void
read(InputIterator& begin, InputIterator end, message& v)
{
    int32_fixed_t magic;
    read(begin, end, magic);
    if (magic != message::MAGIC_NUMBER) {
        // Unrecoverable
        throw errors::invalid_magic_number("Invalid magic number in message header");
    }
    message tmp;
    read(begin, end, tmp.flags);
    // read this info depending on message flags
    if (tmp.flags & message::protocol)
        read(begin, end, tmp.protocol_version);
    if (tmp.flags & message::encoding)
        read(begin, end, tmp.encoding_version);

    read(begin, end, tmp.size);
    v.swap(tmp);
}

/**
 * Try to read message header from buffer.
 * @pre Minimum size of the buffer to succeed is message::min_header_size
 * @param begin
 * @param end
 * @param v
 * @return
 */
template < typename InputIterator >
bool
try_read(InputIterator& start, InputIterator end, message& v)
{
    using flags_reader = detail::reader<message::message_flags>;
    using size_reader = detail::reader<message::size_type>;

    if (end - start < message::magic_number_size)
        return false; // We cannot read even the magic number
    auto begin = start;
    int32_fixed_t magic;
    read(begin, end, magic);
    if (magic != message::MAGIC_NUMBER) {
        // Unrecoverable
        throw errors::invalid_magic_number("Invalid magic number in message header");
    }
    message tmp;
    if (!flags_reader::try_input(begin, end, tmp.flags))
        return false;
    if (tmp.flags & message::protocol) {
        if (!try_read(begin, end, tmp.protocol_version))
            return false;
    }
    if (tmp.flags & message::encoding) {
        if (!try_read(begin, end, tmp.encoding_version))
            return false;
    }
    if (!size_reader::try_input(begin, end, tmp.size))
        return false;

    v.swap(tmp);
    start = begin;
    return true;
}

struct invocation_target {
    core::identity  identity;
    ::std::string   facet;

    void
    swap(invocation_target& rhs)
    {
        using ::std::swap;
        swap(identity, rhs.identity);
        swap(facet, rhs.facet);
    }

    bool
    operator == (invocation_target const& rhs) const
    {
        return identity == rhs.identity && facet == rhs.facet;
    }
    bool
    operator != (invocation_target const& rhs) const
    {
        return !(*this == rhs);
    }
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, invocation_target const& v)
{
    write(o,
        v.identity, v.facet
    );
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, invocation_target& v)
{
    invocation_target tmp;
    read(begin, end, tmp.identity, tmp.facet);
    v.swap(tmp);
}

using multiple_targets = ::std::set<invocation_target>;

struct operation_specs {
    enum operation_type {
        name_hash,
        name_string
    };
    using hash_type    = uint32_fixed_t;
    using operation_id = boost::variant< hash_type, ::std::string >;
    invocation_target   target;
    operation_id        operation;

    void
    swap(operation_specs& rhs)
    {
        using ::std::swap;
        swap(target, rhs.target);
        swap(operation, rhs.operation);
    }

    bool
    operator == (operation_specs const& rhs) const
    {
        return target == rhs.target && operation == rhs.operation;
    }

    bool
    operator != (operation_specs const& rhs) const
    {
        return !(*this == rhs);
    }

    operation_type
    type() const
    {
        return static_cast<operation_type>(operation.which());
    }

    ::std::string const&
    name() const
    {
        return ::boost::get<::std::string>(operation);
    }
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, operation_specs const& v)
{
    write(o,
        v.target, v.operation
    );
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, operation_specs& v)
{
    operation_specs tmp;
    read(begin, end, tmp.target, tmp.operation);
    v.swap(tmp);
}


struct request {
    using request_number    = ::std::uint64_t;
    enum request_mode {
        normal          = 0x01,
        one_way         = 0x02,
        multi_target    = 0x04,
        no_context      = 0x10,
        no_body         = 0x20,
    };
    request_number      number;
    operation_specs     operation;
    request_mode        mode;

    void
    swap(request& rhs)
    {
        using ::std::swap;
        swap(number, rhs.number);
        swap(operation, rhs.operation);
        swap(mode, rhs.mode);
    }

    bool
    operator == (request const& rhs) const
    {
        return number == rhs.number && operation == rhs.operation && mode == rhs.mode;
    }

    bool
    operator != (request const& rhs) const
    {
        return !(*this == rhs);
    }
};

inline request::request_mode
operator | (request::request_mode lhs, request::request_mode rhs)
{
    using integral_type = ::std::underlying_type<request::request_mode>::type;
    return static_cast<request::request_mode>(
            static_cast<integral_type>(lhs) | static_cast<integral_type>(rhs) );
}

inline request::request_mode&
operator |= (request::request_mode& lhs, request::request_mode rhs)
{
    return lhs = lhs | rhs;
}

template < typename OutputIterator >
void
wire_write(OutputIterator o, request const& v)
{
    write(o, v.number, v.operation, v.mode);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, request& v)
{
    request tmp;
    read(begin, end, tmp.number, tmp.operation, tmp.mode);
    v.swap(tmp);
}

struct reply {
    using request_number    = request::request_number;
    enum reply_status {
        success,
        success_no_body,
        user_exception,
        no_object,
        no_facet,
        no_operation,
        unknown_wire_exception,
        unknown_user_exception,
        unknown_exception
    };
    request_number  number;
    reply_status    status;

    void
    swap(reply& rhs)
    {
        using ::std::swap;
        swap(number, rhs.number);
        swap(status, rhs.status);
    }


    bool
    operator == (reply const& rhs) const
    {
        return number == rhs.number && status == rhs.status;
    }

    bool
    operator != (reply const& rhs) const
    {
        return !(*this == rhs);
    }

};

template < typename OutputIterator >
void
wire_write(OutputIterator o, reply const& v)
{
    write(o, v.number, v.status);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, reply& v)
{
    reply tmp;
    read(begin, end, tmp.number, tmp.status);
    v.swap(tmp);
}

}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_MESSAGE_HPP_ */
