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
#include <limits>

// glibc's <math.h> (pulled in transitively by <cmath> on many platforms)
// defines `issubnormal` as a function-like macro. If this header is
// included after such a header, that macro would silently mangle any
// identifier named `issubnormal` at the call site. We avoid using that
// name at all (see is_subnormal below), and undef it defensively in case
// it was already defined before this point.
#ifdef issubnormal
#undef issubnormal
#endif

#define FFunc   [[maybe_unused]] constexpr float
#define DFunc   [[maybe_unused]] constexpr double

#define U32Func [[maybe_unused]] constexpr uint32_t
#define U64Func [[maybe_unused]] constexpr uint64_t
#define BFunc   [[maybe_unused]] constexpr bool
#define IFunc   [[maybe_unused]] constexpr int

namespace baddy754::basic {

    // ---- layout constants ----
    inline constexpr uint32_t F_SIGN_MASK  = 0x80000000u;
    inline constexpr uint32_t F_EXP_MASK   = 0x7f800000u;
    inline constexpr uint32_t F_MANT_MASK  = 0x007fffffu;
    inline constexpr int      F_EXP_BIAS   = 127;
    inline constexpr int      F_MANT_BITS  = 23;

    inline constexpr uint64_t D_SIGN_MASK  = 0x8000000000000000ull;
    inline constexpr uint64_t D_EXP_MASK   = 0x7ff0000000000000ull;
    inline constexpr uint64_t D_MANT_MASK  = 0x000fffffffffffffull;
    inline constexpr int      D_EXP_BIAS   = 1023;
    inline constexpr int      D_MANT_BITS  = 52;

    // ---- bit <-> float conversion ----
    U32Func bits(float x) noexcept;
    U64Func bits(double x) noexcept;
    FFunc from_bits(uint32_t x) noexcept;
    DFunc from_bits(uint64_t x) noexcept;

    // ---- sign ops ----
    FFunc abs(float x) noexcept;
    DFunc abs(double x) noexcept;

    FFunc neg(float x) noexcept;
    DFunc neg(double x) noexcept;

    FFunc copysign(float mag, float sgn) noexcept;
    DFunc copysign(double mag, double sgn) noexcept;

    // ---- classification ----
    BFunc signbit(float x) noexcept;
    BFunc signbit(double x) noexcept;

    BFunc isnan(float x) noexcept;
    BFunc isnan(double x) noexcept;

    BFunc isinf(float x) noexcept;
    BFunc isinf(double x) noexcept;

    BFunc isfinite(float x) noexcept;
    BFunc isfinite(double x) noexcept;

    BFunc iszero(float x) noexcept;
    BFunc iszero(double x) noexcept;

    // Named is_subnormal (not issubnormal) because glibc's <math.h> defines
    // `issubnormal` as a macro; using that name here would get silently
    // mangled by the preprocessor in any translation unit that includes
    // <cmath>/<math.h> after this header.
    BFunc is_subnormal(float x) noexcept;
    BFunc is_subnormal(double x) noexcept;

    BFunc isnormal(float x) noexcept;
    BFunc isnormal(double x) noexcept;

    // ---- field extraction ----
    U32Func exponent(float x) noexcept;   // biased, raw field
    U32Func mantissa(float x) noexcept;   // raw field, no implicit bit
    U32Func sign(float x) noexcept;       // 0 or 1

    U64Func exponent(double x) noexcept;
    U64Func mantissa(double x) noexcept;
    U64Func sign(double x) noexcept;

    IFunc unbiased_exponent(float x) noexcept;  // exponent() - bias
    IFunc unbiased_exponent(double x) noexcept;

    // ---- construction / mutation ----
    FFunc set_exponent(float x, int exponent) noexcept; // sets *unbiased* exponent, keeps sign+mantissa
    DFunc set_exponent(double x, int exponent) noexcept;

    FFunc set_mantissa(float x, uint32_t mantissa) noexcept;
    DFunc set_mantissa(double x, uint64_t mantissa) noexcept;

