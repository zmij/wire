/*
 * murmur_hash.cpp
 *
 *  Created on: May 4, 2016
 *      Author: zmij
 */

#include <wire/util/murmur_hash.hpp>

namespace wire {
namespace hash {

constexpr hash_result<16>::type hash_result<16>::seed;
constexpr hash_result<32>::type hash_result<32>::seed;
constexpr hash_result<64>::type hash_result<64>::seed;

namespace detail {

inline hash_result<64>::type
getblock( hash_result<64>::type const* p, int i )
{
    return p[i];
}

inline hash_result<32>::type
getblock( hash_result<32>::type const* p, int i )
{
    return p[i];
}

inline hash_result<64>::type
rotate(hash_result<64>::type x, hash_result<64>::type r)
{
    return (x << r) | (x >> (64 - r));
}

inline hash_result<32>::type
rotate(hash_result<32>::type x, hash_result<32>::type r)
{
    return (x << r) | (x >> (32 - r));
}

inline hash_result<64>::type
fmix(hash_result<64>::type k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;

    return k;
}

inline hash_result<32>::type
fmix(hash_result<32>::type k)
{
    k ^= k >> 16;
    k *= 0x85ebca6b;
    k ^= k >> 13;
    k *= 0xc2b2ae35;
    k ^= k >> 16;

    return k;
}

}  /* namespace detail */

hash_result<16>::type
hash_result<16>::bytes(void const* ptr, ::std::size_t length, type seed_val)
{
    return 0;
}

hash_result<32>::type
hash_result<32>::bytes(void const* ptr, ::std::size_t length, type seed_val)
{
    static type const c1    = 0xcc9e2d51U;
    static type const c2    = 0x1b873593U;
    static type const r1    = 15;
    static type const r2    = 13;
    static type const m     = 5;
    static type const n     = 0xe6546b64;
    static type const b_sz  = 4;

    ::std::uint8_t const* data = reinterpret_cast< ::std::uint8_t const* >(ptr);
    int const nblocks = length / b_sz;

    type hash = seed_val;
    //--------
    // body
    type const* blocks = reinterpret_cast<type const*>(data);

    for (int i = 0; i < nblocks; ++i) {
        auto k = detail::getblock(blocks, i);
        k *= c1;
        k = detail::rotate(k, r1);
        k *= c2;

        hash ^= k;
        hash = detail::rotate(hash, r2) * m + n;
    }

    //--------
    // tail
    ::std::uint8_t const* tail = reinterpret_cast<::std::uint8_t const*>(data + nblocks*b_sz);
    type k = 0;

    switch (length & 3) {
        case 3: k ^= static_cast<type>(tail[2]) << 16;
        case 2: k ^= static_cast<type>(tail[1]) <<  8;
        case 1: k ^= static_cast<type>(tail[0]) <<  0;
                k *= c1;
                k = detail::rotate(k, r1);
                k *= c2;
                hash ^= k;
    }

    hash ^= length;
    hash = detail::fmix(hash);

    return hash;
}

hash_result<64>::type
hash_result<64>::bytes(void const* ptr, ::std::size_t length, type seed_val)
{
    static type const c1    = 0x87c37b91114253d5ULL;
    static type const c2    = 0x4cf5ad432745937fULL;
    static type const r1    = 31;
    static type const r2    = 27;
    static type const r3    = 33;
    static type const r4    = 31;
    static type const m     = 5;
    static type const n1    = 0x52dce729;
    static type const n2    = 0x38495ab5;
    static type const b_sz  = 16;

    ::std::uint8_t const* data = reinterpret_cast< ::std::uint8_t const* >(ptr);
    int const nblocks = length / b_sz;

    type h1 = seed_val;
    type h2 = seed_val;
    //--------
    // body
    type const* blocks = reinterpret_cast<type const*>(data);

    for (int i = 0; i < nblocks; ++i) {
        type k1 = detail::getblock(blocks, i*2 + 0);
        type k2 = detail::getblock(blocks, i*2 + 1);

        k1 *= c1; k1 = detail::rotate(k1, r1); k1 *= c2; h1 ^= k1;

        h1 = detail::rotate(h1, r2); h1 += h2; h1 = h1 * m + n1;

        k2 *= c2; k2 = detail::rotate(k2, r3); k2 *= c1; h2 ^= k2;

        h2 = detail::rotate(h2, r4); h2 += h1; h2 = h2 * m + n2;
    }

    //--------
    // tail
    ::std::uint8_t const* tail = reinterpret_cast<::std::uint8_t const*>(data + nblocks*b_sz);

    type k1 = 0;
    type k2 = 0;

    switch (length & 15) {
        case 15: k2 ^= static_cast<type>(tail[14]) << 48;
        case 14: k2 ^= static_cast<type>(tail[13]) << 40;
        case 13: k2 ^= static_cast<type>(tail[12]) << 32;
        case 12: k2 ^= static_cast<type>(tail[11]) << 24;
        case 11: k2 ^= static_cast<type>(tail[10]) << 16;
        case 10: k2 ^= static_cast<type>(tail[ 9]) <<  8;
        case  9: k2 ^= static_cast<type>(tail[ 8]) <<  0;
                 k2 *= c2; k2 = detail::rotate(k2, 33);
                 k2 *= c1; h2 ^= k2;

        case  8: k1 ^= static_cast<type>(tail[ 7]) << 56;
        case  7: k1 ^= static_cast<type>(tail[ 6]) << 48;
        case  6: k1 ^= static_cast<type>(tail[ 5]) << 40;
        case  5: k1 ^= static_cast<type>(tail[ 4]) << 32;
        case  4: k1 ^= static_cast<type>(tail[ 3]) << 24;
        case  3: k1 ^= static_cast<type>(tail[ 2]) << 16;
        case  2: k1 ^= static_cast<type>(tail[ 1]) <<  8;
        case  1: k1 ^= static_cast<type>(tail[ 0]) <<  0;
                 k1 *= c1; k1 = detail::rotate(k1, 31);
                 k1 *= c1; h1 ^= k1;
    }

    //--------
    // finalization
    h1 ^= length; h2 ^= length;

    h1 += h2;
    h2 += h1;

    h1 = detail::fmix(h1);
    h2 = detail::fmix(h2);

    h1 += h2;
    h2 += h1;

    return h1 ^ h2;
}

}  /* namespace hash */
}  /* namespace wire */
