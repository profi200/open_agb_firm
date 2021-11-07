#pragma once

// Based on: https://github.com/ARM-software/CMSIS_5/blob/master/CMSIS/Core/Include/cmsis_gcc.h

#include "types.h"


// u32 result, u32 op1.
#define MAKE_INTR_U32_1OP(isVolatile, inst)                            \
ALWAYS_INLINE u32 __##inst(u32 op1)                                    \
{                                                                      \
	u32 res;                                                           \
	if(isVolatile == 1)                                                \
		__asm__ volatile(#inst " %0, %1" : "=r" (res) : "r" (op1) : ); \
	else                                                               \
		__asm__(#inst " %0, %1" : "=r" (res) : "r" (op1) : );          \
	return res;                                                        \
}

// u32 result, u32 op1, u32 op2.
#define MAKE_INTR_U32_2OPS(isVolatile, inst)                                          \
ALWAYS_INLINE u32 __##inst(u32 op1, u32 op2)                                          \
{                                                                                     \
	u32 res;                                                                          \
	if(isVolatile == 1)                                                               \
		__asm__ volatile(#inst " %0, %1, %2" : "=r" (res) : "r" (op1), "r" (op2) : ); \
	else                                                                              \
		__asm__(#inst " %0, %1, %2" : "=r" (res) : "r" (op1), "r" (op2) : );          \
	return res;                                                                       \
}

// s32 result, s32 op1, s32 op2.
#define MAKE_INTR_S32_2OPS(isVolatile, inst)                                          \
ALWAYS_INLINE s32 __##inst(s32 op1, s32 op2)                                          \
{                                                                                     \
	s32 res;                                                                          \
	if(isVolatile == 1)                                                               \
		__asm__ volatile(#inst " %0, %1, %2" : "=r" (res) : "r" (op1), "r" (op2) : ); \
	else                                                                              \
		__asm__(#inst " %0, %1, %2" : "=r" (res) : "r" (op1), "r" (op2) : );          \
	return res;                                                                       \
}

// u32 result, u32 op1, u32 op2, u32 op3.
#define MAKE_INTR_U32_3OPS(isVolatile, inst)                                                         \
ALWAYS_INLINE u32 __##inst(u32 op1, u32 op2, u32 op3)                                                \
{                                                                                                    \
	u32 res;                                                                                         \
	if(isVolatile == 1)                                                                              \
		__asm__ volatile(#inst " %0, %1, %2, %3" : "=r" (res) : "r" (op1), "r" (op2), "r" (op3) : ); \
	else                                                                                             \
		__asm__(#inst " %0, %1, %2, %3" : "=r" (res) : "r" (op1), "r" (op2), "r" (op3) : );          \
	return res;                                                                                      \
}

// s32 result, s32 op1, s32 op2, s32 op3.
#define MAKE_INTR_S32_3OPS(isVolatile, inst)                                                         \
ALWAYS_INLINE s32 __##inst(s32 op1, s32 op2, s32 op3)                                                \
{                                                                                                    \
	s32 res;                                                                                         \
	if(isVolatile == 1)                                                                              \
		__asm__ volatile(#inst " %0, %1, %2, %3" : "=r" (res) : "r" (op1), "r" (op2), "r" (op3) : ); \
	else                                                                                             \
		__asm__(#inst " %0, %1, %2, %3" : "=r" (res) : "r" (op1), "r" (op2), "r" (op3) : );          \
	return res;                                                                                      \
}

#ifndef __ARMEB__ // Little endian
// Special instruction with shared u64 input and output.
// u64 result, u32 op1, u32 op2, u64 acc.
#define MAKE_INTR_U64_U32_U32_U64(isVolatile, inst)                                                                                             \
ALWAYS_INLINE u64 __##inst(u32 op1, u32 op2, u64 acc)                                                                                           \
{                                                                                                                                               \
	union                                                                                                                                       \
	{                                                                                                                                           \
		u32 r32[2];                                                                                                                             \
		u64 r64;                                                                                                                                \
	} r;                                                                                                                                        \
	r.r64 = acc;                                                                                                                                \
                                                                                                                                                \
	if(isVolatile == 1)                                                                                                                         \
		__asm__ volatile(#inst " %0, %1, %2, %3" : "=r" (r.r32[0]), "=r" (r.r32[1]) : "r" (op1), "r" (op2), "0" (r.r32[0]), "1" (r.r32[1]) : ); \
	else                                                                                                                                        \
		__asm__(#inst " %0, %1, %2, %3" : "=r" (r.r32[0]), "=r" (r.r32[1]) : "r" (op1), "r" (op2), "0" (r.r32[0]), "1" (r.r32[1]) : );          \
                                                                                                                                                \
	return r.r64;                                                                                                                               \
}

#else // Big endian

// Special instruction with shared u64 input and output.
// u64 result, u32 op1, u32 op2, u64 acc.
#define MAKE_INTR_U64_U32_U32_U64(isVolatile, inst)                                                                                             \
ALWAYS_INLINE u64 __##inst(u32 op1, u32 op2, u64 acc)                                                                                           \
{                                                                                                                                               \
	union                                                                                                                                       \
	{                                                                                                                                           \
		u32 r32[2];                                                                                                                             \
		u64 r64;                                                                                                                                \
	} r;                                                                                                                                        \
	r.r64 = acc;                                                                                                                                \
                                                                                                                                                \
	if(isVolatile == 1)                                                                                                                         \
		__asm__ volatile(#inst " %0, %1, %2, %3" : "=r" (r.r32[1]), "=r" (r.r32[0]) : "r" (op1), "r" (op2), "0" (r.r32[1]), "1" (r.r32[0]) : ); \
	else                                                                                                                                        \
		__asm__(#inst " %0, %1, %2, %3" : "=r" (r.r32[1]), "=r" (r.r32[0]) : "r" (op1), "r" (op2), "0" (r.r32[1]), "1" (r.r32[0]) : );          \
                                                                                                                                                \
	return r.r64;                                                                                                                               \
}
#endif // #ifndef __ARMEB__



// Pack Halfword Bottom Top.
#define __pkhbt(op1, op2, sh)                                                               \
({                                                                                          \
	u32 __res;                                                                              \
	__asm__("pkhbt %0, %1, %2, lsl %3" : "=r" (__res) : "r" (op1), "r" (op2), "I" (sh) : ); \
	__res;                                                                                  \
})

// Pack Halfword Top Bottom.
#define __pkhtb(op1, op2, sh)                                                                     \
({                                                                                                \
	u32 __res, __sh = (sh);                                                                       \
	if(__sh == 0)                                                                                 \
		__asm__("pkhtb %0, %1, %2" : "=r" (__res) : "r" (op1), "r" (op2) : );                     \
	else                                                                                          \
		__asm__("pkhtb %0, %1, %2, asr %3" : "=r" (__res) : "r" (op1), "r" (op2), "I" (__sh) : ); \
	__res;                                                                                        \
})

MAKE_INTR_S32_2OPS(1, qadd)            // Signed saturating addition.
MAKE_INTR_U32_2OPS(0, qadd8)           // Signed saturating parallel byte-wise addition.
MAKE_INTR_U32_2OPS(0, qadd16)          // Signed saturating parallel halfword-wise addition.
MAKE_INTR_U32_2OPS(0, qasx)            // Signed saturating parallel add and subtract halfwords with exchange.
MAKE_INTR_S32_2OPS(1, qdadd)           // Signed saturating Double and Add.
MAKE_INTR_S32_2OPS(1, qdsub)           // Signed saturating Double and Subtract.
MAKE_INTR_U32_2OPS(0, qsax)            // Signed saturating parallel subtract and add halfwords with exchange.
MAKE_INTR_S32_2OPS(1, qsub)            // Signed saturating Subtract.
MAKE_INTR_U32_2OPS(0, qsub8)           // Signed saturating parallel byte-wise subtraction.
MAKE_INTR_U32_2OPS(0, qsub16)          // Signed saturating parallel halfword-wise subtraction.
MAKE_INTR_U32_1OP(0, rev)              // Reverse the byte order in a word.
MAKE_INTR_U32_1OP(0, rev16)            // Reverse the byte order in each halfword independently.
MAKE_INTR_U32_1OP(0, revsh)            // Reverse the byte order in the bottom halfword, and sign extend to 32 bits.
MAKE_INTR_U32_2OPS(1, sadd8)           // Signed parallel byte-wise addition.
MAKE_INTR_U32_2OPS(1, sadd16)          // Signed parallel halfword-wise addition.
MAKE_INTR_U32_2OPS(1, sasx)            // Signed parallel add and subtract halfwords with exchange.
MAKE_INTR_U32_2OPS(1, sel)             // Select bytes from each operand according to the state of the APSR GE flags.
MAKE_INTR_U32_2OPS(0, shadd8)          // Signed halving parallel byte-wise addition.
MAKE_INTR_U32_2OPS(0, shadd16)         // Signed halving parallel halfword-wise addition.
MAKE_INTR_U32_2OPS(0, shasx)           // Signed halving parallel add and subtract halfwords with exchange.
MAKE_INTR_U32_2OPS(0, shsax)           // Signed halving parallel subtract and add halfwords with exchange.
MAKE_INTR_U32_2OPS(0, shsub8)          // Signed halving parallel byte-wise subtraction.
MAKE_INTR_U32_2OPS(0, shsub16)         // Signed halving parallel halfword-wise subtraction.
MAKE_INTR_U32_3OPS(1, smlabb)          // Signed Multiply Accumulate, with 16-bit operands (bottom, bottom) and a 32-bit result and accumulator.
MAKE_INTR_U32_3OPS(1, smlabt)          // Signed Multiply Accumulate, with 16-bit operands (bottom, top) and a 32-bit result and accumulator.
MAKE_INTR_U32_3OPS(1, smlatb)          // Signed Multiply Accumulate, with 16-bit operands (top, bottom) and a 32-bit result and accumulator.
MAKE_INTR_U32_3OPS(1, smlatt)          // Signed Multiply Accumulate, with 16-bit operands (top, top) and a 32-bit result and accumulator.
MAKE_INTR_U32_3OPS(0, smlad)           // Dual 16-bit Signed Multiply with Addition of products and 32-bit accumulation.
MAKE_INTR_U32_3OPS(0, smladx)          // Dual 16-bit Signed exchange Multiply with Addition of products and 32-bit accumulation.
MAKE_INTR_U64_U32_U32_U64(0, smlal)    // Signed Long Multiply, with optional Accumulate, with 32-bit operands, and 64-bit result and accumulator.
MAKE_INTR_U64_U32_U32_U64(0, smlald)   // Dual 16-bit Signed Multiply with Addition of products and 64-bit Accumulation.
MAKE_INTR_U64_U32_U32_U64(0, smlaldx)  // Dual 16-bit Signed exchange Multiply with Addition of products and 64-bit Accumulation.
MAKE_INTR_U64_U32_U32_U64(0, smlalbb)  // Signed Multiply-Accumulate with 16-bit operands (bottom, bottom) and a 64-bit accumulator.
MAKE_INTR_U64_U32_U32_U64(0, smlalbt)  // Signed Multiply-Accumulate with 16-bit operands (bottom, top) and a 64-bit accumulator.
MAKE_INTR_U64_U32_U32_U64(0, smlaltb)  // Signed Multiply-Accumulate with 16-bit operands (top, bottom) and a 64-bit accumulator.
MAKE_INTR_U64_U32_U32_U64(0, smlaltt)  // Signed Multiply-Accumulate with 16-bit operands (top, top) and a 64-bit accumulator.
MAKE_INTR_U32_3OPS(1, smlawb)          // Signed Multiply-Accumulate Wide, with one 32-bit operand and one 16-bit operand (bottom half), and a 32-bit accumulate value, providing the top 32 bits of the result.
MAKE_INTR_U32_3OPS(1, smlawt)          // Signed Multiply-Accumulate Wide, with one 32-bit operand and one 16-bit operand (top half), and a 32-bit accumulate value, providing the top 32 bits of the result.
MAKE_INTR_U32_3OPS(0, smlsd)           // Dual 16-bit Signed Multiply with Subtraction of products and 32-bit accumulation.
MAKE_INTR_U32_3OPS(0, smlsdx)          // Dual 16-bit Signed exchange Multiply with Subtraction of products and 32-bit accumulation.
MAKE_INTR_U64_U32_U32_U64(0, smlsld)   // Dual 16-bit Signed Multiply with Subtraction of products and 64-bit Accumulation.
MAKE_INTR_U64_U32_U32_U64(0, smlsldx)  // Dual 16-bit Signed exchange Multiply with Subtraction of products and 64-bit Accumulation.
MAKE_INTR_S32_3OPS(0, smmla)           // Signed Most significant word Multiply with Accumulation.
MAKE_INTR_S32_3OPS(0, smmlar)          // Signed Most significant word Multiply with Accumulation and rounding.
MAKE_INTR_S32_3OPS(0, smmls)           // Signed Most significant word Multiply with Subtraction.
MAKE_INTR_S32_2OPS(0, smmlsr)          // Signed Most significant word Multiply with Subtraction and rounding.
MAKE_INTR_S32_2OPS(0, smmul)           // Signed Most significant word Multiply.
MAKE_INTR_S32_2OPS(0, smmulr)          // Signed Most significant word Multiply and round.
MAKE_INTR_U32_2OPS(1, smuad)           // Dual 16-bit Signed Multiply with Addition of products.
MAKE_INTR_U32_2OPS(1, smuadx)          // Dual 16-bit Signed Multiply with Addition of products with exchange.
MAKE_INTR_U32_2OPS(0, smulbb)          // Signed Multiply, with 16-bit operands (bottom, bottom) and a 32-bit result.
MAKE_INTR_U32_2OPS(0, smulbt)          // Signed Multiply, with 16-bit operands (bottom, top) and a 32-bit result.
MAKE_INTR_U32_2OPS(0, smultb)          // Signed Multiply, with 16-bit operands (top, bottom) and a 32-bit result.
MAKE_INTR_U32_2OPS(0, smultt)          // Signed Multiply, with 16-bit operands (top, top) and a 32-bit result.
// TODO: smull  // Signed Long Multiply, with 32-bit operands and 64-bit result.
MAKE_INTR_U32_2OPS(0, smulwb)          // Signed Multiply Wide, with one 32-bit and one 16-bit operand (bottom half), providing the top 32 bits of the result.
MAKE_INTR_U32_2OPS(0, smulwt)          // Signed Multiply Wide, with one 32-bit and one 16-bit operand (top half), providing the top 32 bits of the result.
/* Doesn't affect any flags? */ MAKE_INTR_U32_2OPS(0, smusd)   // Dual 16-bit Signed Multiply with Subtraction of products.
/* Doesn't affect any flags? */ MAKE_INTR_U32_2OPS(0, smusdx)  // Dual 16-bit Signed Multiply with Subtraction of products with exchange.

// Signed Saturate to any bit position, with optional shift before saturating.
#define __ssat(op1, op2)                                                              \
({                                                                                    \
	s32 __res;                                                                        \
	__asm__ volatile("ssat %0, %1, %2" : "=r" (__res) : "I" (op1), "r" (op2) : "cc"); \
	__res;                                                                            \
})

// Parallel halfword Saturate.
#define __ssat16(op1, op2)                                                              \
({                                                                                      \
	u32 __res;                                                                          \
	__asm__ volatile("ssat16 %0, %1, %2" : "=r" (__res) : "I" (op1), "r" (op2) : "cc"); \
	__res;                                                                              \
})

MAKE_INTR_U32_2OPS(1, ssax)            // Signed parallel subtract and add halfwords with exchange.
MAKE_INTR_U32_2OPS(1, ssub8)           // Signed parallel byte-wise subtraction.
MAKE_INTR_U32_2OPS(1, ssub16)          // Signed parallel halfword-wise subtraction.
MAKE_INTR_U32_2OPS(0, sxtab)           // Sign extend Byte with Add, to extend an 8-bit value to a 32-bit value.
MAKE_INTR_U32_2OPS(0, sxtab16)         // Sign extend two Bytes with Add, to extend two 8-bit values to two 16-bit values.
MAKE_INTR_U32_2OPS(0, sxtah)           // Sign extend Halfword with Add, to extend a 16-bit value to a 32-bit value.
MAKE_INTR_U32_1OP(0, sxtb)             // Sign extend Byte, to extend an 8-bit value to a 32-bit value.
MAKE_INTR_U32_1OP(0, sxtb16)           // Sign extend two bytes.
MAKE_INTR_U32_1OP(0, sxth)             // Sign extend Halfword.
MAKE_INTR_U32_2OPS(1, uadd8)           // Unsigned parallel byte-wise addition.
MAKE_INTR_U32_2OPS(1, uadd16)          // Unsigned parallel halfword-wise addition.
MAKE_INTR_U32_2OPS(1, uasx)            // Unsigned parallel add and subtract halfwords with exchange.
MAKE_INTR_U32_2OPS(0, uhadd8)          // Unsigned halving parallel byte-wise addition.
MAKE_INTR_U32_2OPS(0, uhadd16)         // Unsigned halving parallel halfword-wise addition.
MAKE_INTR_U32_2OPS(0, uhasx)           // Unsigned halving parallel add and subtract halfwords with exchange.
MAKE_INTR_U32_2OPS(0, uhsax)           // Unsigned halving parallel subtract and add halfwords with exchange.
MAKE_INTR_U32_2OPS(0, uhsub8)          // Unsigned halving parallel byte-wise subtraction.
MAKE_INTR_U32_2OPS(0, uhsub16)         // Unsigned halving parallel halfword-wise subtraction.
MAKE_INTR_U32_2OPS(0, uqadd8)          // Unsigned saturating parallel byte-wise addition.
MAKE_INTR_U32_2OPS(0, uqadd16)         // Unsigned saturating parallel halfword-wise addition.
MAKE_INTR_U32_2OPS(0, uqasx)           // Unsigned saturating parallel add and subtract halfwords with exchange.
MAKE_INTR_U32_2OPS(0, uqsax)           // Unsigned saturating parallel subtract and add halfwords with exchange.
MAKE_INTR_U32_2OPS(0, uqsub8)          // Unsigned saturating parallel byte-wise subtraction.
MAKE_INTR_U32_2OPS(0, uqsub16)         // Unsigned saturating parallel halfword-wise subtraction.
MAKE_INTR_U32_2OPS(0, usad8)           // Unsigned Sum of Absolute Differences.
MAKE_INTR_U32_3OPS(0, usada8)          // Unsigned Sum of Absolute Differences and Accumulate.

// Unsigned Saturate to any bit position, with optional shift before saturating.
#define __usat(op1, op2)                                                              \
({                                                                                    \
	u32 __res;                                                                        \
	__asm__ volatile("usat %0, %1, %2" : "=r" (__res) : "I" (op1), "r" (op2) : "cc"); \
	__res;                                                                            \
})

// Parallel halfword Saturate.
#define __usat16(op1, op2)                                                              \
({                                                                                      \
	u32 __res;                                                                          \
	__asm__ volatile("usat16 %0, %1, %2" : "=r" (__res) : "I" (op1), "r" (op2) : "cc"); \
	__res;                                                                              \
})

MAKE_INTR_U32_2OPS(1, usax)            // Unsigned parallel subtract and add halfwords with exchange.
MAKE_INTR_U32_2OPS(1, usub8)           // Unsigned parallel byte-wise subtraction.
MAKE_INTR_U32_2OPS(1, usub16)          // Unsigned parallel halfword-wise subtraction.
//MAKE_INTR_U32_2OPS(0, uxtab)           // Zero extend Byte and Add.
MAKE_INTR_U32_2OPS(0, uxtab16)         // Zero extend two Bytes and Add.
//MAKE_INTR_U32_2OPS(0, uxtah)           // Zero extend Halfword and Add.
//MAKE_INTR_U32_1OP(0, uxtb)             // Zero extend Byte.
MAKE_INTR_U32_1OP(0, uxtb16)           // Zero extend two Bytes.
//MAKE_INTR_U32_1OP(0, uxth)             // Zero extend Halfword.


#undef MAKE_INTR_U32_1OP
#undef MAKE_INTR_U32_2OPS
#undef MAKE_INTR_U32_3OPS
