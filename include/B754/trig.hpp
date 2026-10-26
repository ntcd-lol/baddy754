
/* -----
 * Baddy754 -> Trigonometric
 *
 * Trigonometric operations.
 *
 * Creator: ntcd_lol
 * License: MIT
 *
 * For people by people with love :heart: :3
 * -----
 */

#pragma once

#include <bit>
#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>

#include "B754/basic.hpp"

namespace baddy754::trig {
namespace detail {

template <typename T>
struct ieee_traits;

template <>
struct ieee_traits<float> {
    using uint_type = std::uint32_t;

    static constexpr uint_type sign_mask = 0x80000000u;
    static constexpr uint_type exp_mask  = 0x7F800000u;
    static constexpr uint_type frac_mask = 0x007FFFFFu;
    static constexpr int exponent_bias = 127;
    static constexpr int mantissa_bits = 23;
};

template <>
struct ieee_traits<double> {
    using uint_type = std::uint64_t;

    static constexpr uint_type sign_mask = 0x8000000000000000ull;
    static constexpr uint_type exp_mask  = 0x7FF0000000000000ull;
    static constexpr uint_type frac_mask = 0x000FFFFFFFFFFFFFull;
    static constexpr int exponent_bias = 1023;
    static constexpr int mantissa_bits = 52;
};

template <typename T>
concept SupportedFloat =
    std::is_same_v<T, float> ||
    std::is_same_v<T, double>;

template <SupportedFloat T>
struct fp_info {
    using traits = ieee_traits<T>;
    using uint_type = typename traits::uint_type;

    uint_type bits;
    bool sign;
    uint_type exponent;
    uint_type fraction;
    bool is_special;
    bool is_subnormal;
};

template <SupportedFloat T>
inline fp_info<T> inspect(T x) noexcept {
    using tr = ieee_traits<T>;
    using U = typename tr::uint_type;

    const U bits = std::bit_cast<U>(x);
    const U exponent = (bits & tr::exp_mask) >> tr::mantissa_bits;
    const U fraction = bits & tr::frac_mask;

    return {
        bits,
        (bits & tr::sign_mask) != 0,
        exponent,
        fraction,
        exponent == (tr::exp_mask >> tr::mantissa_bits),
        exponent == 0 && fraction != 0
    };
}

/*
 * Taylor approximation:
 *
 * sin(x) ~= x * (1 + x² * P(x²))
 *
 * Intended for |x| <= pi/4.
 */
template <typename T>
inline T sin_kernel(T x) noexcept {
    const T z = x * x;

    if constexpr (std::is_same_v<T, float>) {
        return x * (
            1.0f + z * (
                -1.0f / 6.0f + z * (
                    1.0f / 120.0f + z * (
                        -1.0f / 5040.0f + z * (
                            1.0f / 362880.0f
                        )
                    )
                )
            )
        );
    } else {
        return x * (
            1.0 + z * (
                -1.0 / 6.0 + z * (
                    1.0 / 120.0 + z * (
                        -1.0 / 5040.0 + z * (
                            1.0 / 362880.0 + z * (
                                -1.0 / 39916800.0 + z * (
                                    1.0 / 6227020800.0
                                )
                            )
                        )
                    )
                )
            )
        );
    }
}

template <SupportedFloat T>
inline T sin_impl(T x) noexcept {
    constexpr T half_pi =
        static_cast<T>(1.57079632679489661923132169163975144L);
    constexpr T quarter_pi =
        static_cast<T>(0.78539816339744830961566084581987572L);

    const auto info = inspect(x);

    if (info.is_special) {
        return std::numeric_limits<T>::quiet_NaN();
    }

    if ((info.bits & ~ieee_traits<T>::sign_mask) == 0) {
        return x;
    }

    int quotient = 0;
    T r = std::remquo(x, half_pi, &quotient);

    switch (quotient & 3) {
        case 0:
            break;
        case 1:
            r = half_pi - r;
            break;
        case 2:
            r = -r;
            break;
        case 3:
            r = -half_pi - r;
            break;
    }

    if (r > quarter_pi) {
        r = half_pi - r;
    } else if (r < -quarter_pi) {
        r = -half_pi - r;
    }

    return sin_kernel(r);
}

} // namespace detail

inline float sin(float x) {
    return detail::sin_impl(x);
}

inline double sin(double x) {
    return detail::sin_impl(x);
}

} // namespace baddy754::trig