    FFunc assemble(bool neg_sign, int unbiased_exp, uint32_t mant) noexcept;
    DFunc assemble(bool neg_sign, int unbiased_exp, uint64_t mant) noexcept;

    // ---- fast power-of-2 scaling (exponent-only, no rounding) ----
    FFunc fast_mul2(float x, int n) noexcept;
    FFunc fast_div2(float x, int n) noexcept;
    DFunc fast_mul2(double x, int n) noexcept;
    DFunc fast_div2(double x, int n) noexcept;

    // ---- normalization (called separately, as requested) ----
    //
    // A subnormal float's *true* scientific-notation form (1.mantissa x 2^e)
    // always needs an exponent below the smallest representable normal
    // exponent -- that's the entire reason subnormals exist as a separate
    // encoding. So "normalizing" a subnormal can never mean "rewrite it as a
    // normal-form bit pattern in the same width"; that representation does
    // not exist. What normalize() actually gives you is the true (sign,
    // unbiased exponent, mantissa-with-implicit-leading-1) decomposition,
    // which is what most consumers of "normalize" actually want (e.g. for
    // manual renormalization in wider arithmetic, or inspection).
    struct NormalizedF {
        bool     neg_sign;
        int      unbiased_exp; // true exponent, may be < -126 for ex-subnormals
        uint32_t mantissa;     // includes the implicit leading 1 (bit 23 set)
    };

    struct NormalizedD {
        bool     neg_sign;
        int      unbiased_exp; // true exponent, may be < -1022 for ex-subnormals
        uint64_t mantissa;     // includes the implicit leading 1 (bit 52 set)
    };

    // Decomposes any finite, non-zero float/double into normalized
    // (sign, exponent, 1.mantissa) form, shifting subnormal mantissas left
    // until the leading bit is set. Undefined result for zero/inf/nan --
    // check iszero()/isfinite() first.
    constexpr NormalizedF normalize(float x) noexcept;
    constexpr NormalizedD normalize(double x) noexcept;

    // Reassembles a NormalizedF/D back into a float/double, if the exponent
    // fits in the target format's normal range; otherwise rounds down into
    // a subnormal encoding (shifting the mantissa right, losing precision,
    // exactly like real underflow), flushing to zero if it underflows even
    // subnormal range entirely.
    FFunc from_normalized(NormalizedF n) noexcept;
    DFunc from_normalized(NormalizedD n) noexcept;

    // Forces a *normal* value down into a subnormal-style encoding at a
    // given (lower) target unbiased exponent, shifting the mantissa right
    // and losing precision the same way real hardware does on underflow.
    // Returns x unchanged if it's not normal or target is not below the
    // subnormal floor; returns signed zero if it underflows completely.
    FFunc denormalize(float x, int target_unbiased_exp) noexcept;
    DFunc denormalize(double x, int target_unbiased_exp) noexcept;

}

// ===================== bit <-> float conversion =====================

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

// ============================ sign ops ================================

FFunc baddy754::basic::neg(float x) noexcept { // F Version
    return from_bits(bits(x) ^ F_SIGN_MASK);
}

DFunc baddy754::basic::neg(double x) noexcept { // D Version
    return from_bits(bits(x) ^ D_SIGN_MASK);
}

FFunc baddy754::basic::abs(float x) noexcept { // F Version
    return from_bits(bits(x) & ~F_SIGN_MASK);
}

DFunc baddy754::basic::abs(double x) noexcept { // D Version
    return from_bits(bits(x) & ~D_SIGN_MASK);
}

FFunc baddy754::basic::copysign(float mag, float sgn) noexcept {
    return from_bits((bits(mag) & ~F_SIGN_MASK) | (bits(sgn) & F_SIGN_MASK));
}

DFunc baddy754::basic::copysign(double mag, double sgn) noexcept {
    return from_bits((bits(mag) & ~D_SIGN_MASK) | (bits(sgn) & D_SIGN_MASK));
}

// ========================== classification =============================

BFunc baddy754::basic::signbit(float x) noexcept {
    return (bits(x) & F_SIGN_MASK) != 0;
}

