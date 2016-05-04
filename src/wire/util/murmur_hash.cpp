/*
 * murmur_hash.cpp
 *
 *  Created on: May 4, 2016
 *      Author: zmij
 */

#include <wire/util/murmur_hash.hpp>

namespace wire {
namespace hash {

namespace detail {

inline hash_result
getblock( ::std::uint64_t const* p, int i )
{
    return p[i]; // TODO endian-swapping
}

inline hash_result
rotate(::std::uint64_t x, ::std::int64_t r)
{
    return (x << r) | (x >> (64 -r));
}

inline hash_result
fmix(hash_result k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;

    return k;
}

hash_result
murmur_bytes(void const* ptr, ::std::size_t length, hash_result seed)
{
    ::std::uint8_t const* data = reinterpret_cast< ::std::uint8_t const* >(ptr);
    int const nblocks = length / 16;

    hash_result h1 = seed;
    hash_result h2 = seed;

    hash_result const c1 = 0x87c37b91114253d5ULL;
    hash_result const c2 = 0x4cf5ad432745937fULL;

    //--------
    // body
    ::std::uint64_t const* blocks = reinterpret_cast<::std::uint64_t const*>(data);

    for (int i = 0; i < nblocks; ++i) {
        hash_result k1 = getblock(blocks, i*2 + 0);
        hash_result k2 = getblock(blocks, i*2 + 1);

        k1 *= c1; k1 = rotate(k1, 31); k1 *= c2; h1 ^= k1;

        h1 = rotate(h1, 27); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2 = rotate(k2, 33); k2 *= c1; h2 ^= k2;

        h2 = rotate(h2, 31); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    //--------
    // tail
    ::std::uint8_t const* tail = reinterpret_cast<::std::uint8_t const*>(data + nblocks*16);

    hash_result k1 = 0;
    hash_result k2 = 0;

    switch (length & 15) {
        case 15: k2 ^= static_cast<hash_result>(tail[14]) << 48;
        case 14: k2 ^= static_cast<hash_result>(tail[13]) << 40;
        case 13: k2 ^= static_cast<hash_result>(tail[12]) << 32;
        case 12: k2 ^= static_cast<hash_result>(tail[11]) << 24;
        case 11: k2 ^= static_cast<hash_result>(tail[10]) << 16;
        case 10: k2 ^= static_cast<hash_result>(tail[ 9]) <<  8;
        case  9: k2 ^= static_cast<hash_result>(tail[ 8]) <<  0;
                 k2 *= c2; k2 = rotate(k2, 33); k2 *= c1; h2 ^= k2;

        case  8: k1 ^= static_cast<hash_result>(tail[ 7]) << 56;
        case  7: k1 ^= static_cast<hash_result>(tail[ 6]) << 48;
        case  6: k1 ^= static_cast<hash_result>(tail[ 5]) << 40;
        case  5: k1 ^= static_cast<hash_result>(tail[ 4]) << 32;
        case  4: k1 ^= static_cast<hash_result>(tail[ 3]) << 24;
        case  3: k1 ^= static_cast<hash_result>(tail[ 2]) << 16;
        case  2: k1 ^= static_cast<hash_result>(tail[ 1]) <<  8;
        case  1: k1 ^= static_cast<hash_result>(tail[ 0]) <<  0;
                 k1 *= c1; k1 = rotate(k1, 31); k1 *= c1; h1 ^= k1;
    }

    //--------
    // finalization
    h1 ^= length; h2 ^= length;

    h1 += h2;
    h2 += h1;

    h1 = fmix(h1);
    h2 = fmix(h2);

    h1 += h2;
    h2 += h1;

    return h1 ^ h2;
}

}  /* namespace detail */

}  /* namespace hash */
}  /* namespace wire */
