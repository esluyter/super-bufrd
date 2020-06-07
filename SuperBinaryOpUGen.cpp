/*
    SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
    http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/


#include "SC_PlugIn.h"

#include <boost/align/is_aligned.hpp>


#ifdef _MSC_VER
// hypotf is c99, but not c++
#    define hypotf _hypotf
#endif

#ifdef NOVA_SIMD
#    include "simd_binary_arithmetic.hpp"
#    include "simd_math.hpp"
#    include "simd_memory.hpp"


#    include "function_attributes.h"
using nova::slope_argument;

#    define NOVA_BINARY_WRAPPER(SCNAME, NOVANAME)                                                                      \
        FLATTEN void SCNAME##_aa_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            nova::NOVANAME##_vec_simd(OUT(0), IN(0), IN(1), inNumSamples);                                             \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_aa_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            nova::NOVANAME##_vec_simd<64>(OUT(0), IN(0), IN(1));                                                       \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ia_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            double xa = DOUBLE_ZIN0(unit, 0);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd(OUT(0), xa, IN(1), inNumSamples);                                                \
            unit->mPrevA = xa;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ia_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            double xa = DOUBLE_ZIN0(unit, 0);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd<64>(OUT(0), xa, IN(1));                                                          \
            unit->mPrevA = xa;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ai_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            double xb = DOUBLE_ZIN0(unit, 1);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd(OUT(0), IN(0), xb, inNumSamples);                                                \
            unit->mPrevB = xb;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ai_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            double xb = DOUBLE_ZIN0(unit, 1);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd<64>(OUT(0), IN(0), xb);                                                          \
            unit->mPrevB = xb;                                                                                         \
        }

#    define NOVA_BINARY_WRAPPER_K(SCNAME, NOVANAME)                                                                    \
        FLATTEN void SCNAME##_aa_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            nova::NOVANAME##_vec_simd(OUT(0), IN(0), IN(1), inNumSamples);                                             \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_aa_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            nova::NOVANAME##_vec_simd<64>(OUT(0), IN(0), IN(1));                                                       \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ia_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            double xa = DOUBLE_ZIN0(unit, 0);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd(OUT(0), xa, IN(1), inNumSamples);                                                \
            unit->mPrevA = xa;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ia_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            double xa = DOUBLE_ZIN0(unit, 0);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd<64>(OUT(0), xa, IN(1));                                                          \
            unit->mPrevA = xa;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ai_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            double xb = DOUBLE_ZIN0(unit, 1);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd(OUT(0), IN(0), xb, inNumSamples);                                                \
            unit->mPrevB = xb;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ai_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            double xb = DOUBLE_ZIN0(unit, 1);                                                                                        \
                                                                                                                       \
            nova::NOVANAME##_vec_simd<64>(OUT(0), IN(0), xb);                                                          \
            unit->mPrevB = xb;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ak_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            float xb = unit->mPrevB;                                                                                   \
            double next_b = DOUBLE_ZIN0(unit, 1);                                                                                    \
                                                                                                                       \
            if (xb == next_b) {                                                                                        \
                nova::NOVANAME##_vec_simd(OUT(0), IN(0), xb, inNumSamples);                                            \
            } else {                                                                                                   \
                float slope = CALCSLOPE(next_b, xb);                                                                   \
                nova::NOVANAME##_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);                     \
                unit->mPrevB = next_b;                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ak_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            float xb = unit->mPrevB;                                                                                   \
            double next_b = DOUBLE_ZIN0(unit, 1);                                                                                    \
                                                                                                                       \
            if (xb == next_b) {                                                                                        \
                nova::NOVANAME##_vec_simd<64>(OUT(0), IN(0), xb);                                                      \
            } else {                                                                                                   \
                float slope = CALCSLOPE(next_b, xb);                                                                   \
                nova::NOVANAME##_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);                     \
                unit->mPrevB = next_b;                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        FLATTEN void SCNAME##_ka_nova(SuperBinaryOpUGen* unit, int inNumSamples) {                                          \
            float xa = unit->mPrevA;                                                                                   \
            double next_a = DOUBLE_ZIN0(unit, 0);                                                                                    \
                                                                                                                       \
            if (xa == next_a) {                                                                                        \
                nova::NOVANAME##_vec_simd(OUT(0), xa, IN(1), inNumSamples);                                            \
            } else {                                                                                                   \
                float slope = CALCSLOPE(next_a, xa);                                                                   \
                nova::NOVANAME##_vec_simd(OUT(0), slope_argument(xa, slope), IN(1), inNumSamples);                     \
                unit->mPrevA = next_a;                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        FLATTEN void SCNAME##_ka_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {                                       \
            float xa = unit->mPrevA;                                                                                   \
            double next_a = DOUBLE_ZIN0(unit, 0);                                                                                    \
                                                                                                                       \
            if (xa == next_a) {                                                                                        \
                nova::NOVANAME##_vec_simd<64>(OUT(0), xa, IN(1));                                                      \
            } else {                                                                                                   \
                float slope = CALCSLOPE(next_a, xa);                                                                   \
                nova::NOVANAME##_vec_simd(OUT(0), slope_argument(xa, slope), IN(1), inNumSamples);                     \
                unit->mPrevA = next_a;                                                                                 \
            }                                                                                                          \
        }


#    define DEFINE_TEMPLATE_FUNCTOR(NAME)                                                                              \
        struct NAME##_functor {                                                                                        \
            template <typename FloatType> inline FloatType operator()(FloatType a, FloatType b) const {                \
                return NAME(a, b);                                                                                     \
            }                                                                                                          \
                                                                                                                       \
            template <typename FloatType>                                                                              \
            inline nova::vec<FloatType> operator()(nova::vec<FloatType> a, nova::vec<FloatType> b) const {             \
                return NAME(a, b);                                                                                     \
            }                                                                                                          \
        };

DEFINE_TEMPLATE_FUNCTOR(sc_ring1)
DEFINE_TEMPLATE_FUNCTOR(sc_ring2)
DEFINE_TEMPLATE_FUNCTOR(sc_ring3)
DEFINE_TEMPLATE_FUNCTOR(sc_ring4)

DEFINE_TEMPLATE_FUNCTOR(sc_difsqr)
DEFINE_TEMPLATE_FUNCTOR(sc_sumsqr)
DEFINE_TEMPLATE_FUNCTOR(sc_sqrsum)
DEFINE_TEMPLATE_FUNCTOR(sc_sqrdif)

namespace nova {
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_ring1, sc_ring1_functor)
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_ring2, sc_ring2_functor)
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_ring3, sc_ring3_functor)
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_ring4, sc_ring4_functor)

NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_difsqr, sc_difsqr_functor)
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_sumsqr, sc_sumsqr_functor)
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_sqrsum, sc_sqrsum_functor)
NOVA_SIMD_DEFINE_BINARY_WRAPPER(sc_sqrdif, sc_sqrdif_functor)
}

#endif

using namespace std; // for math functions

static InterfaceTable* ft;

//////////////////////////////////////////////////////////////////////////////////////////////////


/* special binary math operators */
enum {
    opAdd,
    opSub,
    opMul,
    opIDiv,
    opFDiv,
    opMod,
    opEQ,
    opNE,
    opLT,
    opGT,
    opLE,
    opGE,
    // opIdentical,
    // opNotIdentical,

    opMin,
    opMax,
    opBitAnd,
    opBitOr,
    opBitXor,
    opLCM,
    opGCD,
    opRound,
    opRoundUp,
    opTrunc,
    opAtan2,
    opHypot,
    opHypotx,
    opPow,
    opShiftLeft, //
    opShiftRight, //
    opUnsignedShift, //
    opFill, //
    opRing1, // a * (b + 1) == a * b + a
    opRing2, // a * b + a + b
    opRing3, // a*a*b
    opRing4, // a*a*b - a*b*b
    opDifSqr, // a*a - b*b
    opSumSqr, // a*a + b*b
    opSqrSum, // (a + b)^2
    opSqrDif, // (a - b)^2
    opAbsDif, // |a - b|
    opThresh,
    opAMClip,
    opScaleNeg,
    opClip2,
    opExcess,
    opFold2,
    opWrap2,
    opFirstArg,
    opRandRange,
    opExpRandRange,

    opNumBinarySelectors
};


inline double sc_andt(double a, double b) { return int(a) & int(b); }

inline double sc_ort(double a, double b) { return int(a) | int(b); }

inline double sc_xort(double a, double b) { return int(a) ^ int(b); }

inline double sc_rst(double a, double b) { return int(a) >> int(b); }

inline double sc_lst(double a, double b) { return int(a) << int(b); }

inline float sc_gcd(double u, double v) { return (double)sc_gcd((long)std::trunc(u), (long)std::trunc(v)); }

inline float sc_lcm(double u, double v) { return (double)sc_lcm((long)std::trunc(u), (long)std::trunc(v)); }


struct SuperBinaryOpUGen : public Unit {
    double mPrevA, mPrevB;
};

typedef void (*BinaryOpFunc)(SuperBinaryOpUGen* unit, int inNumSamples);

