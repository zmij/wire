/*
 * varint.hpp
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_VARINT_IO_HPP_
#define WIRE_DETAIL_VARINT_IO_HPP_

/**
 * @page varints Varint encoding
 *
 * Varints are a method of serializing integers using one or more bytes.
 * Smaller numbers take a smaller number of bytes.
 *
 * Each byte in a varint, except the last byte, has the most significant
 * bit (msb) set – this indicates that there are further bytes to come.
 * The lower 7 bits of each byte are used to store the two's complement
 * representation of the number in groups of 7 bits, least significant
 * group first.
 *
 * @section unisgned_varint Unsigned integers
 *
 * Unsigned integers are encoded using varint encoding.
 *
 * @section signed_varint Signed integers - ZigZag encoding
 * ZigZag encoding maps signed integers to unsigned integers so that numbers
 * with a small absolute value (for instance, -1) have a small varint encoded
 * value too. It does this in a way that "zig-zags" back and forth through
 * the positive and negative integers, so that -1 is encoded as 1, 1 is encoded
 * as 2, -2 is encoded as 3, and so on.
 *
 * In other words, each value n is encoded using
 * @code
 * (n << 1) ^ (n >> 31)
 * @endcode
 * for int32s, or
 * @code
 * (n << 1) ^ (n >> 63)
 * @endcode
 * for the 64-bit version.
 *
 * @section enum_varint Enumerations
 *
 * Enumeration values are converted to unsigned integral types that can hold
 * the enumeration value and are encoded using varint encoding.
 */

#include <wire/encoding/detail/helpers.hpp>
#include <wire/errors/exceptions.hpp>
#include <wire/util/demangle.hpp>
#include <boost/endian/arithmetic.hpp>

namespace wire {
namespace encoding {
namespace detail {

/**
 * Metafunction implementation to calculate mask for Nth byte of integral type T
 * @tparam T Integral type
 * @tparam N zero-based byte number
 */
template < typename T, size_t N >
struct varint_mask_value {
    using type = T;    /**< Typedef for the integral type. */
    /** Mask value */
    static constexpr type value =
            (static_cast< type >(0xff) << (N * 8)) |
            varint_mask_value< T, N - 1 >::value;
};

/**
 * Terminal metafunction implementation for varint mask for integral type T
 * @tparam T Integral type
 */
template < typename T >
struct varint_mask_value< T, 0 > {
    using type = T;    /**< Typedef for the integral type. */
    /** Mask value */
    static constexpr type value = static_cast<type>(1 << 7);
};

/**
 * Metafunction to calculate bit mask for reading varint encoding of integral type T.
 * The least significant byte has most significant bit on.
 * E.g. for two bytes: 1111 1111 1000 0000
 * @tparam T Integral type
 */
template < typename T >
struct varint_mask : varint_mask_value< T, sizeof(T) - 1 > {};

/**
 * Base declaration on metafunction to calculate unsigned integral type
 * enough to store an enumeration.
 */
template < std::size_t byte_count >
struct enum_integral_type;

/**
 * Specialization for short enums
 */
template <>
struct enum_integral_type<2> {
    /** Result of metafunction */
    using type = uint16_t;
};

/**
 * Specialization for int32 enums
 */
template <>
struct enum_integral_type<4> {
    /** Result of metafunction */
    using type = uint32_t;
};

/**
 * Specialization for int64 enums
 */
template <>
struct enum_integral_type<8> {
    /** Result of metafunction */
    using type = uint64_t;
};

template < typename T, bool is_signed >
struct varint_writer;

/**
 * Implementation of a varint writer for signed integer types,
 * using zig-zag encoding.
 * @tparam T Signed integral value
 */
template < typename T >
struct varint_writer < T, true > {
    /** Decayed type for type being written */
    using base_type = typename arg_type_helper<T>::base_type        ;
    /** Type deduced for argument passing, suitable for writing */
    using in_type = typename arg_type_helper<T>::in_type        ;
    /** Unsigned type of same size for actual buffer writing */
    using unsigned_type = typename std::make_unsigned<base_type>::type;
    /** Writer type for unsigned_type */
    using unsigned_writer = varint_writer< unsigned_type, false >        ;

    enum {
        shift_bits = sizeof(in_type) * 8 - 1 /**< Number of bits shifted for ZigZag encoging */
    };