BFunc baddy754::basic::signbit(double x) noexcept {
    return (bits(x) & D_SIGN_MASK) != 0;
}

BFunc baddy754::basic::isnan(float x) noexcept {
    return (bits(x) & F_EXP_MASK) == F_EXP_MASK &&
           (bits(x) & F_MANT_MASK) != 0;
}

BFunc baddy754::basic::isnan(double x) noexcept {
    return (bits(x) & D_EXP_MASK) == D_EXP_MASK &&
           (bits(x) & D_MANT_MASK) != 0;
}

BFunc baddy754::basic::isinf(float x) noexcept {
    return (bits(x) & F_EXP_MASK) == F_EXP_MASK &&
           (bits(x) & F_MANT_MASK) == 0;
}

BFunc baddy754::basic::isinf(double x) noexcept {
    return (bits(x) & D_EXP_MASK) == D_EXP_MASK &&
           (bits(x) & D_MANT_MASK) == 0;
}

BFunc baddy754::basic::isfinite(float x) noexcept {
    return (bits(x) & F_EXP_MASK) != F_EXP_MASK;
}

BFunc baddy754::basic::isfinite(double x) noexcept {
    return (bits(x) & D_EXP_MASK) != D_EXP_MASK;
}

BFunc baddy754::basic::iszero(float x) noexcept {
    return (bits(x) & ~F_SIGN_MASK) == 0u;
}

BFunc baddy754::basic::iszero(double x) noexcept {
    return (bits(x) & ~D_SIGN_MASK) == 0ull;
}

BFunc baddy754::basic::is_subnormal(float x) noexcept {
    return (bits(x) & F_EXP_MASK) == 0u && (bits(x) & F_MANT_MASK) != 0u;
}

BFunc baddy754::basic::is_subnormal(double x) noexcept {
    return (bits(x) & D_EXP_MASK) == 0ull && (bits(x) & D_MANT_MASK) != 0ull;
}

BFunc baddy754::basic::isnormal(float x) noexcept {
    const uint32_t e = bits(x) & F_EXP_MASK;
    return e != 0u && e != F_EXP_MASK;
}

BFunc baddy754::basic::isnormal(double x) noexcept {
    const uint64_t e = bits(x) & D_EXP_MASK;
    return e != 0ull && e != D_EXP_MASK;
}

// ========================= field extraction =============================

U32Func baddy754::basic::exponent(float x) noexcept {
    return (bits(x) & F_EXP_MASK) >> F_MANT_BITS;
}

U32Func baddy754::basic::mantissa(float x) noexcept {
    return bits(x) & F_MANT_MASK;
}

U32Func baddy754::basic::sign(float x) noexcept {
    return (bits(x) & F_SIGN_MASK) >> 31;
}

U64Func baddy754::basic::exponent(double x) noexcept {
    return (bits(x) & D_EXP_MASK) >> D_MANT_BITS;
}

U64Func baddy754::basic::mantissa(double x) noexcept {
    return bits(x) & D_MANT_MASK;
}

U64Func baddy754::basic::sign(double x) noexcept {
    return (bits(x) & D_SIGN_MASK) >> 63;
}

IFunc baddy754::basic::unbiased_exponent(float x) noexcept {
    return static_cast<int>(exponent(x)) - F_EXP_BIAS;
}

IFunc baddy754::basic::unbiased_exponent(double x) noexcept {
    return static_cast<int>(exponent(x)) - D_EXP_BIAS;
}

// ======================= construction / mutation =========================

FFunc baddy754::basic::set_mantissa(float x, uint32_t mant) noexcept {
    return from_bits((bits(x) & ~F_MANT_MASK) | (mant & F_MANT_MASK));
}

DFunc baddy754::basic::set_mantissa(double x, uint64_t mant) noexcept {
    return from_bits((bits(x) & ~D_MANT_MASK) | (mant & D_MANT_MASK));
}