extern "C" {

void SuperBinaryOpUGen_Ctor(SuperBinaryOpUGen* unit);

// void zero_d(SuperBinaryOpUGen *unit, int inNumSamples);
void zero_1(SuperBinaryOpUGen* unit, int inNumSamples);
void zero_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void firstarg_d(SuperBinaryOpUGen* unit, int inNumSamples);
void firstarg_1(SuperBinaryOpUGen* unit, int inNumSamples);
void firstarg_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void secondarg_d(SuperBinaryOpUGen* unit, int inNumSamples);
void secondarg_1(SuperBinaryOpUGen* unit, int inNumSamples);
void secondarg_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void add_d(SuperBinaryOpUGen* unit, int inNumSamples);
void add_1(SuperBinaryOpUGen* unit, int inNumSamples);
void add_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void add_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void add_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void add_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void add_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_d(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_1(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void sub_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_d(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_1(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void mul_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void div_d(SuperBinaryOpUGen* unit, int inNumSamples);
void div_1(SuperBinaryOpUGen* unit, int inNumSamples);
void div_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void div_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void div_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void div_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void div_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_d(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_1(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void mod_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void max_d(SuperBinaryOpUGen* unit, int inNumSamples);
void max_1(SuperBinaryOpUGen* unit, int inNumSamples);
void max_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void max_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void max_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void max_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void max_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void min_d(SuperBinaryOpUGen* unit, int inNumSamples);
void min_1(SuperBinaryOpUGen* unit, int inNumSamples);
void min_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void min_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void min_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void min_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void min_ia(SuperBinaryOpUGen* unit, int inNumSamples);

void and_d(SuperBinaryOpUGen* unit, int inNumSamples);
void and_1(SuperBinaryOpUGen* unit, int inNumSamples);
void and_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void and_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void and_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void and_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void and_ia(SuperBinaryOpUGen* unit, int inNumSamples);

void or_d(SuperBinaryOpUGen* unit, int inNumSamples);
void or_1(SuperBinaryOpUGen* unit, int inNumSamples);
void or_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void or_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void or_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void or_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void or_ia(SuperBinaryOpUGen* unit, int inNumSamples);

void xor_d(SuperBinaryOpUGen* unit, int inNumSamples);
void xor_1(SuperBinaryOpUGen* unit, int inNumSamples);
void xor_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void xor_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void xor_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void xor_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void xor_ia(SuperBinaryOpUGen* unit, int inNumSamples);

void amclip_d(SuperBinaryOpUGen* unit, int inNumSamples);
void amclip_1(SuperBinaryOpUGen* unit, int inNumSamples);
void amclip_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void amclip_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void amclip_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void amclip_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void amclip_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_d(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_1(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void scaleneg_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_d(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_1(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void pow_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_d(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_1(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void ring1_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_d(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_1(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void ring2_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_d(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_1(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void ring3_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_d(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_1(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void ring4_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_d(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_1(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void thresh_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_d(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_1(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void clip2_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_d(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_1(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void fold2_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_d(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_1(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void wrap2_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_d(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_1(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void excess_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_d(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_1(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void rrand_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_d(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_1(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void exprand_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_d(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_1(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void lt_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void le_d(SuperBinaryOpUGen* unit, int inNumSamples);
void le_1(SuperBinaryOpUGen* unit, int inNumSamples);
void le_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void le_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void le_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void le_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void le_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_d(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_1(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void lcm_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_d(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_1(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void gcd_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_d(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_1(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void gt_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_d(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_1(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void ge_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_d(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_1(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void eq_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_d(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_1(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void neq_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_d(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_1(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void sumsqr_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_d(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_1(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void difsqr_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_d(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_1(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrsum_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_d(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_1(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void sqrdif_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_d(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_1(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void absdif_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void round_d(SuperBinaryOpUGen* unit, int inNumSamples);
void round_1(SuperBinaryOpUGen* unit, int inNumSamples);
void round_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void round_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void round_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void round_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void round_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_d(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_1(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void roundUp_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_d(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_1(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void trunc_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_d(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_1(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void atan2_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_d(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_1(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void hypot_ia(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_d(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_1(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_aa(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_ak(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_ka(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_ai(SuperBinaryOpUGen* unit, int inNumSamples);
void hypotx_ia(SuperBinaryOpUGen* unit, int inNumSamples);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////


static inline double DOUBLE_ZIN0(SuperBinaryOpUGen* unit, int i) {return (double)IN0(i*2) + (double)IN0(i*2+1);}

static inline void DOUBLE_ZOUT0(SuperBinaryOpUGen* unit, double val){
  float msd = (float) val;
  OUT0(0) = msd;
  OUT0(1) = (float) (val-msd);
}
static inline void DOUBLE_ZXPOUT(SuperBinaryOpUGen* unit, float* out0, float* out1, double val){
  float msd = (float) val;
  ZXP(out0) = msd;
  ZXP(out1) = (float) (val-msd);
}

static inline void DOUBLE_ZCOPY(SuperBinaryOpUGen *unit, int inNumSamples, int i){
  float *msd=IN(i*2),*lsd=IN(i*2+1), *out0=OUT(0), *out1=OUT(1);

  do{
    double val = static_cast<double>(*(msd++)) + static_cast<double>(*(lsd++));
    float msd_out = (float) val;
    *(out0++) = msd_out;
    *(out1++) = (float) (val-msd_out);
  }while(--inNumSamples);
}


template<typename Action>
static void DOUBLE_LOOP1(SuperBinaryOpUGen* unit, int inNumSamples, Action action){
  float *a=IN(0),*b=IN(1),*c=IN(2),*d=IN(3), *out0=OUT(0), *out1=OUT(1);
  do{
    double val = action(
      static_cast<double>(*(a++))+static_cast<double>(*(b++)),
      static_cast<double>(*(c++))+static_cast<double>(*(d++)));
    float msd = (float) val;
    *(out0++) = msd;
    *(out1++) = (float) (val-msd);
  }while(--inNumSamples);
}

// loops one ar input (ar_i), returns ar value in action
template<typename Action>
static void DOUBLE_LOOP1_k(SuperBinaryOpUGen* unit, int inNumSamples, int ar_i, Action action){
  float *msd=IN(ar_i*2),*lsd=IN(ar_i*2+1), *out0=OUT(0), *out1=OUT(1);
  do{
    double val = action(
      static_cast<double>(*(msd++))+static_cast<double>(*(lsd++))
    );
    float msd_out = (float) val;
    *(out0++) = msd_out;
    *(out1++) = (float) (val-msd_out);
  }while(--inNumSamples);
}

template<typename Action>
static void DOUBLE_LOOP1_ia(SuperBinaryOpUGen* unit, int inNumSamples, Action action){
  double xa = DOUBLE_ZIN0(unit, 0);
  DOUBLE_LOOP1_k(unit, inNumSamples, 1, [xa,action](double b){return action(xa,b);});
  unit->mPrevA = xa;
}
template<typename Action>
static void DOUBLE_LOOP1_ai(SuperBinaryOpUGen* unit, int inNumSamples, Action action){
  double xb = DOUBLE_ZIN0(unit, 1);
  DOUBLE_LOOP1_k(unit, inNumSamples, 0, [xb,action](double a){return action(a,xb);});
  unit->mPrevB = xb;
}

template<typename Action>
inline static void slope_ak(SuperBinaryOpUGen* unit, int inNumSamples, double kr_prev, double kr_next, Action action){
  float slope = CALCSLOPE(kr_next, kr_prev);
  DOUBLE_LOOP1(unit, inNumSamples,  [&kr_prev,slope, action](double a, double b){
    double res = action(a,kr_prev); kr_prev += slope; return res;
  });
  unit->mPrevB = kr_prev;
}

template<typename Action>
inline static void slope_ka(SuperBinaryOpUGen* unit, int inNumSamples, double kr_prev, double kr_next, Action action){
  float slope = CALCSLOPE(kr_next, kr_prev);
  DOUBLE_LOOP1(unit, inNumSamples,  [&kr_prev,slope, action](double a, double b){
    double res = action(kr_prev,b); kr_prev += slope; return res;
  }); \
  unit->mPrevA = kr_prev;
}

template<typename Action>
static inline void DOUBLE_LOOP1_ak_slope(SuperBinaryOpUGen* unit, int inNumSamples, Action action){
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev, action](double a){return action(a,kr_prev);});
  } else {
      slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}

template<typename Action>
static inline void DOUBLE_LOOP1_ka_slope(SuperBinaryOpUGen* unit, int inNumSamples, Action action){
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev, action](double b){return action(kr_prev,b);});
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}

#define DOUBLE_ZCLEAR(inNumSamples) \
  ZClear(inNumSamples, ZOUT(0));\
  ZClear(inNumSamples, ZOUT(1));\


static bool ChooseOperatorFunc(SuperBinaryOpUGen* unit);

void SuperBinaryOpUGen_Ctor(SuperBinaryOpUGen* unit) {
    unit->mPrevA = DOUBLE_ZIN0(unit, 0);
    unit->mPrevB = DOUBLE_ZIN0(unit, 1);
    bool initialized = ChooseOperatorFunc(unit);

    if (unit->mCalcRate == calc_DemandRate) {
        DOUBLE_ZOUT0(unit, 0.f);
    } else {
        if (!initialized)
            (unit->mCalcFunc)(unit, 1);
    }
}

// TODO: ALL DEMAND-RATE FUNCTIONS ARE UNDONE

/*
void zero_d(SuperBinaryOpUGen *unit, int inNumSamples)
{
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : 0.f;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}
*/

void firstarg_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void secondarg_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void add_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a + b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void sub_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a - b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void mul_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void div_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a / b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void idiv_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : floor(a / b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void mod_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_mod(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void max_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_max(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void min_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_min(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void and_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_andt(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void or_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_ort(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void xor_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_xort(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void rightShift_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_rst(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void leftShift_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_lst(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void lcm_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_lcm(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void gcd_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_gcd(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void amclip_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_amclip(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void scaleneg_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_scaleneg(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void pow_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a < 0.f ? -pow(-a, b) : pow(a, b));
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void ring1_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * b + a;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void ring2_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * b + a + b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void ring3_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * a * b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void ring4_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * a * b - a * b * b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void thresh_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_thresh(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void clip2_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_clip2(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void excess_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_excess(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void lt_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a < b ? 1.f : 0.f);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void gt_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a > b ? 1.f : 0.f);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void le_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a <= b ? 1.f : 0.f);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void ge_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a >= b ? 1.f : 0.f);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void eq_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a == b ? 1.f : 0.f);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void neq_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (a != b ? 1.f : 0.f);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}


void sumsqr_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * a + b * b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void difsqr_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : a * a - b * b;
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void sqrsum_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        float z;
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (z = a + b, z * z);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void sqrdif_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        float z;
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : (z = a - b, z * z);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void absdif_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : fabs(a - b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void round_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_round(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void roundUp_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_roundUp(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void trunc_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_trunc(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void fold2_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_fold2(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void wrap2_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_wrap2(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void atan2_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : atan2(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void hypot_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : hypot(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void hypotx_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float a = DEMANDINPUT_A(0, inNumSamples);
        float b = DEMANDINPUT_A(1, inNumSamples);
        OUT0(0) = sc_isnan(a) || sc_isnan(b) ? NAN : sc_hypotx(a, b);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void rrand_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float xa = DEMANDINPUT_A(0, inNumSamples);
        float xb = DEMANDINPUT_A(1, inNumSamples);
        RGen& rgen = *unit->mParent->mRGen;
        OUT0(0) = sc_isnan(xa) || sc_isnan(xb)
            ? NAN
            : xb > xa ? xa + rgen.frand() * (xb - xa) : (xb + rgen.frand() * (xa - xb));
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}

void exprand_d(SuperBinaryOpUGen* unit, int inNumSamples) {
    if (inNumSamples) {
        float xa = DEMANDINPUT_A(0, inNumSamples);
        float xb = DEMANDINPUT_A(1, inNumSamples);
        RGen& rgen = *unit->mParent->mRGen;
        OUT0(0) = sc_isnan(xa) || sc_isnan(xb) ? NAN : xb > xa ? rgen.exprandrng(xa, xb) : rgen.exprandrng(xb, xa);
    } else {
        RESETINPUT(0);
        RESETINPUT(1);
    }
}


void zero_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, 0.f); }

void firstarg_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, ZIN0(0) ); ZOUT0(1) = ZIN0(1); }

void secondarg_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, ZIN0(2) ); ZOUT0(1) = ZIN0(3); }

void add_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, DOUBLE_ZIN0(unit, 0) + DOUBLE_ZIN0(unit, 1)); }

void sub_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, DOUBLE_ZIN0(unit, 0) - DOUBLE_ZIN0(unit, 1)); }

void mul_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, DOUBLE_ZIN0(unit, 0) * DOUBLE_ZIN0(unit, 1)); }

void div_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, DOUBLE_ZIN0(unit, 0) / DOUBLE_ZIN0(unit, 1)); }

void idiv_1(SuperBinaryOpUGen* unit, int inNumSamples) { DOUBLE_ZOUT0(unit, floor(DOUBLE_ZIN0(unit, 0) / DOUBLE_ZIN0(unit, 1))); }

void mod_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_mod(xa, xb));
}

void max_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_max(xa, xb));
}

void min_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_min(xa, xb));
}

void and_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_andt(xa, xb) );
}

void or_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_ort(xa, xb) );
}

void xor_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_xort(xa, xb) );
}

void rightShift_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_rst(xa, xb) );
}

void leftShift_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_lst(xa, xb) );
}

void lcm_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_lcm(xa, xb) );
}

void gcd_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_gcd(xa, xb) );
}

void amclip_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_amclip(xa, xb) );
}

void scaleneg_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa >= 0.f ? xa : xa * xb );
}

void pow_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb) );
}


void ring1_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa * xb + xa );
}

void ring2_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa * xb + xa + xb );
}

void ring3_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa * xa * xb );
}

void ring4_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa * xa * xb - xa * xb * xb );
}

void thresh_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa < xb ? 0.f : xa );
}

void clip2_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa > xb ? xb : (xa < -xb ? -xb : xa) );
}

void excess_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa > xb ? xa - xb : (xa < -xb ? xa + xb : 0.f) );
}

void lt_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa < xb ? 1.f : 0.f );
}

void le_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa <= xb ? 1.f : 0.f );
}

void gt_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa > xb ? 1.f : 0.f );
}

void ge_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa >= xb ? 1.f : 0.f );
}

void eq_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa == xb ? 1.f : 0.f );
}

void neq_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa != xb ? 1.f : 0.f );
}


void sumsqr_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa * xa + xb * xb );
}

void difsqr_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, xa * xa - xb * xb );
}

void sqrsum_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    float sum = xa + xb;
    DOUBLE_ZOUT0(unit, sum * sum );
}

void sqrdif_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    float dif = xa - xb;
    DOUBLE_ZOUT0(unit, dif * dif );
}

void absdif_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, fabs(xa - xb) );
}

void round_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_round(xa, xb) );
}

void roundUp_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_roundUp(xa, xb) );
}

void trunc_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_trunc(xa, xb) );
}

void fold2_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_fold(xa, -xb, xb) );
}

void wrap2_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_wrap(xa, -xb, xb) );
}

void atan2_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, atan2(xa, xb) );
}

void hypot_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, hypot(xa, xb) );
}

void hypotx_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    DOUBLE_ZOUT0(unit, sc_hypotx(xa, xb) );
}

void rrand_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    RGen& rgen = *unit->mParent->mRGen;
    DOUBLE_ZOUT0(unit, xb > xa ? xa + rgen.frand() * (xb - xa) : (xb + rgen.frand() * (xa - xb)) );
}

void exprand_1(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    double xb = DOUBLE_ZIN0(unit, 1);
    RGen& rgen = *unit->mParent->mRGen;
    DOUBLE_ZOUT0(unit, xb > xa ? rgen.exprandrng(xa, xb) : rgen.exprandrng(xb, xa) );
}

void zero_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    float* out0 = ZOUT(0);
    float* out1 = ZOUT(1);

    ZClear(inNumSamples, out0);
    ZClear(inNumSamples, out1);
}

void firstarg_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_ZCOPY(unit, inNumSamples, 0);
}

#ifdef NOVA_SIMD
FLATTEN void firstarg_aa_nova(SuperBinaryOpUGen* unit, int inNumSamples) { nova::copyvec_simd(OUT(0), IN(0), inNumSamples); }
#endif

void secondarg_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_ZCOPY(unit, inNumSamples, 1);
}

#ifdef NOVA_SIMD
FLATTEN void secondarg_aa_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    nova::copyvec_simd(OUT(0), IN(1), inNumSamples);
}
#endif

void add_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a+b;});
}

void add_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return a+b;};
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev, action](double a){return action(a,kr_prev);});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}


void add_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return a+b;};
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev, action](double b){return action(kr_prev, b);});
      }
  } else {
    slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}

void add_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a+b;});
}

void add_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return a+b;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER(add, plus)

FLATTEN void add_ak_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::copyvec_simd(OUT(0), IN(0), inNumSamples);
        else
            nova::plus_vec_simd(OUT(0), IN(0), xb, inNumSamples);
    } else {
        float slope = CALCSLOPE(next_b, xb);
        nova::plus_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
        unit->mPrevB = next_b;
    }
}

FLATTEN void add_ak_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::copyvec_simd<64>(OUT(0), IN(0));
        else
            nova::plus_vec_simd<64>(OUT(0), IN(0), xb);
    } else {
        float slope = CALCSLOPE(next_b, xb);
        nova::plus_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
        unit->mPrevB = next_b;
    }
}


FLATTEN void add_ka_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        if (xa == 0.f)
            nova::copyvec_simd(OUT(0), IN(1), inNumSamples);
        else
            nova::plus_vec_simd(OUT(0), IN(1), xa, inNumSamples);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        nova::plus_vec_simd(OUT(0), IN(1), slope_argument(xa, slope), inNumSamples);
        unit->mPrevA = next_a;
    }
}

