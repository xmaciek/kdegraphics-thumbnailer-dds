// MIT License
//
// Copyright (c) 2024 Maciej Latocha <latocha.maciek@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstdint>
#include <cassert>
#include <tuple>
#include <algorithm>

namespace {

using uint128_t = unsigned __int128;
static_assert( alignof( uint128_t ) == 16 );

constexpr inline uint8_t FIXUP_INDICES_2_SUBSETS[ 64 ] {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  2,  8,  2,  2,  8,  8, 15,  2,  8,  2,  2,  8,  8,  2,  2,
    15, 15,  6,  8,  2,  8, 15, 15,  2,  8,  2,  2,  2, 15, 15,  6,
    6,   2,  6,  8, 15, 15,  2,  2, 15, 15, 15, 15, 15,  2,  2, 15,
};

constexpr inline std::tuple<uint8_t, uint8_t> FIXUP_INDICES_3_SUBSETS[ 64 ] {
    { 3, 15 }, { 3, 8 },   { 15, 8 }, { 15, 3 },  { 8, 15 }, { 3, 15 },  { 15, 3 },  { 15, 8 },
    { 8, 15 }, { 8, 15 },  { 6, 15 }, { 6, 15 },  { 6, 15 }, { 5, 15 },  { 3, 15 },  { 3, 8 },
    { 3, 15 }, { 3, 8 },   { 8, 15 }, { 15, 3 },  { 3, 15 }, { 3, 8 },   { 6, 15 },  { 10, 8 },
    { 5, 3 },  { 8, 15 },  { 8, 6 },  { 6, 10 },  { 8, 15 }, { 5, 15 },  { 15, 10 }, { 15, 8 },
    { 8, 15 }, { 15, 3 },  { 3, 15 }, { 5, 10 },  { 6, 10 }, { 10, 8 },  { 8, 9 },   { 15, 10 },
    { 15, 6 }, { 3, 15 },  { 15, 8 }, { 5, 15 },  { 15, 3 }, { 15, 6 },  { 15, 6 },  { 15, 8 },
    { 3, 15 }, { 15, 3 },  { 5, 15 }, { 5, 15 },  { 5, 15 }, { 8, 15 },  { 5, 15 },  { 10, 15 },
    { 5, 15 }, { 10, 15 }, { 8, 15 }, { 13, 15 }, { 15, 3 }, { 12, 15 }, { 3, 15 },  { 3, 8 },
};

constexpr inline uint8_t BC7_PARTITION_2_SUBSETS[ 64 ][ 16 ] {
    { 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
    { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 }, { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 },
    { 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
    { 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 }, { 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 }, { 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
    { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 }, { 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 }, { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
    { 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 }, { 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 }, { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 }, { 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
    { 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 }, { 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 }, { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 }, { 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
    { 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
    { 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
    { 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 }, { 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 }, { 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 }, { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 },
};

constexpr inline uint8_t BC7_PARTITION_3_SUBSETS[ 64 ][ 16 ] {
    { 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 1, 2, 2, 2, 2 }, { 0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1 },
    { 0, 0, 0, 0, 2, 0, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1 }, { 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2 }, { 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 2, 2 },
    { 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 }, { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2 },
    { 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2 }, { 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2 },
    { 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2 }, { 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0, 2, 2, 2, 0 },
    { 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2 }, { 0, 1, 1, 1, 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2 }, { 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2, 0, 2, 2, 2 }, { 0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2 }, { 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 2, 2, 1, 0 },
    { 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0 }, { 0, 0, 1, 2, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2 },
    { 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1 },
    { 0, 0, 2, 2, 1, 1, 0, 2, 1, 1, 0, 2, 0, 0, 2, 2 }, { 0, 1, 1, 0, 0, 1, 1, 0, 2, 0, 0, 2, 2, 2, 2, 2 },
    { 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1 }, { 0, 0, 0, 0, 2, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 2, 2, 2 }, { 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 2, 0, 0, 1, 1 },
    { 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 2, 2 }, { 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 }, { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 },
    { 0, 1, 2, 0, 2, 0, 1, 2, 1, 2, 0, 1, 0, 1, 2, 0 }, { 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1 },
    { 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1 }, { 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1 }, { 0, 0, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2, 1, 1, 2, 2 },
    { 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1 }, { 0, 2, 2, 0, 1, 2, 2, 1, 0, 2, 2, 0, 1, 2, 2, 1 },
    { 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1 }, { 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2 }, { 0, 2, 2, 2, 0, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1 },
    { 0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2 }, { 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2 },
    { 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2 }, { 0, 0, 0, 2, 1, 1, 1, 2, 1, 1, 1, 2, 0, 0, 0, 2 },
    { 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2 }, { 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2 },
    { 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2 },
    { 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2 },
    { 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1 }, { 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2 },
    { 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 1, 1, 1, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 2, 0 },
};

inline uint8_t lerp2bit( uint16_t e0, uint16_t e1, uint16_t index )
{
    static const uint16_t WEIGHTS[ 4 ]{ 0, 21, 43, 64 };
    const uint16_t w = WEIGHTS[ index ];
    return static_cast<uint8_t>( ( ( 64 - w ) * e0 + w * e1 + 32 ) >> 6 );
}

inline uint8_t lerp3bit( uint16_t e0, uint16_t e1, uint16_t indice )
{
    static const uint16_t WEIGHTS[ 8 ]{ 0, 9, 18, 27, 37, 46, 55, 64 };
    const uint16_t w = WEIGHTS[ indice ];
    return static_cast<uint8_t>( ( ( 64 - w ) * e0 + w * e1 + 32 ) >> 6 );
}

inline uint8_t lerp4bit( uint16_t e0, uint16_t e1, uint16_t indice )
{
    static const uint16_t WEIGHTS[ 16 ]{ 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };
    const uint16_t w = WEIGHTS[ indice ];
    return static_cast<uint8_t>( ( ( 64 - w ) * e0 + w * e1 + 32 ) >> 6 );
}

// NOTE: inserts 0 into specified fixup position
constexpr inline uint64_t fixupIndices( uint64_t indices, uint8_t fixup )
{
    uint64_t mask = ~0ull << fixup;
    uint64_t data = indices & mask;
    data <<= 1;
    data |= indices & ~mask;
    return data;
}
static_assert( fixupIndices( 0b1111, 2 ) == 0b11011 );

template <size_t TSIZE>
constexpr inline uint8_t readIndice( uint64_t indices, uint8_t index )
{
    const uint8_t mask = ~( ~0ull << TSIZE );
    return (uint8_t)( indices >> ( index * TSIZE ) ) & mask;
}
static_assert( readIndice<3>( 0b1100110, 1 ) == 0b100 );

template <size_t a, size_t b, size_t c>
constexpr inline uint8_t unpackComponent( uint8_t component, uint8_t pbit )
{
    uint8_t ret = component << a;
    ret |= pbit << b;
    ret |= component >> c;
    return ret;
}
static_assert( unpackComponent<2,1,5>( 0b111100, 0 ) == 0b11110001 );

struct alignas( 16 ) BC7 {
    struct Mode0 {
        uint128_t mode : 1;
        uint128_t partition : 4;
        uint128_t bitsR0 : 4; uint128_t bitsR1 : 4; uint128_t bitsR2 : 4; uint128_t bitsR3 : 4; uint128_t bitsR4 : 4; uint128_t bitsR5 : 4;
        uint128_t bitsG0 : 4; uint128_t bitsG1 : 4; uint128_t bitsG2 : 4; uint128_t bitsG3 : 4; uint128_t bitsG4 : 4; uint128_t bitsG5 : 4;
        uint128_t bitsB0 : 4; uint128_t bitsB1 : 4; uint128_t bitsB2 : 4; uint128_t bitsB3 : 4; uint128_t bitsB4 : 4; uint128_t bitsB5 : 4;
        uint128_t bitsP0 : 1; uint128_t bitsP1 : 1; uint128_t bitsP2 : 1; uint128_t bitsP3 : 1; uint128_t bitsP4 : 1; uint128_t bitsP5 : 1;
        uint128_t bitsIndex : 45;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<4, 3, 1>;
            const uint8_t subset = BC7_PARTITION_3_SUBSETS[ partition ][ index ];
            const auto [fixup1, fixup2] = FIXUP_INDICES_3_SUBSETS[ partition ];
            uint64_t indices = bitsIndex;
            indices = fixupIndices( indices, 2 );
            indices = fixupIndices( indices, fixup1 * 3 + 2 );
            indices = fixupIndices( indices, fixup2 * 3 + 2 );
            uint8_t indice = readIndice<3>( indices, index );
            switch ( subset ) {
            case 0: return colorfn::makeARGB8888(
                lerp3bit( unpack( bitsR0, bitsP0 ), unpack( bitsR1, bitsP1 ), indice ),
                lerp3bit( unpack( bitsG0, bitsP0 ), unpack( bitsG1, bitsP1 ), indice ),
                lerp3bit( unpack( bitsB0, bitsP0 ), unpack( bitsB1, bitsP1 ), indice ),
                255 );

            case 1: return colorfn::makeARGB8888(
                lerp3bit( unpack( bitsR2, bitsP2 ), unpack( bitsR3, bitsP3 ), indice ),
                lerp3bit( unpack( bitsG2, bitsP2 ), unpack( bitsG3, bitsP3 ), indice ),
                lerp3bit( unpack( bitsB2, bitsP2 ), unpack( bitsB3, bitsP3 ), indice ),
                255 );

            case 2: return colorfn::makeARGB8888(
                lerp3bit( unpack( bitsR4, bitsP4 ), unpack( bitsR5, bitsP5 ), indice ),
                lerp3bit( unpack( bitsG4, bitsP4 ), unpack( bitsG5, bitsP5 ), indice ),
                lerp3bit( unpack( bitsB4, bitsP4 ), unpack( bitsB5, bitsP5 ), indice ),
                255 );

            [[unlikely]] default: return 0;
            }
        }
    };

    struct Mode1 {
        uint128_t mode : 2;
        uint128_t partition : 6;
        uint128_t bitsR0 : 6; uint128_t bitsR1 : 6; uint128_t bitsR2 : 6; uint128_t bitsR3 : 6;
        uint128_t bitsG0 : 6; uint128_t bitsG1 : 6; uint128_t bitsG2 : 6; uint128_t bitsG3 : 6;
        uint128_t bitsB0 : 6; uint128_t bitsB1 : 6; uint128_t bitsB2 : 6; uint128_t bitsB3 : 6;
        uint128_t bitsP0 : 1; uint128_t bitsP1 : 1;
        uint128_t bitsIndex : 46;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<2, 1, 5>;
            const uint8_t subset = BC7_PARTITION_2_SUBSETS[ partition ][ index ];
            const uint8_t fixup = FIXUP_INDICES_2_SUBSETS[ partition ];
            uint64_t indices = bitsIndex;
            indices = fixupIndices( indices, 2 );
            indices = fixupIndices( indices, fixup * 3 + 2 );
            uint8_t indice = readIndice<3>( indices, index );
            switch ( subset ) {
            case 0: return colorfn::makeARGB8888(
                lerp3bit( unpack( bitsR0, bitsP0 ), unpack( bitsR1, bitsP0 ), indice ),
                lerp3bit( unpack( bitsG0, bitsP0 ), unpack( bitsG1, bitsP0 ), indice ),
                lerp3bit( unpack( bitsB0, bitsP0 ), unpack( bitsB1, bitsP0 ), indice ),
                255 );

            case 1: return colorfn::makeARGB8888(
                lerp3bit( unpack( bitsR2, bitsP1 ), unpack( bitsR3, bitsP1 ), indice ),
                lerp3bit( unpack( bitsG2, bitsP1 ), unpack( bitsG3, bitsP1 ), indice ),
                lerp3bit( unpack( bitsB2, bitsP1 ), unpack( bitsB3, bitsP1 ), indice ),
                255 );

            [[unlikely]] default: return 0;
            }
        }
    };

    struct Mode2 {
        uint128_t mode : 3;
        uint128_t partition : 6;
        uint128_t bitsR0 : 5; uint128_t bitsR1 : 5; uint128_t bitsR2 : 5; uint128_t bitsR3 : 5; uint128_t bitsR4 : 5; uint128_t bitsR5 : 5;
        uint128_t bitsG0 : 5; uint128_t bitsG1 : 5; uint128_t bitsG2 : 5; uint128_t bitsG3 : 5; uint128_t bitsG4 : 5; uint128_t bitsG5 : 5;
        uint128_t bitsB0 : 5; uint128_t bitsB1 : 5; uint128_t bitsB2 : 5; uint128_t bitsB3 : 5; uint128_t bitsB4 : 5; uint128_t bitsB5 : 5;
        uint128_t bitsIndex : 29;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<3, 0, 2>;
            const uint8_t subset = BC7_PARTITION_3_SUBSETS[ partition ][ index ];
            const auto [fixup1, fixup2] = FIXUP_INDICES_3_SUBSETS[ partition ];
            uint64_t indices = bitsIndex;
            indices = fixupIndices( indices, 1 );
            indices = fixupIndices( indices, fixup1 * 2 + 1 );
            indices = fixupIndices( indices, fixup2 * 2 + 1 );
            uint8_t indice = readIndice<2>( indices, index );
            switch ( subset ) {
            case 0: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR0, 0 ), unpack( bitsR1, 0 ), indice ),
                lerp2bit( unpack( bitsG0, 0 ), unpack( bitsG1, 0 ), indice ),
                lerp2bit( unpack( bitsB0, 0 ), unpack( bitsB1, 0 ), indice ),
                255 );

            case 1: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR2, 0 ), unpack( bitsR3, 0 ), indice ),
                lerp2bit( unpack( bitsG2, 0 ), unpack( bitsG3, 0 ), indice ),
                lerp2bit( unpack( bitsB2, 0 ), unpack( bitsB3, 0 ), indice ),
                255 );

            case 2: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR4, 0 ), unpack( bitsR5, 0 ), indice ),
                lerp2bit( unpack( bitsG4, 0 ), unpack( bitsG5, 0 ), indice ),
                lerp2bit( unpack( bitsB4, 0 ), unpack( bitsB5, 0 ), indice ),
                255 );