FFunc baddy754::basic::set_exponent(float x, int exp) noexcept {
    const uint32_t biased = static_cast<uint32_t>(exp + F_EXP_BIAS) & 0xffu;
    return from_bits((bits(x) & ~F_EXP_MASK) | (biased << F_MANT_BITS));
}

DFunc baddy754::basic::set_exponent(double x, int exp) noexcept {
    const uint64_t biased = static_cast<uint64_t>(exp + D_EXP_BIAS) & 0x7ffull;
    return from_bits((bits(x) & ~D_EXP_MASK) | (biased << D_MANT_BITS));
}

FFunc baddy754::basic::assemble(bool neg_sign, int unbiased_exp, uint32_t mant) noexcept {
    const uint32_t s = neg_sign ? F_SIGN_MASK : 0u;
    const uint32_t e = (static_cast<uint32_t>(unbiased_exp + F_EXP_BIAS) & 0xffu) << F_MANT_BITS;
    const uint32_t m = mant & F_MANT_MASK;
    return from_bits(s | e | m);
}

DFunc baddy754::basic::assemble(bool neg_sign, int unbiased_exp, uint64_t mant) noexcept {
    const uint64_t s = neg_sign ? D_SIGN_MASK : 0ull;
    const uint64_t e = (static_cast<uint64_t>(unbiased_exp + D_EXP_BIAS) & 0x7ffull) << D_MANT_BITS;
    const uint64_t m = mant & D_MANT_MASK;
    return from_bits(s | e | m);
}

// =================== fast power-of-2 scaling (exponent-only) ===================
//
// These skip any rounding logic and directly bump the biased exponent field.
// Valid only for finite, normal results; caller is responsible for staying
// within representable exponent range (no overflow/underflow checks), true
// to the "trick" nature of this library.

FFunc baddy754::basic::fast_mul2(float x, int n) noexcept {
    return from_bits(bits(x) + (static_cast<uint32_t>(n) << F_MANT_BITS));
}

FFunc baddy754::basic::fast_div2(float x, int n) noexcept {
    return from_bits(bits(x) - (static_cast<uint32_t>(n) << F_MANT_BITS));
}

DFunc baddy754::basic::fast_mul2(double x, int n) noexcept {
    return from_bits(bits(x) + (static_cast<uint64_t>(n) << D_MANT_BITS));
}

DFunc baddy754::basic::fast_div2(double x, int n) noexcept {
    return from_bits(bits(x) - (static_cast<uint64_t>(n) << D_MANT_BITS));
}

// ============================ normalization ===============================
//
// normalize(): decomposes x into true (sign, unbiased exponent, mantissa
// with explicit leading 1) form. For an already-normal x this just adds
// back the implicit bit. For a subnormal x, the mantissa is shifted left
// until the leading 1 reaches the implicit-bit position, decrementing the
// exponent once per shift -- the result's unbiased_exp will legitimately
// fall below the format's normal floor (that gap below the floor is
// exactly what subnormals exist to cover), so it is returned as a plain
// int rather than forced back into a same-width bit pattern that cannot
// hold it. Must be invoked explicitly -- nothing else in this header calls
// it implicitly.

constexpr baddy754::basic::NormalizedF baddy754::basic::normalize(float x) noexcept {
    const bool neg_sign = signbit(x);

    if (is_subnormal(x)) {
        uint32_t mant = mantissa(x);
        int unbiased_exp = 1 - F_EXP_BIAS;
        while ((mant & (1u << F_MANT_BITS)) == 0u) {
            mant <<= 1;
            --unbiased_exp;
        }
        return NormalizedF{neg_sign, unbiased_exp, mant};
    }

    // Already normal (or zero/inf/nan -- caller is expected to have
    // checked isfinite()/iszero() first for those edge cases).
    return NormalizedF{neg_sign, unbiased_exponent(x), mantissa(x) | (1u << F_MANT_BITS)};
}