FLATTEN void add_ka_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        if (xa == 0.f)
            nova::copyvec_simd<64>(OUT(0), IN(1));
        else
            nova::plus_vec_simd<64>(OUT(0), IN(1), xa);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        nova::plus_vec_simd(OUT(0), IN(1), slope_argument(xa, slope), inNumSamples);
        unit->mPrevA = next_a;
    }
}

#endif


/////////////////////////

void sub_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a-b;});
}
void sub_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return a-b;};
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev, action](double a){return action(a,kr_prev);});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void sub_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return a-b;};
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples,  1, [kr_prev, action](double b){return action(kr_prev, b);});
      }
  } else {
      slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void sub_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a-b;});
}

void sub_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return a-b;});
}



#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER(sub, minus)

FLATTEN void sub_ak_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::copyvec_simd(OUT(0), IN(0), inNumSamples);
        else
            nova::minus_vec_simd(OUT(0), IN(0), xb, inNumSamples);
    } else {
        float slope = CALCSLOPE(next_b, xb);
        nova::minus_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
        unit->mPrevB = next_b;
    }
}

FLATTEN void sub_ak_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::copyvec_aa_simd<64>(OUT(0), IN(0));
        else
            nova::minus_vec_simd<64>(OUT(0), IN(0), xb);
    } else {
        float slope = CALCSLOPE(next_b, xb);
        nova::minus_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
        unit->mPrevB = next_b;
    }
}

FLATTEN void sub_ka_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        nova::minus_vec_simd(OUT(0), xa, IN(1), inNumSamples);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        nova::minus_vec_simd(OUT(0), slope_argument(xa, slope), IN(1), inNumSamples);
        unit->mPrevA = next_a;
    }
}

FLATTEN void sub_ka_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        nova::minus_vec_simd<64>(OUT(0), xa, IN(1));
    } else {
        float slope = CALCSLOPE(next_a, xa);
        nova::minus_vec_simd(OUT(0), slope_argument(xa, slope), IN(1), inNumSamples);
        unit->mPrevA = next_a;
    }
}

#endif


void mul_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*b;});
}
void mul_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return a*b;};
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCLEAR(inNumSamples);
      }else if (kr_prev == 1) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev, action](double ar){return action(ar,kr_prev);});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void mul_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return a*b;};
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCLEAR(inNumSamples);
      } else if (kr_prev == 1) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      }else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev, action](double b){return action(kr_prev, b);});
      }
  } else {
      slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void mul_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a*b;});
}
void mul_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return a*b;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER(mul, times)

FLATTEN void mul_ka_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        if (xa == 0.f)
            nova::zerovec_simd(OUT(0), inNumSamples);
        else if (xa == 1.f)
            nova::copyvec_simd(OUT(0), IN(1), inNumSamples);
        else
            nova::times_vec_simd(OUT(0), IN(1), xa, inNumSamples);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        unit->mPrevA = next_a;

        nova::times_vec_simd(OUT(0), IN(1), slope_argument(xa, slope), inNumSamples);
    }
}

FLATTEN void mul_ka_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        if (xa == 0.f)
            nova::zerovec_simd<64>(OUT(0));
        else if (xa == 1.f)
            nova::copyvec_simd<64>(OUT(0), IN(1));
        else
            nova::times_vec_simd<64>(OUT(0), IN(1), xa);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        unit->mPrevA = next_a;

        nova::times_vec_simd(OUT(0), IN(1), slope_argument(xa, slope), inNumSamples);
    }
}

FLATTEN void mul_ak_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::zerovec_simd(OUT(0), inNumSamples);
        else if (xb == 1.f)
            nova::copyvec_simd(OUT(0), IN(0), inNumSamples);
        else
            nova::times_vec_simd(OUT(0), IN(0), xb, inNumSamples);
    } else {
        float slope = CALCSLOPE(next_b, xb);
        unit->mPrevB = next_b;

        nova::times_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
    }
}

FLATTEN void mul_ak_nova_64(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::zerovec_simd<64>(OUT(0));
        else if (xb == 1.f)
            nova::copyvec_simd<64>(OUT(0), IN(0));
        else
            nova::times_vec_simd<64>(OUT(0), IN(0), xb);
    } else {
        float slope = CALCSLOPE(next_b, xb);
        unit->mPrevB = next_b;

        nova::times_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
    }
}

#endif

void div_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a/b;});
}
void div_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCLEAR(inNumSamples);
      }else if (kr_prev == 1) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          float recip = 1.f / kr_prev;
          DOUBLE_LOOP1_k(unit, inNumSamples,  0, [recip](double a){return a*recip;});
      }
  } else {
    auto action = [](double a, double b){return a/b;};
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void div_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action =   [](double a, double b){return a/b;};
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCLEAR(inNumSamples);
      } else if (kr_prev == 1) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      }else {
          DOUBLE_LOOP1_k(unit, inNumSamples,  1, [kr_prev, action](double b){return action(kr_prev, b);});
      }
  } else {
    slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void div_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a/b;});
}
void div_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xb = DOUBLE_ZIN0(unit,1);
    float rxb = 1.f / xb;
    DOUBLE_LOOP1_k(unit, inNumSamples, 0, [rxb](double a){return a*rxb;});
    unit->mPrevB = xb;
}


#ifdef NOVA_SIMD
FLATTEN void div_aa_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    nova::over_vec_simd(OUT(0), IN(0), IN(1), inNumSamples);
}

FLATTEN void div_ia_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);

    nova::over_vec_simd(OUT(0), xa, IN(1), inNumSamples);
    unit->mPrevA = xa;
}

FLATTEN void div_ai_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xb = DOUBLE_ZIN0(unit, 1);

    nova::times_vec_simd(OUT(0), IN(0), sc_reciprocal(xb), inNumSamples);
    unit->mPrevB = xb;
}

FLATTEN void div_ak_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b) {
        if (xb == 0.f)
            nova::zerovec_simd(OUT(0), inNumSamples);
        else if (xb == 1.f)
            nova::copyvec_simd(OUT(0), IN(0), inNumSamples);
        else {
            float recip = 1.f / xb;
            nova::times_vec_simd(OUT(0), IN(0), recip, inNumSamples);
        }
    } else {
        float slope = CALCSLOPE(next_b, xb);
        nova::over_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
        unit->mPrevB = next_b;
    }
}

FLATTEN void div_ka_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        if (xa == 0.f)
            nova::zerovec_simd(OUT(0), inNumSamples);
        else
            nova::over_vec_simd(OUT(0), xa, IN(1), inNumSamples);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        nova::over_vec_simd(OUT(0), slope_argument(xa, slope), IN(1), inNumSamples);
        unit->mPrevA = next_a;
    }
}

#endif

void idiv_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return floor(a/b);});
}

void idiv_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return floor(a/b);};
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples,  0, [kr_prev, action](double ar){return action(ar,kr_prev);});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}


void idiv_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return floor(a/b);};
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples,  1, [kr_prev, action](double b){return action(kr_prev,b);});
      }
  } else {
    slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}

void idiv_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return floor(a/b);});
}

void idiv_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return floor(a/b);});
}


void mod_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_mod(a,b);});
}
void mod_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return sc_mod(a,b);};
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples,  0, [kr_prev, action](double ar){return action(ar,kr_prev);});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void mod_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  auto action = [](double a, double b){return sc_mod(a,b);};
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples,  1, [kr_prev, action](double b){return action(kr_prev, b);});
      }
  } else {
    slope_ka(unit, inNumSamples, kr_prev, kr_next, action);
  }
}
void mod_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_mod(a,b);});
}
void mod_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_mod(a,b);});
}


void max_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_max(a,b);});
}
void max_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples, [](double a, double b){return sc_max(a,b);});
}
void max_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples, [](double a, double b){return sc_max(a,b);});
}
void max_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_max(a,b);});
}
void max_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_max(a,b);});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(max, max)
#endif


void min_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_min(a,b);});
}
void min_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_min(a,b);}
  );
}
void min_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_min(a,b);}
  );
}
void min_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_min(a,b);});
}
void min_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_min(a,b);});
}


#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(min, min)
#endif


void and_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_andt(a,b);});
}

void and_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_andt(a,b);}
  );
}

void and_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_andt(a,b);}
  );
}

void and_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_andt(a,b);});
}


void and_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_andt(a,b);});
}


void or_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_ort(a,b);});
}

void or_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_ort(a,b);}
  );
}

void or_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_ort(a,b);}
  );
}

void or_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_ort(a,b);});
}


void or_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_ort(a,b);});
}


void xor_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_xort(a,b);});
}

void xor_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_xort(a,b);}
  );
}

void xor_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_xort(a,b);}
  );
}

void xor_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_ort(a,b);});
}


void xor_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_ort(a,b);});
}


void rightShift_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_rst(a,b);});
}

void rightShift_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_rst(a,b);}
  );
}

void rightShift_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_rst(a,b);}
  );
}

void rightShift_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_rst(a,b);});
}


void rightShift_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_rst(a,b);});
}


void leftShift_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_lst(a,b);});
}

void leftShift_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_lst(a,b);}
  );
}

void leftShift_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_lst(a,b);}
  );
}

void leftShift_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_lst(a,b);});
}


void leftShift_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_lst(a,b);});
}


void lcm_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_lcm(a,b);});
}

void lcm_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_lcm(a,b);}
  );
}

void lcm_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_lcm(a,b);}
  );
}

void lcm_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_lcm(a,b);});
}


void lcm_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_lcm(a,b);});
}


void gcd_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_gcd(a,b);});
}

void gcd_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_gcd(a,b);}
  );
}

void gcd_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_gcd(a,b);}
  );
}

void gcd_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_gcd(a,b);});
}


void gcd_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_gcd(a,b);});
}


void amclip_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_amclip(a,b);});
}

void amclip_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return sc_amclip(a,b);}
  );
}

void amclip_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return sc_amclip(a,b);}
  );
}

void amclip_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return sc_amclip(a,b);});
}


void amclip_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return sc_amclip(a,b);});
}


void scaleneg_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a >= 0.f ? a : a * b;});
}

void scaleneg_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a >= 0.f ? a : a * b;}
  );
}

void scaleneg_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev >= 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return kr_prev * b;});
      }
  } else {
    slope_ka(unit, inNumSamples, kr_prev, kr_next,
       [](double a, double b){return a >= 0.f ? a : a * b;}
    );
  }
}

void scaleneg_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a >= 0.f ? a : a * b;});
}


void scaleneg_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return a >= 0.f ? a : a * b;});
}


void pow_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a >= 0.f ? pow(a,b) : -pow(-a,b);});
}


void pow_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a >= 0.f ? pow(a,b) : -pow(-a,b);}
  );
}

void pow_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev >= 0.f) {
        DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return pow(kr_prev,b);});
      } else {
        DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return -pow(-kr_prev,b);});
      }
  } else {
    slope_ka(unit, inNumSamples, kr_prev, kr_next,
      [](double a, double b){return a >= 0.f ? pow(a,b) : -pow(-a,b);}
    );
  }
}

void pow_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1_ia(unit, inNumSamples,
      [](double a, double b){return a >= 0.f ? pow(a,b) : -pow(-a,b);}
    );
}


void pow_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a >= 0.f ? pow(a,b) : -pow(-a,b);}
  );
}

#ifdef NOVA_SIMD
FLATTEN void pow_aa_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    nova::spow_vec_simd(OUT(0), IN(0), IN(1), inNumSamples);
}

FLATTEN void pow_ak_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xb = unit->mPrevB;
    double next_b = DOUBLE_ZIN0(unit, 1);

    if (xb == next_b)
        nova::spow_vec_simd(OUT(0), IN(0), xb, inNumSamples);
    else {
        float slope = CALCSLOPE(next_b, xb);
        nova::spow_vec_simd(OUT(0), IN(0), slope_argument(xb, slope), inNumSamples);
        unit->mPrevB = next_b;
    }
}

FLATTEN void pow_ka_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    float xa = unit->mPrevA;
    double next_a = DOUBLE_ZIN0(unit, 0);

    if (xa == next_a) {
        if (xa >= 0.f)
            nova::pow_vec_simd(OUT(0), xa, IN(1), inNumSamples);
        else
            nova::spow_vec_simd(OUT(0), xa, IN(1), inNumSamples);
    } else {
        float slope = CALCSLOPE(next_a, xa);
        nova::spow_vec_simd(OUT(0), slope_argument(xa, slope), IN(1), inNumSamples);
        unit->mPrevA = next_a;
    }
}


FLATTEN void pow_ia_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xa = DOUBLE_ZIN0(unit, 0);
    if (xa > 0.f)
        nova::pow_vec_simd(OUT(0), xa, IN(1), inNumSamples);
    else
        nova::spow_vec_simd(OUT(0), xa, IN(1), inNumSamples);
    unit->mPrevA = xa;
}


FLATTEN void pow_ai_nova(SuperBinaryOpUGen* unit, int inNumSamples) {
    double xb = DOUBLE_ZIN0(unit, 1);
    nova::spow_vec_simd(OUT(0), IN(0), xb, inNumSamples);
}
#endif

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER(ring1, sc_ring1)
NOVA_BINARY_WRAPPER(ring2, sc_ring2)
NOVA_BINARY_WRAPPER(ring3, sc_ring3)
NOVA_BINARY_WRAPPER(ring4, sc_ring4)
#endif


void ring1_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*b+a;});
}