            [[unlikely]] default: return 0;
            }
        }
    };

    struct Mode3 {
        uint128_t mode : 4;
        uint128_t partition : 6;
        uint128_t bitsR0 : 7; uint128_t bitsR1 : 7; uint128_t bitsR2 : 7; uint128_t bitsR3 : 7;
        uint128_t bitsG0 : 7; uint128_t bitsG1 : 7; uint128_t bitsG2 : 7; uint128_t bitsG3 : 7;
        uint128_t bitsB0 : 7; uint128_t bitsB1 : 7; uint128_t bitsB2 : 7; uint128_t bitsB3 : 7;
        uint128_t bitsP0 : 1; uint128_t bitsP1 : 1; uint128_t bitsP2 : 1; uint128_t bitsP3 : 1;
        uint128_t bitsIndex : 30;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<1, 0, 8>;
            const uint8_t subset = BC7_PARTITION_2_SUBSETS[ partition ][ index ];
            const uint8_t fixup = FIXUP_INDICES_2_SUBSETS[ partition ];
            uint64_t indices = bitsIndex;
            indices = fixupIndices( indices, 1 );
            indices = fixupIndices( indices, fixup * 2 + 1 );
            uint8_t indice = readIndice<2>( indices, index );
            switch ( subset ) {
            case 0: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR0, bitsP0 ), unpack( bitsR1, bitsP1 ), indice ),
                lerp2bit( unpack( bitsG0, bitsP0 ), unpack( bitsG1, bitsP1 ), indice ),
                lerp2bit( unpack( bitsB0, bitsP0 ), unpack( bitsB1, bitsP1 ), indice ),
                255 );

            case 1: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR2, bitsP2 ), unpack( bitsR3, bitsP3 ), indice ),
                lerp2bit( unpack( bitsG2, bitsP2 ), unpack( bitsG3, bitsP3 ), indice ),
                lerp2bit( unpack( bitsB2, bitsP2 ), unpack( bitsB3, bitsP3 ), indice ),
                255 );

            [[unlikely]] default: return 0;
            }
        }
    };

    struct Mode4 {
        uint128_t mode : 5;
        uint128_t rotation : 2;
        uint128_t idxMode : 1;
        uint128_t bitsR0 : 5; uint128_t bitsR1 : 5;
        uint128_t bitsG0 : 5; uint128_t bitsG1 : 5;
        uint128_t bitsB0 : 5; uint128_t bitsB1 : 5;
        uint128_t bitsA0 : 6; uint128_t bitsA1 : 6;
        uint128_t bitsIndices1 : 31;
        uint128_t bitsIndices2 : 47;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpackC = unpackComponent<3, 0, 2>;
            static constexpr auto& unpackA = unpackComponent<2, 0, 4>;
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;
            uint8_t a = 0;
            const uint64_t indices1 = fixupIndices( bitsIndices1, 1 );
            const uint64_t indices2 = fixupIndices( bitsIndices2, 2 );
            const uint8_t indice1 = readIndice<2>( indices1, index );
            const uint8_t indice2 = readIndice<3>( indices2, index );
            switch ( idxMode ) {
            case 0:
                r = lerp2bit( unpackC( bitsR0, 0 ), unpackC( bitsR1, 0 ), indice1 );
                g = lerp2bit( unpackC( bitsG0, 0 ), unpackC( bitsG1, 0 ), indice1 );
                b = lerp2bit( unpackC( bitsB0, 0 ), unpackC( bitsB1, 0 ), indice1 );
                a = lerp3bit( unpackA( bitsA0, 0 ), unpackA( bitsA1, 0 ), indice2 );
                break;
            case 1:
                r = lerp3bit( unpackC( bitsR0, 0 ), unpackC( bitsR1, 0 ), indice2 );
                g = lerp3bit( unpackC( bitsG0, 0 ), unpackC( bitsG1, 0 ), indice2 );
                b = lerp3bit( unpackC( bitsB0, 0 ), unpackC( bitsB1, 0 ), indice2 );
                a = lerp2bit( unpackA( bitsA0, 0 ), unpackA( bitsA1, 0 ), indice1 );
                break;
            }

            switch ( rotation ) {
            default: break;
            case 0b01: std::swap( a, r ); break;
            case 0b10: std::swap( a, g ); break;
            case 0b11: std::swap( a, b ); break;
            }
            return colorfn::makeARGB8888( r, g, b, a );
        }
    };

    struct Mode5 {
        uint128_t mode : 6;
        uint128_t rotation : 2;
        uint128_t bitsR0 : 7; uint128_t bitsR1 : 7;
        uint128_t bitsG0 : 7; uint128_t bitsG1 : 7;
        uint128_t bitsB0 : 7; uint128_t bitsB1 : 7;
        uint128_t bitsA0 : 8; uint128_t bitsA1 : 8;
        uint128_t bitsColorIndex : 31;
        uint128_t bitsAlphaIndex : 31;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<1, 0, 6>;
            const uint64_t indicesC = fixupIndices( bitsColorIndex, 1 );
            const uint64_t indicesA = fixupIndices( bitsAlphaIndex, 1 );
            const uint8_t indiceC = readIndice<2>( indicesC, index );
            const uint8_t indiceA = readIndice<2>( indicesA, index );
            uint8_t r = lerp2bit( unpack( bitsR0, 0 ), unpack( bitsR1, 0 ), indiceC );
            uint8_t g = lerp2bit( unpack( bitsG0, 0 ), unpack( bitsG1, 0 ), indiceC );
            uint8_t b = lerp2bit( unpack( bitsB0, 0 ), unpack( bitsB1, 0 ), indiceC );
            uint8_t a = lerp2bit( bitsA0, bitsA1, indiceA );
            switch ( rotation ) {
            default: break;
            case 0b01: std::swap( a, r ); break;
            case 0b10: std::swap( a, g ); break;
            case 0b11: std::swap( a, b ); break;
            }
            return colorfn::makeARGB8888( r, g, b, a );
        }
    };

    struct Mode6 {
        uint128_t mode : 7;
        uint128_t bitsR0 : 7; uint128_t bitsR1 : 7;
        uint128_t bitsG0 : 7; uint128_t bitsG1 : 7;
        uint128_t bitsB0 : 7; uint128_t bitsB1 : 7;
        uint128_t bitsA0 : 7; uint128_t bitsA1 : 7;
        uint128_t bitsP0 : 1; uint128_t bitsP1 : 1;
        uint128_t bitsIndex : 63;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<1, 0, 8>;
            const uint64_t indices = fixupIndices( bitsIndex, 3 );
            const uint8_t indice = readIndice<4>( indices, index );
            uint8_t r = lerp4bit( unpack( bitsR0, bitsP0 ), unpack( bitsR1, bitsP1 ), indice );
            uint8_t g = lerp4bit( unpack( bitsG0, bitsP0 ), unpack( bitsG1, bitsP1 ), indice );
            uint8_t b = lerp4bit( unpack( bitsB0, bitsP0 ), unpack( bitsB1, bitsP1 ), indice );
            uint8_t a = lerp4bit( unpack( bitsA0, bitsP0 ), unpack( bitsA1, bitsP1 ), indice );
            return colorfn::makeARGB8888( r, g, b, a );
        }
    };

    struct Mode7 {
        uint128_t mode : 8;
        uint128_t partition : 6;
        uint128_t bitsR0 : 5; uint128_t bitsR1 : 5; uint128_t bitsR2 : 5; uint128_t bitsR3 : 5;
        uint128_t bitsG0 : 5; uint128_t bitsG1 : 5; uint128_t bitsG2 : 5; uint128_t bitsG3 : 5;
        uint128_t bitsB0 : 5; uint128_t bitsB1 : 5; uint128_t bitsB2 : 5; uint128_t bitsB3 : 5;
        uint128_t bitsA0 : 5; uint128_t bitsA1 : 5; uint128_t bitsA2 : 5; uint128_t bitsA3 : 5;
        uint128_t bitsP0 : 1; uint128_t bitsP1 : 1; uint128_t bitsP2 : 1; uint128_t bitsP3 : 1;
        uint128_t bitsIndex : 30;

        uint32_t operator [] ( uint32_t index ) const
        {
            assert( __builtin_popcount( mode ) == 1 );
            static constexpr auto& unpack = unpackComponent<3, 1, 2>;
            const uint8_t subset = BC7_PARTITION_2_SUBSETS[ partition ][ index ];
            const uint8_t fixup = FIXUP_INDICES_2_SUBSETS[ partition ];
            uint64_t indices = bitsIndex;
            indices = fixupIndices( indices, 1 );
            indices = fixupIndices( indices, fixup * 2 + 1 );
            uint8_t indice = readIndice<2>( indices, index );
            switch ( subset ) {
            case 0: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR0, bitsP0 ), unpack( bitsR1, bitsP1 ), indice ),
                lerp2bit( unpack( bitsG0, bitsP0 ), unpack( bitsG1, bitsP1 ), indice ),
                lerp2bit( unpack( bitsB0, bitsP0 ), unpack( bitsB1, bitsP1 ), indice ),
                lerp2bit( unpack( bitsA0, bitsP0 ), unpack( bitsA1, bitsP1 ), indice )
                );
            case 1: return colorfn::makeARGB8888(
                lerp2bit( unpack( bitsR2, bitsP2 ), unpack( bitsR3, bitsP3 ), indice ),
                lerp2bit( unpack( bitsG2, bitsP2 ), unpack( bitsG3, bitsP3 ), indice ),
                lerp2bit( unpack( bitsB2, bitsP2 ), unpack( bitsB3, bitsP3 ), indice ),
                lerp2bit( unpack( bitsA2, bitsP2 ), unpack( bitsA3, bitsP3 ), indice )
                );

            [[unlikely]] default: return 0;
            }
        }
    };

    union {
        uint8_t raw[ 16 ];
        Mode0 mode0;
        Mode1 mode1;
        Mode2 mode2;
        Mode3 mode3;
        Mode4 mode4;
        Mode5 mode5;
        Mode6 mode6;
        Mode7 mode7;
    };

    uint32_t operator [] ( uint32_t i ) const
    {
        const uint8_t mode = raw[ 0 ];
        switch ( mode ? __builtin_ctz( mode ) : 8 ) {
        case 0: return mode0[ i ];
        case 1: return mode1[ i ];
        case 2: return mode2[ i ];
        case 3: return mode3[ i ];
        case 4: return mode4[ i ];
        case 5: return mode5[ i ];
        case 6: return mode6[ i ];
        case 7: return mode7[ i ];
        [[unlikely]] default:
            assert( !"BC7 block corrupted, expected at least 1 bit set in mode field" );
            return colorfn::makeARGB8888( 0xD4, 0x21, 0x3D, 0xFF );
        }
    }
};
static_assert( sizeof( BC7 ) == 16, "sizeof BC7 not equal 16" );

} // namespace