constexpr baddy754::basic::NormalizedD baddy754::basic::normalize(double x) noexcept {
    const bool neg_sign = signbit(x);

    if (is_subnormal(x)) {
        uint64_t mant = mantissa(x);
        int unbiased_exp = 1 - D_EXP_BIAS;
        while ((mant & (1ull << D_MANT_BITS)) == 0ull) {
            mant <<= 1;
            --unbiased_exp;
        }
        return NormalizedD{neg_sign, unbiased_exp, mant};
    }

    return NormalizedD{neg_sign, unbiased_exponent(x), mantissa(x) | (1ull << D_MANT_BITS)};
}

// from_normalized(): inverse of normalize(). If the exponent fits the
// format's normal range, reassembles directly. If it's below the normal
// floor, rounds down into subnormal encoding (right-shifting the mantissa,
// same precision loss as real underflow), flushing to signed zero if it
// underflows past representable subnormal range entirely.

FFunc baddy754::basic::from_normalized(NormalizedF n) noexcept {
    constexpr int kMinNormalExp = 1 - F_EXP_BIAS;
    constexpr int kMaxNormalExp = 254 - F_EXP_BIAS;

    if (n.unbiased_exp >= kMinNormalExp && n.unbiased_exp <= kMaxNormalExp) {
        return assemble(n.neg_sign, n.unbiased_exp, n.mantissa & F_MANT_MASK);
    }

    if (n.unbiased_exp < kMinNormalExp) {
        const int shift = kMinNormalExp - n.unbiased_exp;
        if (shift >= 32) {
            return copysign(0.0f, n.neg_sign ? -1.0f : 1.0f);
        }
        const uint32_t shifted_mant = n.mantissa >> shift;
        return from_bits((n.neg_sign ? F_SIGN_MASK : 0u) | (shifted_mant & F_MANT_MASK));
    }

    // Exponent too large: no finite representation -- return signed infinity.
    return copysign(std::numeric_limits<float>::infinity(), n.neg_sign ? -1.0f : 1.0f);
}

DFunc baddy754::basic::from_normalized(NormalizedD n) noexcept {
    constexpr int kMinNormalExp = 1 - D_EXP_BIAS;
    constexpr int kMaxNormalExp = 2046 - D_EXP_BIAS;

    if (n.unbiased_exp >= kMinNormalExp && n.unbiased_exp <= kMaxNormalExp) {
        return assemble(n.neg_sign, n.unbiased_exp, n.mantissa & D_MANT_MASK);
    }

    if (n.unbiased_exp < kMinNormalExp) {
        const int shift = kMinNormalExp - n.unbiased_exp;
        if (shift >= 64) {
            return copysign(0.0, n.neg_sign ? -1.0 : 1.0);
        }
        const uint64_t shifted_mant = n.mantissa >> shift;
        return from_bits((n.neg_sign ? D_SIGN_MASK : uint64_t{0}) | (shifted_mant & D_MANT_MASK));
    }

    return copysign(std::numeric_limits<double>::infinity(), n.neg_sign ? -1.0 : 1.0);
}

// denormalize(): forcibly re-expresses a normal value at a lower, subnormal-
// range unbiased exponent by decomposing it and handing it to
// from_normalized(), which already contains the shift-and-flush-to-zero
// underflow logic. Returns x unchanged for non-normal inputs or when
// target_unbiased_exp is not below the subnormal threshold (1 - bias).

FFunc baddy754::basic::denormalize(float x, int target_unbiased_exp) noexcept {
    constexpr int kSubnormalExp = 1 - F_EXP_BIAS;
    if (!isnormal(x) || target_unbiased_exp >= kSubnormalExp) {
        return x;
    }
    NormalizedF n = normalize(x);
    n.unbiased_exp = target_unbiased_exp;
    return from_normalized(n);
}

DFunc baddy754::basic::denormalize(double x, int target_unbiased_exp) noexcept {
    constexpr int kSubnormalExp = 1 - D_EXP_BIAS;
    if (!isnormal(x) || target_unbiased_exp >= kSubnormalExp) {
        return x;
    }
    NormalizedD n = normalize(x);
    n.unbiased_exp = target_unbiased_exp;
    return from_normalized(n);
}