void ring1_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
    double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
    if (kr_prev == kr_next) {
        if (kr_prev == 0.f) {
            DOUBLE_ZCOPY(unit, inNumSamples, 0);
        } else if (kr_prev == 1.f) {
            DOUBLE_LOOP1_k(unit, inNumSamples, 0, [](double a){return a + a;});
        } else {
            DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev](double a){return a * kr_prev + a;});
        }
    } else {
        slope_ak(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * b + a;});
    }
}

void ring1_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCLEAR(inNumSamples);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return kr_prev * b + kr_prev;});
      }
  } else {
      slope_ka(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * b + a;});
  }
}

void ring1_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a * b + a;}
  );
}


void ring1_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a * b + a;}
  );
}


void ring2_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*b+a+b;});
}

void ring2_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 0);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev](double a){return a * kr_prev + a + kr_prev;});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * b + a + b;});
  }
}

void ring2_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCOPY(unit, inNumSamples, 1);
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return kr_prev * b + kr_prev + b;});
      }
  } else {
      slope_ka(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * b + a + b;});
  }
}

void ring2_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a * b + a + b;}
  );
}


void ring2_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a * b + a + b;}
  );
}


void ring3_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*a*b;});
}

void ring3_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
  if (kr_prev == kr_next) {
      if (kr_prev == 0.f) {
          DOUBLE_ZCLEAR(inNumSamples);
      } else if (kr_prev == 1.f) {
          DOUBLE_LOOP1_k(unit, inNumSamples, 0, [](double a){return a * a;});
      } else {
          DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev](double a){return a * a * kr_prev;});
      }
  } else {
      slope_ak(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * a * b;});
  }
}

void ring3_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
    double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
    if (kr_prev == kr_next) {
        if (kr_prev == 0.f) {
            DOUBLE_ZCLEAR(inNumSamples);
        } else if (kr_prev == 1.f){
            DOUBLE_ZCOPY(unit, inNumSamples, 1);
        } else {
            DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return kr_prev * kr_prev * b;});
        }
    } else {
        slope_ka(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * a * b;});
    }
}

void ring3_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a * a * b;}
  );
}


void ring3_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a * a * b;}
  );
}


void ring4_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*a*b - a*b*b;});
}

void ring4_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
    double kr_prev = unit->mPrevB; double kr_next = DOUBLE_ZIN0(unit, 1);
    if (kr_prev == kr_next) {
        if (kr_prev == 0.f) {
            DOUBLE_ZCLEAR(inNumSamples);
        } else if (kr_prev == 1.f) {
            DOUBLE_LOOP1_k(unit, inNumSamples, 0, [](double a){return a * a - a;});
        } else {
            DOUBLE_LOOP1_k(unit, inNumSamples, 0, [kr_prev](double a){return a * a * kr_prev - a * kr_prev * kr_prev;});
        }
    } else {
        slope_ak(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * a * b - a * b * b;});
    }
}

void ring4_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
    double kr_prev = unit->mPrevA; double kr_next = DOUBLE_ZIN0(unit, 0);
    if (kr_prev == kr_next) {
        if (kr_prev == 0.f) {
            DOUBLE_ZCLEAR(inNumSamples);
        } else if (kr_prev == 1.f){
            DOUBLE_LOOP1_k(unit, inNumSamples, 1, [](double b){return b - b * b;});
        } else {
            DOUBLE_LOOP1_k(unit, inNumSamples, 1, [kr_prev](double b){return kr_prev * kr_prev * b - kr_prev * b * b;});
        }
    } else {
        slope_ka(unit, inNumSamples, kr_prev, kr_next, [](double a, double b){return a * a * b - a * b * b;});
    }
}

void ring4_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a * a * b - a * b * b;}
  );
}


void ring4_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a * a * b - a * b * b;}
  );
}


void thresh_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a < b ? 0.f : a;});
}

void thresh_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return a < b ? 0.f : a;}
  );
}

void thresh_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a < b ? 0.f : a;}
  );
}

void thresh_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a < b ? 0.f : a;});
}


void thresh_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return a < b ? 0.f : a;});
}


void clip2_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a > b ? b : (a < -b ? -b : a);});
}

void clip2_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return a > b ? b : (a < -b ? -b : a);}
  );
}

void clip2_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a > b ? b : (a < -b ? -b : a);}
  );
}

void clip2_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a > b ? b : (a < -b ? -b : a);}
  );
}


void clip2_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a > b ? b : (a < -b ? -b : a);}
  );
}


#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(clip2, clip2)
#endif


void excess_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a > b ? a - b : (a < -b ? a + b : 0.f);});
}

void excess_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return a > b ? a - b : (a < -b ? a + b : 0.f);}
  );
}

void excess_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a > b ? a - b : (a < -b ? a + b : 0.f);}
  );
}

void excess_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples, [](double a, double b){return a > b ? a - b : (a < -b ? a + b : 0.f);});
}

void excess_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples, [](double a, double b){return a > b ? a - b : (a < -b ? a + b : 0.f);});
}

void rrand_aa(SuperBinaryOpUGen* unit, int inNumSamples) {

    RGET

    DOUBLE_LOOP1(unit, inNumSamples,
      [&s1,&s2,&s3](double a, double b) -> double{return (b > a ? a + frand2(s1, s2, s3) * (b - a) : (b + frand2(s1, s2, s3) * (a - b)));}
    );

    RPUT
}

void rrand_ak(SuperBinaryOpUGen* unit, int inNumSamples) {

    RGET
    DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
      [&s1,&s2,&s3](double a, double b) -> double{return (b > a ? a + frand2(s1, s2, s3) * (b - a) : (b + frand2(s1, s2, s3) * (a - b)));}
    );
    RPUT
}

void rrand_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  RGET
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [&s1,&s2,&s3](double a, double b) -> double{return (b > a ? a + frand2(s1, s2, s3) * (b - a) : (b + frand2(s1, s2, s3) * (a - b)));}
  );
  RPUT
}

void rrand_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  RGET
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [&s1,&s2,&s3](double a, double b) -> double{return (b > a ? a + frand2(s1, s2, s3) * (b - a) : (b + frand2(s1, s2, s3) * (a - b)));}
  );
  RPUT
}


void rrand_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  RGET
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [&s1,&s2,&s3](double a, double b) -> double{return (b > a ? a + frand2(s1, s2, s3) * (b - a) : (b + frand2(s1, s2, s3) * (a - b)));}
  );
  RPUT
}


void exprand_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,
      [unit](double a, double b) {RGen& rgen = *unit->mParent->mRGen; return b > a ? rgen.exprandrng(a, b) : rgen.exprandrng(b, a);}
    );
}

void exprand_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [unit](double a, double b) {RGen& rgen = *unit->mParent->mRGen; return b > a ? rgen.exprandrng(a, b) : rgen.exprandrng(b, a);}
  );
}

void exprand_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [unit](double a, double b) {RGen& rgen = *unit->mParent->mRGen; return b > a ? rgen.exprandrng(a, b) : rgen.exprandrng(b, a);}
  );
}

void exprand_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [unit](double a, double b) {RGen& rgen = *unit->mParent->mRGen; return b > a ? rgen.exprandrng(a, b) : rgen.exprandrng(b, a);}
  );
}


void exprand_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [unit](double a, double b) {RGen& rgen = *unit->mParent->mRGen; return b > a ? rgen.exprandrng(a, b) : rgen.exprandrng(b, a);}
  );
}


void lt_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a < b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(lt, less)
#endif


void lt_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return a < b ? 1.f : 0.f;}
  );
}

void lt_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a < b ? 1.f : 0.f;}
  );
}

void lt_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a < b ? 1.f : 0.f;}
  );
}


void lt_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a < b ? 1.f : 0.f;}
  );
}


void le_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a <= b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(le, less_equal)
#endif

void le_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a <= b ? 1.f : 0.f;});
}

void le_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return a <= b ? 1.f : 0.f;});
}

void le_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return a <= b ? 1.f : 0.f;});
}


void le_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return a <= b ? 1.f : 0.f;});
}


void gt_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a > b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(gt, greater)
#endif


void gt_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a > b ? 1.f : 0.f;});
}

void gt_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return a > b ? 1.f : 0.f;});
}

void gt_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a > b ? 1.f : 0.f;});
}


void gt_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return a > b ? 1.f : 0.f;});
}


void ge_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a >= b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(ge, greater_equal)
#endif


void ge_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a >= b ? 1.f : 0.f;});
}

void ge_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return a >= b ? 1.f : 0.f;});
}

void ge_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return a >= b ? 1.f : 0.f;});
}


void ge_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return a >= b ? 1.f : 0.f;});
}


void eq_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a == b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(eq, equal)
#endif

void eq_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a == b ? 1.f : 0.f;});

}

void eq_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return a == b ? 1.f : 0.f;});
}

void eq_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return a == b ? 1.f : 0.f;});
}


void eq_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return a == b ? 1.f : 0.f;});
}


void neq_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a != b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(neq, notequal)
#endif


void neq_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a != b ? 1.f : 0.f;});
}

void neq_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return a != b ? 1.f : 0.f;});
}

void neq_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return a != b ? 1.f : 0.f;});
}


void neq_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return a != b ? 1.f : 0.f;});
}

#ifdef NOVA_SIMD
NOVA_BINARY_WRAPPER_K(sumsqr, sc_sumsqr)
NOVA_BINARY_WRAPPER_K(difsqr, sc_difsqr)
NOVA_BINARY_WRAPPER_K(sqrsum, sc_sqrsum)
NOVA_BINARY_WRAPPER_K(sqrdif, sc_sqrdif)
#endif

void sumsqr_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*a+b*b;});
}

void sumsqr_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,
    [](double a, double b){return a*a+b*b;}
  );
}

void sumsqr_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,
    [](double a, double b){return a*a+b*b;}
  );
}


void sumsqr_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,
    [](double a, double b){return a*a+b*b;}
  );
}


void sumsqr_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,
    [](double a, double b){return a*a+b*b;}
  );
}


void difsqr_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return a*a-b*b;});
}

void difsqr_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return a*a-b*b;});
}

void difsqr_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return a*a-b*b;});
}

void difsqr_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return a*a-b*b;});
}


void difsqr_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return a*a-b*b;});
}


void sqrsum_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){double sum=a+b; return sum*sum;});
}

void sqrsum_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){double sum=a+b; return sum*sum;});
}

void sqrsum_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){double sum=a+b; return sum*sum;});
}

void sqrsum_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){double sum=a+b; return sum*sum;});
}


void sqrsum_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){double sum=a+b; return sum*sum;});
}


void sqrdif_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){double diff=a-b; return diff*diff;});
}

void sqrdif_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){double diff=a-b; return diff*diff;});
}

void sqrdif_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){double diff=a-b; return diff*diff;});
}


void sqrdif_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){double diff=a-b; return diff*diff;});
}


void sqrdif_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){double diff=a-b; return diff*diff;});
}


void absdif_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return fabs( a - b );});
}

void absdif_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return fabs( a - b );});
}

void absdif_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return fabs( a - b );});
}

void absdif_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return fabs( a - b );});
}


void absdif_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return fabs( a - b );});
}


void round_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_round( a , b );});
}

void round_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return sc_round( a , b );});
}

void round_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return sc_round( a , b );});
}


void round_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return sc_round( a , b );});
}


void round_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return sc_round( a , b );});
}


void roundUp_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_roundUp( a , b );});
}

void roundUp_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return sc_roundUp( a , b );});
}

void roundUp_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return sc_roundUp( a , b );});
}


void roundUp_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return sc_roundUp( a , b );});
}


void roundUp_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return sc_roundUp( a , b );});
}


void trunc_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_trunc( a , b );});
}

void trunc_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return sc_trunc( a , b );});
}

void trunc_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return sc_trunc( a , b );});
}

void trunc_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return sc_trunc( a , b );});
}


void trunc_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return sc_trunc( a , b );});
}


void fold2_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_fold( a , -b, b );});
}

void fold2_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return sc_fold( a , -b, b );});
}

void fold2_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return sc_fold( a , -b, b );});
}

void fold2_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return sc_fold( a , -b, b );});
}


void fold2_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return sc_fold( a , -b, b );});
}


void wrap2_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_wrap( a , -b, b );});
}

void wrap2_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return sc_wrap( a , -b, b );});
}

void wrap2_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return sc_wrap( a , -b, b );});
}

void wrap2_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return sc_wrap( a , -b, b );});
}


void wrap2_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return sc_wrap( a , -b, b );});
}


void atan2_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return atan2( a , b );});
}

void atan2_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return atan2( a , b );});
}

void atan2_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return atan2( a , b );});
}

void atan2_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return atan2( a , b );});
}


void atan2_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return atan2( a , b );});
}


void hypot_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return hypotf( a , b );});
}

void hypot_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return hypotf( a , b );});
}

void hypot_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return hypotf( a , b );});
}

void hypot_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return hypotf( a , b );});
}


void hypot_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return hypotf( a , b );});
}


void hypotx_aa(SuperBinaryOpUGen* unit, int inNumSamples) {
    DOUBLE_LOOP1(unit, inNumSamples,  [](double a, double b){return sc_hypotx( a , b );});
}

void hypotx_ak(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ak_slope(unit, inNumSamples,  [](double a, double b){return sc_hypotx( a , b );});
}

void hypotx_ka(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ka_slope(unit, inNumSamples,  [](double a, double b){return sc_hypotx( a , b );});
}

void hypotx_ia(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ia(unit, inNumSamples,  [](double a, double b){return sc_hypotx( a , b );});
}