    /**
     * Write signed integral value using ZigZag and Varint encodings.
     * @tparam OutputIterator Output iterator type
     * @param o output iterator
     * @param v signed integral value
     */
    template < typename OutputIterator >
    static void
    output( OutputIterator o, in_type v)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        using value_type = typename output_iterator_check::value_type    ;

        unsigned_writer::output(o,
            static_cast< unsigned_type >( (v << 1) ^ (v >> shift_bits) ));
    }
};

/**
 * Varint encoding write algorithm selector depending on whether the type is
 * an enumeration.
 */
template < typename T, bool is_enum >
struct varint_enum_writer;

/**
 * Varint encoding writer for enumerations.
 */
template < typename T >
struct varint_enum_writer< T, true > {
    /** Enumeration decayed type */
    using base_type = typename arg_type_helper<T>::base_type        ;
    /** Type deduced for argument passing, suitable for writing */
    using in_type = typename arg_type_helper<T>::in_type        ;
    /** Unsigned integral type enough for storing the enumeration type */
    using integral_type = typename enum_integral_type< sizeof(T) >::type;
    /** Varint writer type for the integral type */
    using writer_type = varint_writer< integral_type, false >        ;

    /**
     * Convert enumeration value to an unsigned integral value and write it
     * using varint encoding.
     * @tparam OutputIterator Output iterator type
     * @param o output iterator
     * @param v enumeration value
     */
    template < typename OutputIterator >
    static void
    output( OutputIterator o, in_type v)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        using value_type = typename output_iterator_check::value_type    ;
        writer_type::output(o, static_cast<integral_type>(v));
    }
};

/**
 * Implementation of varint writer for unsigned types
 */
template < typename T >
struct varint_enum_writer < T, false > {
    /** Decayed type for type being written */
    using base_type = typename arg_type_helper<T>::base_type;
    /** Type deduced for argument passing, suitable for writing */
    using in_type = typename arg_type_helper<T>::in_type;
    /** Mask for checking if there is more data in high bits */
    using mask_type = varint_mask< T >                    ;
    /** Mask for least significant byte */
    using lsb_mask_type = varint_mask_value< T, 0 >            ;

    //@{
    /** @name Bit masks */
    static constexpr base_type eighth_bit = lsb_mask_type::value;    // 0b10000000
    static constexpr base_type seven_bits = ~lsb_mask_type::value;    // 0b01111111
    //@}

    /**
     * Write unsigned integral value using varint encoding
     * @tparam OutputIterator Output iterator type
     * @param o output iterator
     * @param v unsigned integral value
     */
    template < typename OutputIterator >
    static void
    output( OutputIterator o, in_type v)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        using value_type = typename output_iterator_check::value_type    ;

        v = boost::endian::native_to_little(v);
        value_type current = v & seven_bits;
        while (v & mask_type::value) {
            current |= eighth_bit;
            *o++ = current;
            v = v >> 7;
            current = v & seven_bits;
        }
        *o++ = current;
    }
};

/**
 * Varint encoding algorithm selector for unsigned integral values.
 */
template < typename T >
struct varint_writer< T, false >
    : varint_enum_writer< T, std::is_enum<T>::value > {};

template < typename T, bool is_signed >
struct varint_reader;

/**
 * Traits for reading a signed type using ZigZag encoding
 */
template < typename T >
struct zig_zag_traits {
    /** Decayed type */
    using type = typename std::decay<T>::type        ;
    /** Corresponding unsigned type */
    using unsigned_type = typename std::make_unsigned<type>::type;
    enum {
        shift_bits = sizeof(type) * 8 - 1
    };
};

/**
 * Specialization of traits for reading a signed type using ZigZag encoding for int16_t
 */
template <>
struct zig_zag_traits<int16_t> {
    /** Decayed type */
    using type = int16_t                                ;
    /** Corresponding unsigned type */
    using unsigned_type = typename std::make_unsigned<type>::type;
    enum {
        shift_bits = 31
    };
};

/**
 * Varing encoding reader implementation for signed types
 */
template < typename T >
struct varint_reader< T, true > {
    /** Decayed type for type being read */
    using type = typename std::decay<T>::type        ;
    /** Corresponding unsigned type */
    using unsigned_type = typename std::make_unsigned<type>::type;
    /** Reader type for unsigned_type */
    using unsigned_reader = varint_reader< unsigned_type, false >;
    /** ZigZag encoding traits */
    using traits = zig_zag_traits<type>                ;

    enum {
        shift_bits = traits::shift_bits
    };

