// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// _BacktestEngineCustomVisualizerService.cpp : Implementation of CBacktestEngineCustomVisualizerService

#define BOOST_ALL_NO_LIB
#define BOOST_INT128_ALLOW_SIGN_CONVERSION
#include <boost/int128.hpp>
#include <boost/int128/charconv.hpp>

char* toChars(char* ptr, char* ptrEnd, boost::int128::uint128_t u128)
{
    auto r = boost::charconv::to_chars(ptr, ptrEnd, u128);
    *r.ptr = '\0';
    return r.ptr;
}

char* toChars(char* ptr, char* ptrEnd, boost::int128::int128_t i128)
{
#if 0
    auto r = boost::charconv::to_chars(ptr, ptrEnd, i128);
    *r.ptr = '\0';
    return r.ptr;
#else
    // workaround for boost::charconv bug:https://github.com/boostorg/charconv/pull/289
    boost::int128::uint128_t u128;
    if (i128 < 0)
    {
        *ptr++ = '-';
        if (i128 == std::numeric_limits<boost::int128::int128_t>::min())
            u128 = boost::int128::uint128_t{std::numeric_limits<boost::int128::int128_t>::max()} + 1U;
        else
            u128 = boost::int128::uint128_t{i128 * -1};
    }
    else
        u128 = boost::int128::uint128_t{i128};
    return toChars(ptr, ptrEnd, u128);
#endif
}