void hypotx_ai(SuperBinaryOpUGen* unit, int inNumSamples) {
  DOUBLE_LOOP1_ai(unit, inNumSamples,  [](double a, double b){return sc_hypotx( a , b );});
}

static BinaryOpFunc ChooseOneSampleFunc(SuperBinaryOpUGen* unit) {
    BinaryOpFunc func = &zero_1;

    switch (unit->mSpecialIndex) {
    // case opSilence2 : func = &zero_1; break;
    case opAdd:
        func = &add_1;
        break;
    case opSub:
        func = &sub_1;
        break;
    case opMul:
        func = &mul_1;
        break;
    case opFDiv:
        func = &div_1;
        break;
    case opIDiv:
        func = &idiv_1;
        break;
    case opMod:
        func = &mod_1;
        break;
    case opEQ:
        func = &eq_1;
        break;
    case opNE:
        func = &neq_1;
        break;
    case opLT:
        func = &lt_1;
        break;
    case opGT:
        func = &gt_1;
        break;
    case opLE:
        func = &le_1;
        break;
    case opGE:
        func = &ge_1;
        break;
    case opMin:
        func = &min_1;
        break;
    case opMax:
        func = &max_1;
        break;
    case opBitAnd:
        func = &and_1;
        break;
    case opBitOr:
        func = &or_1;
        break;
    case opBitXor:
        func = &xor_1;
        break;
    case opShiftRight:
        func = &rightShift_1;
        break;
    case opShiftLeft:
        func = &leftShift_1;
        break;
    case opLCM:
        func = &lcm_1;
        break;
    case opGCD:
        func = &gcd_1;
        break;
    case opRound:
        func = &round_1;
        break;
    case opRoundUp:
        func = &roundUp_1;
        break;
    case opTrunc:
        func = &trunc_1;
        break;
    case opAtan2:
        func = &atan2_1;
        break;
    case opHypot:
        func = &hypot_1;
        break;
    case opHypotx:
        func = &hypotx_1;
        break;
    case opPow:
        func = &pow_1;
        break;
    case opRing1:
        func = &ring1_1;
        break;
    case opRing2:
        func = &ring2_1;
        break;
    case opRing3:
        func = &ring3_1;
        break;
    case opRing4:
        func = &ring4_1;
        break;
    case opDifSqr:
        func = &difsqr_1;
        break;
    case opSumSqr:
        func = &sumsqr_1;
        break;
    case opSqrSum:
        func = &sqrsum_1;
        break;
    case opSqrDif:
        func = &sqrdif_1;
        break;
    case opAbsDif:
        func = &absdif_1;
        break;
    case opThresh:
        func = &thresh_1;
        break;
    case opAMClip:
        func = &amclip_1;
        break;
    case opScaleNeg:
        func = &scaleneg_1;
        break;
    case opClip2:
        func = &clip2_1;
        break;
    case opFold2:
        func = &fold2_1;
        break;
    case opWrap2:
        func = &wrap2_1;
        break;
    case opExcess:
        func = &excess_1;
        break;
    case opFirstArg:
        func = &firstarg_1;
        break;
    case opRandRange:
        func = &rrand_1;
        break;
    case opExpRandRange:
        func = &exprand_1;
        break;
    // case opSecondArg : func = &secondarg_1; break;
    default:
        func = &add_1;
        break;
    }
    return func;
}


static BinaryOpFunc ChooseDemandFunc(SuperBinaryOpUGen* unit) {
    BinaryOpFunc func = &zero_1;

    switch (unit->mSpecialIndex) {
    // case opSilence2 : func = &zero_d; break;
    case opAdd:
        func = &add_d;
        break;
    case opSub:
        func = &sub_d;
        break;
    case opMul:
        func = &mul_d;
        break;
    case opFDiv:
        func = &div_d;
        break;
    case opIDiv:
        func = &idiv_d;
        break;
    case opMod:
        func = &mod_d;
        break;
    case opEQ:
        func = &eq_d;
        break;
    case opNE:
        func = &neq_d;
        break;
    case opLT:
        func = &lt_d;
        break;
    case opGT:
        func = &gt_d;
        break;
    case opLE:
        func = &le_d;
        break;
    case opGE:
        func = &ge_d;
        break;
    case opMin:
        func = &min_d;
        break;
    case opMax:
        func = &max_d;
        break;
    case opBitAnd:
        func = &and_d;
        break;
    case opBitOr:
        func = &or_d;
        break;
    case opBitXor:
        func = &xor_d;
        break;
    case opShiftRight:
        func = &rightShift_d;
        break;
    case opShiftLeft:
        func = &leftShift_d;
        break;
    case opLCM:
        func = &lcm_d;
        break;
    case opGCD:
        func = &gcd_d;
        break;
    case opRound:
        func = &round_d;
        break;
    case opRoundUp:
        func = &roundUp_d;
        break;
    case opTrunc:
        func = &trunc_d;
        break;
    case opAtan2:
        func = &atan2_d;
        break;
    case opHypot:
        func = &hypot_d;
        break;
    case opHypotx:
        func = &hypotx_d;
        break;
    case opPow:
        func = &pow_d;
        break;
    case opRing1:
        func = &ring1_d;
        break;
    case opRing2:
        func = &ring2_d;
        break;
    case opRing3:
        func = &ring3_d;
        break;
    case opRing4:
        func = &ring4_d;
        break;
    case opDifSqr:
        func = &difsqr_d;
        break;
    case opSumSqr:
        func = &sumsqr_d;
        break;
    case opSqrSum:
        func = &sqrsum_d;
        break;
    case opSqrDif:
        func = &sqrdif_d;
        break;
    case opAbsDif:
        func = &absdif_d;
        break;
    case opThresh:
        func = &thresh_d;
        break;
    case opAMClip:
        func = &amclip_d;
        break;
    case opScaleNeg:
        func = &scaleneg_d;
        break;
    case opClip2:
        func = &clip2_d;
        break;
    case opFold2:
        func = &fold2_d;
        break;
    case opWrap2:
        func = &wrap2_d;
        break;
    case opExcess:
        func = &excess_d;
        break;
    case opFirstArg:
        func = &firstarg_d;
        break;
    case opRandRange:
        func = &rrand_d;
        break;
    case opExpRandRange:
        func = &exprand_d;
        break;

    // case opSecondArg : func = &secondarg_d; break;
    default:
        func = &add_d;
        break;
    }
    return func;
}


