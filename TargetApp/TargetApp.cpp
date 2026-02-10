// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// TargetApp.cpp : Example application which will be debugged

#include <iostream>
#include <windows.h>
#include <memory>
#include "../../../src/symbol.h"
#define BOOST_INT128_ALLOW_SIGN_CONVERSION
#define BOOST_DECIMAL_DETAIL_INT128_ALLOW_SIGN_CONVERSION
#include <boost/decimal.hpp>

char* toChars(char* ptr, char* ptrEnd, boost::int128::int128_t i128);
char* toChars(char* ptr, char* ptrEnd, boost::int128::uint128_t u128);

inline std::string to_string(boost::int128::int128_t i128)
{
    char buf[64];
    char* e = toChars(buf, buf + sizeof(buf), i128);
    return std::string(buf, e);
}

inline std::string to_string(boost::int128::uint128_t u128)
{
    char buf[64];
    char* e = toChars(buf, buf + sizeof(buf), u128);
    return std::string(buf, e);
}

template<typename T>
void pr(const char* name, T t)
{
    using namespace std;
    printf("name: %s, val: %s", name, to_string(t).c_str());
    reinterpret_cast<uint64_t&>(t) |= (1ULL << 63);
    printf(", neg val: %s\n", to_string(t).c_str());
}

int wmain(int argc, WCHAR* argv[])
{
    Symbol0* sym0_spy = new Symbol0("spy");
    std::unique_ptr<Symbol0> sym0_ptr(sym0_spy);
    Symbol0 sym0_nylda0("NYLD.A");
    Symbol0 sym0_unknown0;

    Symbol1* sym1_spy = new Symbol1("spy");
    std::unique_ptr<Symbol1> sym1_ptr(sym1_spy);
    Symbol1 sym1_nylda1("NYLD.A");
    Symbol1 sym1_unknown1;

    Symbol2* sym2_spy = new Symbol2("spy");
    std::unique_ptr<Symbol2> sym2_ptr(sym2_spy);
    Symbol2 sym2_nylda2("NYLD.A");
    Symbol2 sym2_unknown2;

    std::vector<Symbol0> syms0 = { Symbol0("SPY"), Symbol0("aapl") };
    std::vector<Symbol1> syms1 = { Symbol1("SPY"), Symbol1("aapl") };
    std::vector<Symbol2> syms2 = { Symbol2("SPY"), Symbol2("aapl") };

    boost::int128::int128_t i128 = -123;
    boost::int128::uint128_t u128 = 123;

    for (int i=1; i < 30; ++i)
    {
        i128 *= i;
        u128 *= i;

        printf("i128 val: %s\n", to_string(i128).c_str());
        printf("u128 val: %s\n", to_string(u128).c_str());
    }

    printf("min i128 val: %s\n", to_string(std::numeric_limits<boost::int128::int128_t>::min()).c_str());
    printf("max i128 val: %s\n", to_string(std::numeric_limits<boost::int128::int128_t>::max()).c_str());
    printf("max u128 val: %s\n", to_string(std::numeric_limits<boost::int128::uint128_t>::max()).c_str());


    boost::decimal::decimal64_t snan, qnan, inf;
    reinterpret_cast<uint64_t&>(snan) = 0x7E00000000000000ull;
    reinterpret_cast<uint64_t&>(qnan) = 0x7C00000000000000ull;
    reinterpret_cast<uint64_t&>(inf)  = 0x7800000000000000ull;

    boost::decimal::decimal64_t neg_snan = snan, neg_qnan = qnan, neg_inf = inf;
    reinterpret_cast<uint64_t&>(neg_snan) |= (1ULL << 63);
    reinterpret_cast<uint64_t&>(neg_qnan) |= (1ULL << 63);
    reinterpret_cast<uint64_t&>(neg_inf) |= (1ULL << 63);

    pr("signaling_NaN", std::numeric_limits< boost::decimal::decimal64_t>::signaling_NaN());
    pr("quiet_NaN", std::numeric_limits<boost::decimal::decimal64_t>::quiet_NaN());
    pr("infinity", std::numeric_limits<boost::decimal::decimal64_t>::infinity());

    //pr("d signaling_NaN", std::numeric_limits<double>::signaling_NaN());
    //pr("d quiet_NaN", std::numeric_limits<double>::quiet_NaN());
    //pr("d infinity", std::numeric_limits<double>::infinity());

    boost::decimal::decimal64_t num1("123.456");
    auto z_snan = std::numeric_limits< boost::decimal::decimal64_t>::signaling_NaN();
    auto z_qnan = std::numeric_limits< boost::decimal::decimal64_t>::quiet_NaN();
    auto z_inf = std::numeric_limits< boost::decimal::decimal64_t>::infinity();

    __debugbreak(); // program will stop here. Evaluate 'sym2_spy', 'sym2_nylda' and 'sym2_unknown' in the locals or watch window.

    printf("Test complete\n");

    return 0;
}