    /**
     * Read unsigned varint value from buffer and convert it to signed one
     * using ZigZag encoding
     * @param begin Start of read sequence
     * @param end End of read sequence
     * @param v Value to read
     * @return Pair of iterator and boolean flag if the operation was successful
     */
    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, type& v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        unsigned_type tmp;
        try {
            unsigned_reader::input(begin, end, tmp);
            v = static_cast<type>(tmp >> 1) ^
                    (static_cast<type>(tmp) << shift_bits >> shift_bits);
        } catch (errors::unmarshal_error const&) {
            throw errors::unmarshal_error("Failed to read signed value of "
                    + util::demangle<T>());
        }
    }
};

template < typename T, bool is_enum >
struct varint_enum_reader;

/**
 * Varint encoding reader for enumerations
 */
template < typename T >
struct varint_enum_reader< T, true > {
    /** Enumeration decayed type */
    using base_type = typename arg_type_helper<T>::base_type        ;
    /** Type deduced for argument passing, suitable for reading (a non-const reference) */
    using out_type = typename arg_type_helper<T>::out_type        ;
    /** Unsigned integral type enough for storing the enumeration type */
    using integral_type = typename enum_integral_type< sizeof(T) >::type;
    /** Varint reader type for the integral type */
    using reader_type = varint_reader< integral_type, false >        ;

    /**
     * Read an unsigned integral value from input sequence and convert it to
     * enumeration type.
     * @param begin Start of read sequence
     * @param end End of read sequence
     * @param v Value to read
     * @return Pair of iterator and boolean flag if the operation was successful
     */
    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        integral_type iv;
        try {
            reader_type::input(begin, end, iv);
            v = static_cast<base_type>(iv);
        } catch (errors::unmarshal_error const&) {
            throw errors::unmarshal_error("Failed to read enumeration "
                    + util::demangle<T>() + " value");
        }
    }
    template < typename InputIterator >
    static bool
    try_input(InputIterator& begin, InputIterator end, out_type v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        integral_type iv;
        if (reader_type::try_input(begin, end, iv)) {
            v = static_cast<base_type>(iv);
            return true;
        }
        return false;
    }
};

/**
 * Varint encoding reader for unsigned values
 */
template < typename T >
struct varint_enum_reader< T, false > {
    /** Decayed type */
    using base_type = typename arg_type_helper<T>::base_type;
    /** Type deduced for argument passing, suitable for reading (a non-const reference) */
    using out_type = typename arg_type_helper<T>::out_type;
    /** Mask for least significant byte */
    using lsb_mask_type = varint_mask_value< T, 0 >            ;

    //@{
    /** @name Bit masks */
    static constexpr base_type eighth_bit = lsb_mask_type::value;    // 0b10000000
    static constexpr base_type seven_bits = ~lsb_mask_type::value;    // 0b01111111
    static constexpr uint32_t  bit_count  = sizeof(T) * 8;
    //@}

    /**
     * Read unsigned integral value using varint encoding
     * @param begin Start of read sequence
     * @param end End of read sequence
     * @param v Value to read
     * @return Pair of iterator and boolean flag if the operation was successful
     */
    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        base_type tmp = 0;
        bool more = true;
        for (uint32_t n = 0; more && begin != end && (7 * n) <= bit_count; ++n) {
            base_type curr_byte = (byte)*begin++;
            tmp |= (curr_byte & seven_bits) << (7 * n);
            more = curr_byte & eighth_bit;
        }
        if (more) {
            throw errors::unmarshal_error("Failed to read unsigned integral value of "
                    + util::demangle<T>());
        }
        v = boost::endian::little_to_native(tmp);
    }

    template < typename InputIterator >
    static bool
    try_input(InputIterator& start, InputIterator end, out_type v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;
        auto begin = start;
        base_type tmp = 0;
        bool more = true;
        for (uint32_t n = 0; more && begin != end && (7 * n) <= bit_count; ++n) {
            base_type curr_byte = (byte)*begin++;
            tmp |= (curr_byte & seven_bits) << (7 * n);
            more = curr_byte & eighth_bit;
        }
        if (more) {
            if (begin == end) {
                return false;
            } else {
                throw errors::unmarshal_error("Failed to read unsigned integral value of "
                        + util::demangle<T>());
            }
        } else {
            v = boost::endian::little_to_native(tmp);
            start = begin;
            return true;
        }
    }
};

template < typename T >
struct varint_reader< T, false >
    : varint_enum_reader< T, std::is_enum<T>::value > {};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_DETAIL_VARINT_IO_HPP_ */