static BinaryOpFunc ChooseNormalFunc(SuperBinaryOpUGen* unit) {
    BinaryOpFunc func = &zero_1;

    // TODO: should rates really be equal between msd and lsd?
    int rateA = sc_max(INRATE(0), INRATE(1));
    int rateB = sc_max(INRATE(2), INRATE(3));

    switch (rateA) {
    case calc_FullRate:
        switch (rateB) {
        case calc_FullRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_aa;
                break;
            case opSub:
                func = &sub_aa;
                break;
            case opMul:
                func = &mul_aa;
                break;
            case opFDiv:
                func = &div_aa;
                break;
            case opIDiv:
                func = &idiv_aa;
                break;
            case opMod:
                func = &mod_aa;
                break;
            case opEQ:
                func = &eq_aa;
                break;
            case opNE:
                func = &neq_aa;
                break;
            case opLT:
                func = &lt_aa;
                break;
            case opGT:
                func = &gt_aa;
                break;
            case opLE:
                func = &le_aa;
                break;
            case opGE:
                func = &ge_aa;
                break;
            case opMin:
                func = &min_aa;
                break;
            case opMax:
                func = &max_aa;
                break;
            case opBitAnd:
                func = &and_aa;
                break;
            case opBitOr:
                func = &or_aa;
                break;
            case opBitXor:
                func = &xor_aa;
                break;
            case opShiftRight:
                func = &rightShift_aa;
                break;
            case opShiftLeft:
                func = &leftShift_aa;
                break;
            case opLCM:
                func = &lcm_aa;
                break;
            case opGCD:
                func = &gcd_aa;
                break;
            case opRound:
                func = &round_aa;
                break;
            case opRoundUp:
                func = &roundUp_aa;
                break;
            case opTrunc:
                func = &trunc_aa;
                break;
            case opAtan2:
                func = &atan2_aa;
                break;
            case opHypot:
                func = &hypot_aa;
                break;
            case opHypotx:
                func = &hypotx_aa;
                break;
            case opPow:
                func = &pow_aa;
                break;
            case opRing1:
                func = &ring1_aa;
                break;
            case opRing2:
                func = &ring2_aa;
                break;
            case opRing3:
                func = &ring3_aa;
                break;
            case opRing4:
                func = &ring4_aa;
                break;
            case opDifSqr:
                func = &difsqr_aa;
                break;
            case opSumSqr:
                func = &sumsqr_aa;
                break;
            case opSqrSum:
                func = &sqrsum_aa;
                break;
            case opSqrDif:
                func = &sqrdif_aa;
                break;
            case opAbsDif:
                func = &absdif_aa;
                break;
            case opThresh:
                func = &thresh_aa;
                break;
            case opAMClip:
                func = &amclip_aa;
                break;
            case opScaleNeg:
                func = &scaleneg_aa;
                break;
            case opClip2:
                func = &clip2_aa;
                break;
            case opFold2:
                func = &fold2_aa;
                break;
            case opWrap2:
                func = &wrap2_aa;
                break;
            case opExcess:
                func = &excess_aa;
                break;
            case opRandRange:
                func = &rrand_aa;
                break;
            case opExpRandRange:
                func = &exprand_aa;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
                // case opSecondArg : func = &secondarg_aa; break;


            default:
                func = &add_aa;
                break;
            }
            break;
        case calc_BufRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ak;
                break;
            case opSub:
                func = &sub_ak;
                break;
            case opMul:
                func = &mul_ak;
                break;
            case opFDiv:
                func = &div_ak;
                break;
            case opIDiv:
                func = &idiv_ak;
                break;
            case opMod:
                func = &mod_ak;
                break;
            case opEQ:
                func = &eq_ak;
                break;
            case opNE:
                func = &neq_ak;
                break;
            case opLT:
                func = &lt_ak;
                break;
            case opGT:
                func = &gt_ak;
                break;
            case opLE:
                func = &le_ak;
                break;
            case opGE:
                func = &ge_ak;
                break;
            case opMin:
                func = &min_ak;
                break;
            case opMax:
                func = &max_ak;
                break;
            case opBitAnd:
                func = &and_ak;
                break;
            case opBitOr:
                func = &or_ak;
                break;
            case opBitXor:
                func = &xor_ak;
                break;
            case opShiftRight:
                func = &rightShift_ak;
                break;
            case opShiftLeft:
                func = &leftShift_ak;
                break;
            case opLCM:
                func = &lcm_ak;
                break;
            case opGCD:
                func = &gcd_ak;
                break;
            case opRound:
                func = &round_ak;
                break;
            case opRoundUp:
                func = &roundUp_ak;
                break;
            case opTrunc:
                func = &trunc_ak;
                break;
            case opAtan2:
                func = &atan2_ak;
                break;
            case opHypot:
                func = &hypot_ak;
                break;
            case opHypotx:
                func = &hypotx_ak;
                break;
            case opPow:
                func = &pow_ak;
                break;
            case opRing1:
                func = &ring1_ak;
                break;
            case opRing2:
                func = &ring2_ak;
                break;
            case opRing3:
                func = &ring3_ak;
                break;
            case opRing4:
                func = &ring4_ak;
                break;
            case opDifSqr:
                func = &difsqr_ak;
                break;
            case opSumSqr:
                func = &sumsqr_ak;
                break;
            case opSqrSum:
                func = &sqrsum_ak;
                break;
            case opSqrDif:
                func = &sqrdif_ak;
                break;
            case opAbsDif:
                func = &absdif_ak;
                break;
            case opThresh:
                func = &thresh_ak;
                break;
            case opAMClip:
                func = &amclip_ak;
                break;
            case opScaleNeg:
                func = &scaleneg_ak;
                break;
            case opClip2:
                func = &clip2_ak;
                break;
            case opFold2:
                func = &fold2_ak;
                break;
            case opWrap2:
                func = &wrap2_ak;
                break;
            case opExcess:
                func = &excess_ak;
                break;
            case opRandRange:
                func = &rrand_ak;
                break;
            case opExpRandRange:
                func = &exprand_ak;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
                // case opSecondArg : func = &secondarg_aa; break;


            default:
                func = &add_ak;
                break;
            }
            break;
        case calc_ScalarRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ai;
                break;
            case opSub:
                func = &sub_ai;
                break;
            case opMul:
                func = &mul_ai;
                break;
            case opFDiv:
                func = &div_ai;
                break;
            case opIDiv:
                func = &idiv_ai;
                break;
            case opMod:
                func = &mod_ai;
                break;
            case opEQ:
                func = &eq_ai;
                break;
            case opNE:
                func = &neq_ai;
                break;
            case opLT:
                func = &lt_ai;
                break;
            case opGT:
                func = &gt_ai;
                break;
            case opLE:
                func = &le_ai;
                break;
            case opGE:
                func = &ge_ai;
                break;
            case opMin:
                func = &min_ai;
                break;
            case opMax:
                func = &max_ai;
                break;
            case opBitAnd:
                func = &and_ai;
                break;
            case opBitOr:
                func = &or_ai;
                break;
            case opBitXor:
                func = &xor_ai;
                break;
            case opShiftRight:
                func = &rightShift_ai;
                break;
            case opShiftLeft:
                func = &leftShift_ai;
                break;
            case opLCM:
                func = &lcm_ai;
                break;
            case opGCD:
                func = &gcd_ai;
                break;
            case opRound:
                func = &round_ai;
                break;
            case opRoundUp:
                func = &roundUp_ai;
                break;
            case opTrunc:
                func = &trunc_ai;
                break;
            case opAtan2:
                func = &atan2_ai;
                break;
            case opHypot:
                func = &hypot_ai;
                break;
            case opHypotx:
                func = &hypotx_ai;
                break;
            case opPow:
                func = &pow_ai;
                break;
            case opRing1:
                func = &ring1_ai;
                break;
            case opRing2:
                func = &ring2_ai;
                break;
            case opRing3:
                func = &ring3_ai;
                break;
            case opRing4:
                func = &ring4_ai;
                break;
            case opDifSqr:
                func = &difsqr_ai;
                break;
            case opSumSqr:
                func = &sumsqr_ai;
                break;
            case opSqrSum:
                func = &sqrsum_ai;
                break;
            case opSqrDif:
                func = &sqrdif_ai;
                break;
            case opAbsDif:
                func = &absdif_ai;
                break;
            case opThresh:
                func = &thresh_ai;
                break;
            case opAMClip:
                func = &amclip_ai;
                break;
            case opScaleNeg:
                func = &scaleneg_ai;
                break;
            case opClip2:
                func = &clip2_ai;
                break;
            case opFold2:
                func = &fold2_ai;
                break;
            case opWrap2:
                func = &wrap2_ai;
                break;
            case opExcess:
                func = &excess_ai;
                break;
            case opRandRange:
                func = &rrand_ai;
                break;
            case opExpRandRange:
                func = &exprand_ai;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ai;
                break;
            }
        }
        break;
    case calc_BufRate:
        if (rateB == calc_FullRate) {
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ka;
                break;
            case opSub:
                func = &sub_ka;
                break;
            case opMul:
                func = &mul_ka;
                break;
            case opFDiv:
                func = &div_ka;
                break;
            case opIDiv:
                func = &idiv_ka;
                break;
            case opMod:
                func = &mod_ka;
                break;
            case opEQ:
                func = &eq_ka;
                break;
            case opNE:
                func = &neq_ka;
                break;
            case opLT:
                func = &lt_ka;
                break;
            case opGT:
                func = &gt_ka;
                break;
            case opLE:
                func = &le_ka;
                break;
            case opGE:
                func = &ge_ka;
                break;
            case opMin:
                func = &min_ka;
                break;
            case opMax:
                func = &max_ka;
                break;
            case opBitAnd:
                func = &and_ka;
                break;
            case opBitOr:
                func = &or_ka;
                break;
            case opBitXor:
                func = &xor_ka;
                break;
            case opShiftRight:
                func = &rightShift_ka;
                break;
            case opShiftLeft:
                func = &leftShift_ka;
                break;
            case opLCM:
                func = &lcm_ka;
                break;
            case opGCD:
                func = &gcd_ka;
                break;
            case opRound:
                func = &round_ka;
                break;
            case opRoundUp:
                func = &roundUp_ka;
                break;
            case opTrunc:
                func = &trunc_ka;
                break;
            case opAtan2:
                func = &atan2_ka;
                break;
            case opHypot:
                func = &hypot_ka;
                break;
            case opHypotx:
                func = &hypotx_ka;
                break;
            case opPow:
                func = &pow_ka;
                break;
            case opRing1:
                func = &ring1_ka;
                break;
            case opRing2:
                func = &ring2_ka;
                break;
            case opRing3:
                func = &ring3_ka;
                break;
            case opRing4:
                func = &ring4_ka;
                break;
            case opDifSqr:
                func = &difsqr_ka;
                break;
            case opSumSqr:
                func = &sumsqr_ka;
                break;
            case opSqrSum:
                func = &sqrsum_ka;
                break;
            case opSqrDif:
                func = &sqrdif_ka;
                break;
            case opAbsDif:
                func = &absdif_ka;
                break;
            case opThresh:
                func = &thresh_ka;
                break;
            case opAMClip:
                func = &amclip_ka;
                break;
            case opScaleNeg:
                func = &scaleneg_ka;
                break;
            case opClip2:
                func = &clip2_ka;
                break;
            case opFold2:
                func = &fold2_ka;
                break;
            case opWrap2:
                func = &wrap2_ka;
                break;
            case opExcess:
                func = &excess_ka;
                break;
            case opRandRange:
                func = &rrand_ka;
                break;
            case opExpRandRange:
                func = &exprand_ka;
                break;
            // case opFirstArg : func = &firstarg_aa; break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ka;
                break;
            }
        } else {
            // this should have been caught by mBufLength == 1
            func = &zero_aa;
        }
        break;
    case calc_ScalarRate:
        if (rateB == calc_FullRate) {
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ia;
                break;
            case opSub:
                func = &sub_ia;
                break;
            case opMul:
                func = &mul_ia;
                break;
            case opFDiv:
                func = &div_ia;
                break;
            case opIDiv:
                func = &idiv_ia;
                break;
            case opMod:
                func = &mod_ia;
                break;
            case opEQ:
                func = &eq_ia;
                break;
            case opNE:
                func = &neq_ia;
                break;
            case opLT:
                func = &lt_ia;
                break;
            case opGT:
                func = &gt_ia;
                break;
            case opLE:
                func = &le_ia;
                break;
            case opGE:
                func = &ge_ia;
                break;
            case opMin:
                func = &min_ia;
                break;
            case opMax:
                func = &max_ia;
                break;
            case opBitAnd:
                func = &and_ia;
                break;
            case opBitOr:
                func = &or_ia;
                break;
            case opBitXor:
                func = &xor_ia;
                break;
            case opShiftRight:
                func = &rightShift_ia;
                break;
            case opShiftLeft:
                func = &leftShift_ia;
                break;
            case opLCM:
                func = &lcm_ia;
                break;
            case opGCD:
                func = &gcd_ia;
                break;
            case opRound:
                func = &round_ia;
                break;
            case opRoundUp:
                func = &roundUp_ia;
                break;
            case opTrunc:
                func = &trunc_ia;
                break;
            case opAtan2:
                func = &atan2_ia;
                break;
            case opHypot:
                func = &hypot_ia;
                break;
            case opHypotx:
                func = &hypotx_ia;
                break;
            case opPow:
                func = &pow_ia;
                break;
            case opRing1:
                func = &ring1_ia;
                break;
            case opRing2:
                func = &ring2_ia;
                break;
            case opRing3:
                func = &ring3_ia;
                break;
            case opRing4:
                func = &ring4_ia;
                break;
            case opDifSqr:
                func = &difsqr_ia;
                break;
            case opSumSqr:
                func = &sumsqr_ia;
                break;
            case opSqrSum:
                func = &sqrsum_ia;
                break;
            case opSqrDif:
                func = &sqrdif_ia;
                break;
            case opAbsDif:
                func = &absdif_ia;
                break;
            case opThresh:
                func = &thresh_ia;
                break;
            case opAMClip:
                func = &amclip_ia;
                break;
            case opScaleNeg:
                func = &scaleneg_ia;
                break;
            case opClip2:
                func = &clip2_ia;
                break;
            case opFold2:
                func = &fold2_ia;
                break;
            case opWrap2:
                func = &wrap2_ia;
                break;
            case opExcess:
                func = &excess_ia;
                break;
            case opRandRange:
                func = &rrand_ia;
                break;
            case opExpRandRange:
                func = &exprand_ia;
                break;
            // case opFirstArg : func = &firstarg_aa; break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ia;
                break;
            }
        } else {
            // this should have been caught by mBufLength == 1
            func = &zero_aa;
        }
        break;
    }

    return func;
}

