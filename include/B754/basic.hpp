/* -----
 * Baddy754 -> Basic
 *
 * Basic operations.
 *
 * Creator: ntcd_lol
 * License: MIT
 *
 * For people by people with love :heart: :3
 * -----
 */

/* -----
 * We have 2 types of number and variations of function:
 *
 *  1. F Version
 *    - Float variant, 32-bit.
 *
 *  2. D Version
 *    - Double float variant, 64-bit.
 *
 *  I Version
 *    - Int variant, 32-bit. [DEPRECATED: Baddy754 only for IEEE 754 Tricks.] 
 *
 * -----
 */

#include <cstdint>
#include <bit>

#define FFunc   [[maybe_unused]] constexpr float
#define DFunc   [[maybe_unused]] constexpr double

#define U32Func [[maybe_unused]] constexpr uint32_t
#define U64Func [[maybe_unused]] constexpr uint64_t

namespace baddy754::basic {

    FFunc abs(float x);
    DFunc abs(double x);

    FFunc neg(float x);
    DFunc neg(double x);

    constexpr bool signbit(float x);
    constexpr bool isnan(float x);
    constexpr bool isinf(float x);
    constexpr bool isfinite(float x);

    U32Func bits(float x);
    U64Func bits(double x);
    FFunc from_bits(uint32_t x);
    DFunc from_bits(uint64_t x);

    U32Func exponent(float x);
    U32Func mantissa(float x);
    U32Func sign(float x);

    FFunc set_exponent(float x, int exponent);
    FFunc fast_mul2(float x, int n);
    FFunc fast_div2(float x, int n);

}

U32Func baddy754::basic::bits(float x) noexcept { // 32-bit
    return std::bit_cast<uint32_t>(x);
}

U64Func baddy754::basic::bits(double x) noexcept { // 64-bit
    return std::bit_cast<uint64_t>(x); 
}

FFunc baddy754::basic::from_bits(uint32_t x) noexcept { // F Version
    return std::bit_cast<float>(x);
}

DFunc baddy754::basic::from_bits(uint64_t x) noexcept { // D Version
    return std::bit_cast<double>(x);
}

FFunc baddy754::basic::neg(float x) noexcept { // F Version
    return std::bit_cast<float>(
        std::bit_cast<std::uint32_t>(x) ^ 0x80000000u
    );
}

DFunc baddy754::basic::neg(double x) noexcept { // D Version
    return std::bit_cast<double>(
        std::bit_cast<std::uint64_t>(x) ^ 0x8000000000000000ull
    );
}

FFunc baddy754::basic::abs(float x) noexcept { // F Version
    return from_bits(bits(x) & 0x7fffffffu);
}

DFunc baddy754::basic::abs(double x) noexcept { // D Version
    return from_bits(bits(x) & 0x7fffffffffffffffull);
}