#ifdef NOVA_SIMD
static BinaryOpFunc ChooseNovaSimdFunc_64(SuperBinaryOpUGen* unit) {
    BinaryOpFunc func = &zero_1;

    int rateA = INRATE(0);
    int rateB = INRATE(1);

    switch (rateA) {
    case calc_FullRate:
        switch (rateB) {
        case calc_FullRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_aa_nova_64;
                break;
            case opSub:
                func = &sub_aa_nova_64;
                break;
            case opMul:
                func = &mul_aa_nova_64;
                break;
            case opFDiv:
                func = &div_aa_nova;
                break;
            case opIDiv:
                func = &idiv_aa;
                break;
            case opMod:
                func = &mod_aa;
                break;
            case opEQ:
                func = &eq_aa_nova_64;
                break;
            case opNE:
                func = &neq_aa_nova_64;
                break;
            case opLT:
                func = &lt_aa_nova_64;
                break;
            case opGT:
                func = &gt_aa_nova_64;
                break;
            case opLE:
                func = &le_aa_nova_64;
                break;
            case opGE:
                func = &ge_aa_nova_64;
                break;
            case opMin:
                func = &min_aa_nova_64;
                break;
            case opMax:
                func = &max_aa_nova_64;
                break;
            case opBitAnd:
                func = &and_aa;
                break;
            case opBitOr:
                func = &or_aa;
                break;
            case opBitXor:
                func = &xor_aa;
                break;
            case opShiftRight:
                func = &rightShift_aa;
                break;
            case opShiftLeft:
                func = &leftShift_aa;
                break;
            case opLCM:
                func = &lcm_aa;
                break;
            case opGCD:
                func = &gcd_aa;
                break;
            case opRound:
                func = &round_aa;
                break;
            case opRoundUp:
                func = &roundUp_aa;
                break;
            case opTrunc:
                func = &trunc_aa;
                break;
            case opAtan2:
                func = &atan2_aa;
                break;
            case opHypot:
                func = &hypot_aa;
                break;
            case opHypotx:
                func = &hypotx_aa;
                break;
            case opPow:
                func = &pow_aa_nova;
                break;
            case opRing1:
                func = &ring1_aa_nova_64;
                break;
            case opRing2:
                func = &ring2_aa_nova_64;
                break;
            case opRing3:
                func = &ring3_aa_nova_64;
                break;
            case opRing4:
                func = &ring4_aa_nova_64;
                break;
            case opDifSqr:
                func = &difsqr_aa_nova_64;
                break;
            case opSumSqr:
                func = &sumsqr_aa_nova_64;
                break;
            case opSqrSum:
                func = &sqrsum_aa_nova_64;
                break;
            case opSqrDif:
                func = &sqrdif_aa_nova_64;
                break;
            case opAbsDif:
                func = &absdif_aa;
                break;
            case opThresh:
                func = &thresh_aa;
                break;
            case opAMClip:
                func = &amclip_aa;
                break;
            case opScaleNeg:
                func = &scaleneg_aa;
                break;
            case opClip2:
                func = &clip2_aa_nova_64;
                break;
            case opFold2:
                func = &fold2_aa;
                break;
            case opWrap2:
                func = &wrap2_aa;
                break;
            case opExcess:
                func = &excess_aa;
                break;
            case opRandRange:
                func = &rrand_aa;
                break;
            case opExpRandRange:
                func = &exprand_aa;
                break;
            case opFirstArg:
                func = &firstarg_aa_nova;
                break;
            // case opSecondArg : func = &secondarg_aa_nova; break;
            default:
                func = &add_aa;
                break;
            }
            break;
        case calc_BufRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ak_nova_64;
                break;
            case opSub:
                func = &sub_ak_nova_64;
                break;
            case opMul:
                func = &mul_ak_nova_64;
                break;
            case opFDiv:
                func = &div_ak_nova;
                break;
            case opIDiv:
                func = &idiv_ak;
                break;
            case opMod:
                func = &mod_ak;
                break;
            case opEQ:
                func = &eq_ak_nova_64;
                break;
            case opNE:
                func = &neq_ak_nova_64;
                break;
            case opLT:
                func = &lt_ak_nova_64;
                break;
            case opGT:
                func = &gt_ak_nova_64;
                break;
            case opLE:
                func = &le_ak_nova_64;
                break;
            case opGE:
                func = &ge_ak_nova_64;
                break;
            case opMin:
                func = &min_ak_nova_64;
                break;
            case opMax:
                func = &max_ak_nova_64;
                break;
            case opBitAnd:
                func = &and_ak;
                break;
            case opBitOr:
                func = &or_ak;
                break;
            case opBitXor:
                func = &xor_ak;
                break;
            case opShiftRight:
                func = &rightShift_ak;
                break;
            case opShiftLeft:
                func = &leftShift_ak;
                break;
            case opLCM:
                func = &lcm_ak;
                break;
            case opGCD:
                func = &gcd_ak;
                break;
            case opRound:
                func = &round_ak;
                break;
            case opRoundUp:
                func = &roundUp_ak;
                break;
            case opTrunc:
                func = &trunc_ak;
                break;
            case opAtan2:
                func = &atan2_ak;
                break;
            case opHypot:
                func = &hypot_ak;
                break;
            case opHypotx:
                func = &hypotx_ak;
                break;
            case opPow:
                func = &pow_ak_nova;
                break;
            case opRing1:
                func = &ring1_ak;
                break;
            case opRing2:
                func = &ring2_ak;
                break;
            case opRing3:
                func = &ring3_ak;
                break;
            case opRing4:
                func = &ring4_ak;
                break;
            case opDifSqr:
                func = &difsqr_ak_nova_64;
                break;
            case opSumSqr:
                func = &sumsqr_ak_nova_64;
                break;
            case opSqrSum:
                func = &sqrsum_ak_nova_64;
                break;
            case opSqrDif:
                func = &sqrdif_ak_nova_64;
                break;
            case opAbsDif:
                func = &absdif_ak;
                break;
            case opThresh:
                func = &thresh_ak;
                break;
            case opAMClip:
                func = &amclip_ak;
                break;
            case opScaleNeg:
                func = &scaleneg_ak;
                break;
            case opClip2:
                func = &clip2_ak_nova_64;
                break;
            case opFold2:
                func = &fold2_ak;
                break;
            case opWrap2:
                func = &wrap2_ak;
                break;
            case opExcess:
                func = &excess_ak;
                break;
            case opRandRange:
                func = &rrand_ak;
                break;
            case opExpRandRange:
                func = &exprand_ak;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ak;
                break;
            }
            break;
        case calc_ScalarRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ai_nova_64;
                break;
            case opSub:
                func = &sub_ai_nova_64;
                break;
            case opMul:
                func = &mul_ai_nova_64;
                break;
            case opFDiv:
                func = &div_ai_nova;
                break;
            case opIDiv:
                func = &idiv_ai;
                break;
            case opMod:
                func = &mod_ai;
                break;
            case opEQ:
                func = &eq_ai_nova_64;
                break;
            case opNE:
                func = &neq_ai_nova_64;
                break;
            case opLT:
                func = &lt_ai_nova_64;
                break;
            case opGT:
                func = &gt_ai_nova_64;
                break;
            case opLE:
                func = &le_ai_nova_64;
                break;
            case opGE:
                func = &ge_ai_nova_64;
                break;
            case opMin:
                func = &min_ai_nova_64;
                break;
            case opMax:
                func = &max_ai_nova_64;
                break;
            case opBitAnd:
                func = &and_ai;
                break;
            case opBitOr:
                func = &or_ai;
                break;
            case opBitXor:
                func = &xor_ai;
                break;
            case opShiftRight:
                func = &rightShift_ai;
                break;
            case opShiftLeft:
                func = &leftShift_ai;
                break;
            case opLCM:
                func = &lcm_ai;
                break;
            case opGCD:
                func = &gcd_ai;
                break;
            case opRound:
                func = &round_ai;
                break;
            case opRoundUp:
                func = &roundUp_ai;
                break;
            case opTrunc:
                func = &trunc_ai;
                break;
            case opAtan2:
                func = &atan2_ai;
                break;
            case opHypot:
                func = &hypot_ai;
                break;
            case opHypotx:
                func = &hypotx_ai;
                break;
            case opPow:
                func = &pow_ai_nova;
                break;
            case opRing1:
                func = &ring1_ai_nova_64;
                break;
            case opRing2:
                func = &ring2_ai_nova_64;
                break;
            case opRing3:
                func = &ring3_ai_nova_64;
                break;
            case opRing4:
                func = &ring4_ai_nova_64;
                break;
            case opDifSqr:
                func = &difsqr_ai_nova_64;
                break;
            case opSumSqr:
                func = &sumsqr_ai_nova_64;
                break;
            case opSqrSum:
                func = &sqrsum_ai_nova_64;
                break;
            case opSqrDif:
                func = &sqrdif_ai_nova_64;
                break;
            case opAbsDif:
                func = &absdif_ai;
                break;
            case opThresh:
                func = &thresh_ai;
                break;
            case opAMClip:
                func = &amclip_ai;
                break;
            case opScaleNeg:
                func = &scaleneg_ai;
                break;
            case opClip2:
                func = &clip2_ai_nova_64;
                break;
            case opFold2:
                func = &fold2_ai;
                break;
            case opWrap2:
                func = &wrap2_ai;
                break;
            case opExcess:
                func = &excess_ai;
                break;
            case opRandRange:
                func = &rrand_ai;
                break;
            case opExpRandRange:
                func = &exprand_ai;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ai;
                break;
            }
        }
        break;
    case calc_BufRate:
        if (rateB == calc_FullRate) {
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ka_nova_64;
                break;
            case opSub:
                func = &sub_ka_nova_64;
                break;
            case opMul:
                func = &mul_ka_nova_64;
                break;
            case opFDiv:
                func = &div_ka_nova;
                break;
            case opIDiv:
                func = &idiv_ka;
                break;
            case opMod:
                func = &mod_ka;
                break;
            case opEQ:
                func = &eq_ka_nova_64;
                break;
            case opNE:
                func = &neq_ka_nova_64;
                break;
            case opLT:
                func = &lt_ka_nova_64;
                break;
            case opGT:
                func = &gt_ka_nova_64;
                break;
            case opLE:
                func = &le_ka_nova_64;
                break;
            case opGE:
                func = &ge_ka_nova_64;
                break;
            case opMin:
                func = &min_ka_nova_64;
                break;
            case opMax:
                func = &max_ka_nova_64;
                break;
            case opBitAnd:
                func = &and_ka;
                break;
            case opBitOr:
                func = &or_ka;
                break;
            case opBitXor:
                func = &xor_ka;
                break;
            case opShiftRight:
                func = &rightShift_ka;
                break;
            case opShiftLeft:
                func = &leftShift_ka;
                break;
            case opLCM:
                func = &lcm_ka;
                break;
            case opGCD:
                func = &gcd_ka;
                break;
            case opRound:
                func = &round_ka;
                break;
            case opRoundUp:
                func = &roundUp_ka;
                break;
            case opTrunc:
                func = &trunc_ka;
                break;
            case opAtan2:
                func = &atan2_ka;
                break;
            case opHypot:
                func = &hypot_ka;
                break;
            case opHypotx:
                func = &hypotx_ka;
                break;
            case opPow:
                func = &pow_ka_nova;
                break;
            case opRing1:
                func = &ring1_ka;
                break;
            case opRing2:
                func = &ring2_ka;
                break;
            case opRing3:
                func = &ring3_ka;
                break;
            case opRing4:
                func = &ring4_ka;
                break;
            case opDifSqr:
                func = &difsqr_ka_nova_64;
                break;
            case opSumSqr:
                func = &sumsqr_ka_nova_64;
                break;
            case opSqrSum:
                func = &sqrsum_ka_nova_64;
                break;
            case opSqrDif:
                func = &sqrdif_ka_nova_64;
                break;
            case opAbsDif:
                func = &absdif_ka;
                break;
            case opThresh:
                func = &thresh_ka;
                break;
            case opAMClip:
                func = &amclip_ka;
                break;
            case opScaleNeg:
                func = &scaleneg_ka;
                break;
            case opClip2:
                func = &clip2_ka_nova_64;
                break;
            case opFold2:
                func = &fold2_ka;
                break;
            case opWrap2:
                func = &wrap2_ka;
                break;
            case opExcess:
                func = &excess_ka;
                break;
            case opRandRange:
                func = &rrand_ka;
                break;
            case opExpRandRange:
                func = &exprand_ka;
                break;
            // case opFirstArg : func = &firstarg_aa; break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ka;
                break;
            }
        } else {
            // this should have been caught by mBufLength == 1
            func = &zero_aa;
        }
        break;
    case calc_ScalarRate:
        if (rateB == calc_FullRate) {
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ia_nova_64;
                break;
            case opSub:
                func = &sub_ia_nova_64;
                break;
            case opMul:
                func = &mul_ia_nova_64;
                break;
            case opFDiv:
                func = &div_ia_nova;
                break;
            case opIDiv:
                func = &idiv_ia;
                break;
            case opMod:
                func = &mod_ia;
                break;
            case opEQ:
                func = &eq_ia_nova_64;
                break;
            case opNE:
                func = &neq_ia_nova_64;
                break;
            case opLT:
                func = &lt_ia_nova_64;
                break;
            case opGT:
                func = &gt_ia_nova_64;
                break;
            case opLE:
                func = &le_ia_nova_64;
                break;
            case opGE:
                func = &ge_ia_nova_64;
                break;
            case opMin:
                func = &min_ia_nova_64;
                break;
            case opMax:
                func = &max_ia_nova_64;
                break;
            case opBitAnd:
                func = &and_ia;
                break;
            case opBitOr:
                func = &or_ia;
                break;
            case opBitXor:
                func = &xor_ia;
                break;
            case opShiftRight:
                func = &rightShift_ia;
                break;
            case opShiftLeft:
                func = &leftShift_ia;
                break;
            case opLCM:
                func = &lcm_ia;
                break;
            case opGCD:
                func = &gcd_ia;
                break;
            case opRound:
                func = &round_ia;
                break;
            case opRoundUp:
                func = &roundUp_ia;
                break;
            case opTrunc:
                func = &trunc_ia;
                break;
            case opAtan2:
                func = &atan2_ia;
                break;
            case opHypot:
                func = &hypot_ia;
                break;
            case opHypotx:
                func = &hypotx_ia;
                break;
            case opPow:
                func = &pow_ia_nova;
                break;
            case opRing1:
                func = &ring1_ia_nova_64;
                break;
            case opRing2:
                func = &ring2_ia_nova_64;
                break;
            case opRing3:
                func = &ring3_ia_nova_64;
                break;
            case opRing4:
                func = &ring4_ia_nova_64;
                break;
            case opDifSqr:
                func = &difsqr_ia_nova_64;
                break;
            case opSumSqr:
                func = &sumsqr_ia_nova_64;
                break;
            case opSqrSum:
                func = &sqrsum_ia_nova_64;
                break;
            case opSqrDif:
                func = &sqrdif_ia_nova_64;
                break;
            case opAbsDif:
                func = &absdif_ia;
                break;
            case opThresh:
                func = &thresh_ia;
                break;
            case opAMClip:
                func = &amclip_ia;
                break;
            case opScaleNeg:
                func = &scaleneg_ia;
                break;
            case opClip2:
                func = &clip2_ia_nova_64;
                break;
            case opFold2:
                func = &fold2_ia;
                break;
            case opWrap2:
                func = &wrap2_ia;
                break;
            case opExcess:
                func = &excess_ia;
                break;
            case opRandRange:
                func = &rrand_ia;
                break;
            case opExpRandRange:
                func = &exprand_ia;
                break;
            // case opFirstArg : func = &firstarg_aa; break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ia;
                break;
            }
        } else {
            // this should have been caught by mBufLength == 1
            func = &zero_aa;
        }
        break;
    }

    return func;
}


static BinaryOpFunc ChooseNovaSimdFunc(SuperBinaryOpUGen* unit) {
    if (BUFLENGTH == 64)
        return ChooseNovaSimdFunc_64(unit);

    BinaryOpFunc func = &zero_1;

    int rateA = INRATE(0);
    int rateB = INRATE(1);

    switch (rateA) {
    case calc_FullRate:
        switch (rateB) {
        case calc_FullRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_aa_nova;
                break;
            case opSub:
                func = &sub_aa_nova;
                break;
            case opMul:
                func = &mul_aa_nova;
                break;
            case opFDiv:
                func = &div_aa_nova;
                break;
            case opIDiv:
                func = &idiv_aa;
                break;
            case opMod:
                func = &mod_aa;
                break;
            case opEQ:
                func = &eq_aa_nova;
                break;
            case opNE:
                func = &neq_aa_nova;
                break;
            case opLT:
                func = &lt_aa_nova;
                break;
            case opGT:
                func = &gt_aa_nova;
                break;
            case opLE:
                func = &le_aa_nova;
                break;
            case opGE:
                func = &ge_aa_nova;
                break;
            case opMin:
                func = &min_aa_nova;
                break;
            case opMax:
                func = &max_aa_nova;
                break;
            case opBitAnd:
                func = &and_aa;
                break;
            case opBitOr:
                func = &or_aa;
                break;
            case opBitXor:
                func = &xor_aa;
                break;
            case opShiftRight:
                func = &rightShift_aa;
                break;
            case opShiftLeft:
                func = &leftShift_aa;
                break;
            case opLCM:
                func = &lcm_aa;
                break;
            case opGCD:
                func = &gcd_aa;
                break;
            case opRound:
                func = &round_aa;
                break;
            case opRoundUp:
                func = &roundUp_aa;
                break;
            case opTrunc:
                func = &trunc_aa;
                break;
            case opAtan2:
                func = &atan2_aa;
                break;
            case opHypot:
                func = &hypot_aa;
                break;
            case opHypotx:
                func = &hypotx_aa;
                break;
            case opPow:
                func = &pow_aa_nova;
                break;
            case opRing1:
                func = &ring1_aa_nova;
                break;
            case opRing2:
                func = &ring2_aa_nova;
                break;
            case opRing3:
                func = &ring3_aa_nova;
                break;
            case opRing4:
                func = &ring4_aa_nova;
                break;
            case opDifSqr:
                func = &difsqr_aa_nova;
                break;
            case opSumSqr:
                func = &sumsqr_aa_nova;
                break;
            case opSqrSum:
                func = &sqrsum_aa_nova;
                break;
            case opSqrDif:
                func = &sqrdif_aa_nova;
                break;
            case opAbsDif:
                func = &absdif_aa;
                break;
            case opThresh:
                func = &thresh_aa;
                break;
            case opAMClip:
                func = &amclip_aa;
                break;
            case opScaleNeg:
                func = &scaleneg_aa;
                break;
            case opClip2:
                func = &clip2_aa_nova;
                break;
            case opFold2:
                func = &fold2_aa;
                break;
            case opWrap2:
                func = &wrap2_aa;
                break;
            case opExcess:
                func = &excess_aa;
                break;
            case opRandRange:
                func = &rrand_aa;
                break;
            case opExpRandRange:
                func = &exprand_aa;
                break;
            case opFirstArg:
                func = &firstarg_aa_nova;
                break;
            // case opSecondArg : func = &secondarg_aa_nova; break;
            default:
                func = &add_aa;
                break;
            }
            break;
        case calc_BufRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ak_nova;
                break;
            case opSub:
                func = &sub_ak_nova;
                break;
            case opMul:
                func = &mul_ak_nova;
                break;
            case opFDiv:
                func = &div_ak_nova;
                break;
            case opIDiv:
                func = &idiv_ak;
                break;
            case opMod:
                func = &mod_ak;
                break;
            case opEQ:
                func = &eq_ak_nova;
                break;
            case opNE:
                func = &neq_ak_nova;
                break;
            case opLT:
                func = &lt_ak_nova;
                break;
            case opGT:
                func = &gt_ak_nova;
                break;
            case opLE:
                func = &le_ak_nova;
                break;
            case opGE:
                func = &ge_ak_nova;
                break;
            case opMin:
                func = &min_ak_nova;
                break;
            case opMax:
                func = &max_ak_nova;
                break;
            case opBitAnd:
                func = &and_ak;
                break;
            case opBitOr:
                func = &or_ak;
                break;
            case opBitXor:
                func = &xor_ak;
                break;
            case opShiftRight:
                func = &rightShift_ak;
                break;
            case opShiftLeft:
                func = &leftShift_ak;
                break;
            case opLCM:
                func = &lcm_ak;
                break;
            case opGCD:
                func = &gcd_ak;
                break;
            case opRound:
                func = &round_ak;
                break;
            case opRoundUp:
                func = &roundUp_ak;
                break;
            case opTrunc:
                func = &trunc_ak;
                break;
            case opAtan2:
                func = &atan2_ak;
                break;
            case opHypot:
                func = &hypot_ak;
                break;
            case opHypotx:
                func = &hypotx_ak;
                break;
            case opPow:
                func = &pow_ak_nova;
                break;
            case opRing1:
                func = &ring1_ak;
                break;
            case opRing2:
                func = &ring2_ak;
                break;
            case opRing3:
                func = &ring3_ak;
                break;
            case opRing4:
                func = &ring4_ak;
                break;
            case opDifSqr:
                func = &difsqr_ak_nova;
                break;
            case opSumSqr:
                func = &sumsqr_ak_nova;
                break;
            case opSqrSum:
                func = &sqrsum_ak_nova;
                break;
            case opSqrDif:
                func = &sqrdif_ak_nova;
                break;
            case opAbsDif:
                func = &absdif_ak;
                break;
            case opThresh:
                func = &thresh_ak;
                break;
            case opAMClip:
                func = &amclip_ak;
                break;
            case opScaleNeg:
                func = &scaleneg_ak;
                break;
            case opClip2:
                func = &clip2_ak_nova;
                break;
            case opFold2:
                func = &fold2_ak;
                break;
            case opWrap2:
                func = &wrap2_ak;
                break;
            case opExcess:
                func = &excess_ak;
                break;
            case opRandRange:
                func = &rrand_ak;
                break;
            case opExpRandRange:
                func = &exprand_ak;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ak;
                break;
            }
            break;
        case calc_ScalarRate:
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ai_nova;
                break;
            case opSub:
                func = &sub_ai_nova;
                break;
            case opMul:
                func = &mul_ai_nova;
                break;
            case opFDiv:
                func = &div_ai_nova;
                break;
            case opIDiv:
                func = &idiv_ai;
                break;
            case opMod:
                func = &mod_ai;
                break;
            case opEQ:
                func = &eq_ai_nova;
                break;
            case opNE:
                func = &neq_ai_nova;
                break;
            case opLT:
                func = &lt_ai_nova;
                break;
            case opGT:
                func = &gt_ai_nova;
                break;
            case opLE:
                func = &le_ai_nova;
                break;
            case opGE:
                func = &ge_ai_nova;
                break;
            case opMin:
                func = &min_ai_nova;
                break;
            case opMax:
                func = &max_ai_nova;
                break;
            case opBitAnd:
                func = &and_ai;
                break;
            case opBitOr:
                func = &or_ai;
                break;
            case opBitXor:
                func = &xor_ai;
                break;
            case opShiftRight:
                func = &rightShift_ai;
                break;
            case opShiftLeft:
                func = &leftShift_ai;
                break;
            case opLCM:
                func = &lcm_ai;
                break;
            case opGCD:
                func = &gcd_ai;
                break;
            case opRound:
                func = &round_ai;
                break;
            case opRoundUp:
                func = &roundUp_ai;
                break;
            case opTrunc:
                func = &trunc_ai;
                break;
            case opAtan2:
                func = &atan2_ai;
                break;
            case opHypot:
                func = &hypot_ai;
                break;
            case opHypotx:
                func = &hypotx_ai;
                break;
            case opPow:
                func = &pow_ai_nova;
                break;
            case opRing1:
                func = &ring1_ai_nova;
                break;
            case opRing2:
                func = &ring2_ai_nova;
                break;
            case opRing3:
                func = &ring3_ai_nova;
                break;
            case opRing4:
                func = &ring4_ai_nova;
                break;
            case opDifSqr:
                func = &difsqr_ai_nova;
                break;
            case opSumSqr:
                func = &sumsqr_ai_nova;
                break;
            case opSqrSum:
                func = &sqrsum_ai_nova;
                break;
            case opSqrDif:
                func = &sqrdif_ai_nova;
                break;
            case opAbsDif:
                func = &absdif_ai;
                break;
            case opThresh:
                func = &thresh_ai;
                break;
            case opAMClip:
                func = &amclip_ai;
                break;
            case opScaleNeg:
                func = &scaleneg_ai;
                break;
            case opClip2:
                func = &clip2_ai_nova;
                break;
            case opFold2:
                func = &fold2_ai;
                break;
            case opWrap2:
                func = &wrap2_ai;
                break;
            case opExcess:
                func = &excess_ai;
                break;
            case opRandRange:
                func = &rrand_ai;
                break;
            case opExpRandRange:
                func = &exprand_ai;
                break;
            case opFirstArg:
                func = &firstarg_aa;
                break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ai;
                break;
            }
        }
        break;
    case calc_BufRate:
        if (rateB == calc_FullRate) {
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ka_nova;
                break;
            case opSub:
                func = &sub_ka_nova;
                break;
            case opMul:
                func = &mul_ka_nova;
                break;
            case opFDiv:
                func = &div_ka_nova;
                break;
            case opIDiv:
                func = &idiv_ka;
                break;
            case opMod:
                func = &mod_ka;
                break;
            case opEQ:
                func = &eq_ka_nova;
                break;
            case opNE:
                func = &neq_ka_nova;
                break;
            case opLT:
                func = &lt_ka_nova;
                break;
            case opGT:
                func = &gt_ka_nova;
                break;
            case opLE:
                func = &le_ka_nova;
                break;
            case opGE:
                func = &ge_ka_nova;
                break;
            case opMin:
                func = &min_ka_nova;
                break;
            case opMax:
                func = &max_ka_nova;
                break;
            case opBitAnd:
                func = &and_ka;
                break;
            case opBitOr:
                func = &or_ka;
                break;
            case opBitXor:
                func = &xor_ka;
                break;
            case opShiftRight:
                func = &rightShift_ka;
                break;
            case opShiftLeft:
                func = &leftShift_ka;
                break;
            case opLCM:
                func = &lcm_ka;
                break;
            case opGCD:
                func = &gcd_ka;
                break;
            case opRound:
                func = &round_ka;
                break;
            case opRoundUp:
                func = &roundUp_ka;
                break;
            case opTrunc:
                func = &trunc_ka;
                break;
            case opAtan2:
                func = &atan2_ka;
                break;
            case opHypot:
                func = &hypot_ka;
                break;
            case opHypotx:
                func = &hypotx_ka;
                break;
            case opPow:
                func = &pow_ka_nova;
                break;
            case opRing1:
                func = &ring1_ka;
                break;
            case opRing2:
                func = &ring2_ka;
                break;
            case opRing3:
                func = &ring3_ka;
                break;
            case opRing4:
                func = &ring4_ka;
                break;
            case opDifSqr:
                func = &difsqr_ka_nova;
                break;
            case opSumSqr:
                func = &sumsqr_ka_nova;
                break;
            case opSqrSum:
                func = &sqrsum_ka_nova;
                break;
            case opSqrDif:
                func = &sqrdif_ka_nova;
                break;
            case opAbsDif:
                func = &absdif_ka;
                break;
            case opThresh:
                func = &thresh_ka;
                break;
            case opAMClip:
                func = &amclip_ka;
                break;
            case opScaleNeg:
                func = &scaleneg_ka;
                break;
            case opClip2:
                func = &clip2_ka_nova;
                break;
            case opFold2:
                func = &fold2_ka;
                break;
            case opWrap2:
                func = &wrap2_ka;
                break;
            case opExcess:
                func = &excess_ka;
                break;
            case opRandRange:
                func = &rrand_ka;
                break;
            case opExpRandRange:
                func = &exprand_ka;
                break;
            // case opFirstArg : func = &firstarg_aa; break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ka;
                break;
            }
        } else {
            // this should have been caught by mBufLength == 1
            func = &zero_aa;
        }
        break;
    case calc_ScalarRate:
        if (rateB == calc_FullRate) {
            switch (unit->mSpecialIndex) {
            // case opSilence2 : func = &zero_aa; break;
            case opAdd:
                func = &add_ia_nova;
                break;
            case opSub:
                func = &sub_ia_nova;
                break;
            case opMul:
                func = &mul_ia_nova;
                break;
            case opFDiv:
                func = &div_ia_nova;
                break;
            case opIDiv:
                func = &idiv_ia;
                break;
            case opMod:
                func = &mod_ia;
                break;
            case opEQ:
                func = &eq_ia_nova;
                break;
            case opNE:
                func = &neq_ia_nova;
                break;
            case opLT:
                func = &lt_ia_nova;
                break;
            case opGT:
                func = &gt_ia_nova;
                break;
            case opLE:
                func = &le_ia_nova;
                break;
            case opGE:
                func = &ge_ia_nova;
                break;
            case opMin:
                func = &min_ia_nova;
                break;
            case opMax:
                func = &max_ia_nova;
                break;
            case opBitAnd:
                func = &and_ia;
                break;
            case opBitOr:
                func = &or_ia;
                break;
            case opBitXor:
                func = &xor_ia;
                break;
            case opShiftRight:
                func = &rightShift_ia;
                break;
            case opShiftLeft:
                func = &leftShift_ia;
                break;
            case opLCM:
                func = &lcm_ia;
                break;
            case opGCD:
                func = &gcd_ia;
                break;
            case opRound:
                func = &round_ia;
                break;
            case opRoundUp:
                func = &roundUp_ia;
                break;
            case opTrunc:
                func = &trunc_ia;
                break;
            case opAtan2:
                func = &atan2_ia;
                break;
            case opHypot:
                func = &hypot_ia;
                break;
            case opHypotx:
                func = &hypotx_ia;
                break;
            case opPow:
                func = &pow_ia_nova;
                break;
            case opRing1:
                func = &ring1_ia_nova;
                break;
            case opRing2:
                func = &ring2_ia_nova;
                break;
            case opRing3:
                func = &ring3_ia_nova;
                break;
            case opRing4:
                func = &ring4_ia_nova;
                break;
            case opDifSqr:
                func = &difsqr_ia_nova;
                break;
            case opSumSqr:
                func = &sumsqr_ia_nova;
                break;
            case opSqrSum:
                func = &sqrsum_ia_nova;
                break;
            case opSqrDif:
                func = &sqrdif_ia_nova;
                break;
            case opAbsDif:
                func = &absdif_ia;
                break;
            case opThresh:
                func = &thresh_ia;
                break;
            case opAMClip:
                func = &amclip_ia;
                break;
            case opScaleNeg:
                func = &scaleneg_ia;
                break;
            case opClip2:
                func = &clip2_ia_nova;
                break;
            case opFold2:
                func = &fold2_ia;
                break;
            case opWrap2:
                func = &wrap2_ia;
                break;
            case opExcess:
                func = &excess_ia;
                break;
            case opRandRange:
                func = &rrand_ia;
                break;
            case opExpRandRange:
                func = &rrand_ia;
                break;
            // case opFirstArg : func = &firstarg_aa; break;
            // case opSecondArg : func = &secondarg_aa; break;
            default:
                func = &add_ia;
                break;
            }
        } else {
            // this should have been caught by mBufLength == 1
            func = &zero_aa;
        }
        break;
    }

    return func;
}

#endif

bool ChooseOperatorFunc(SuperBinaryOpUGen* unit) {
    //Print("->ChooseOperatorFunc %d %d\n", unit->mSpecialIndex, BUFLENGTH);
    BinaryOpFunc func = &zero_aa;
    bool ret = false;

    if (BUFLENGTH == 1) {
        if (unit->mCalcRate == calc_DemandRate) {
            func = ChooseDemandFunc(unit);
        } else {
            func = ChooseOneSampleFunc(unit);
        }
#if defined(NOVA_SIMD)
    } else if (boost::alignment::is_aligned(BUFLENGTH, 16)) {
        /* select normal function for initialization */
        func = ChooseNormalFunc(unit);
        func(unit, 1);

        /* select simd function */
        func = ChooseNovaSimdFunc(unit);
        ret = true;
#endif
    } else {
        func = ChooseNormalFunc(unit);
    }
    unit->mCalcFunc = (UnitCalcFunc)func;
    // Print("<-ChooseOperatorFunc %p\n", func);
    // Print("calc %d\n", unit->mCalcRate);
    return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////


PluginLoad(BinaryOp) {
    ft = inTable;

    DefineSimpleUnit(SuperBinaryOpUGen);
}
