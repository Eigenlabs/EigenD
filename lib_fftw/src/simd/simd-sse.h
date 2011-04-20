/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef FFTW_SINGLE
#error "SSE only works in single precision"
#endif

#define VL 2            /* SIMD complex vector length */
#define ALIGNMENT 8     /* alignment for LD/ST */
#define ALIGNMENTA 16   /* alignment for LDA/STA */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#define RIGHT_CPU X(have_sse)
extern int RIGHT_CPU(void);

/* gcc compiles the following code only when __SSE__ is defined */
#if defined(__SSE__) || !defined(__GNUC__)

/* some versions of glibc's sys/cdefs.h define __inline to be empty,
   which is wrong because xmmintrin.h defines several inline
   procedures */
#undef __inline

#include <xmmintrin.h>

typedef __m128 V;
#define VADD _mm_add_ps
#define VSUB _mm_sub_ps
#define VMUL _mm_mul_ps
#define VXOR _mm_xor_ps
#define SHUFPS _mm_shuffle_ps
#define STOREH(addr, val) _mm_storeh_pi((__m64 *)(addr), val)
#define STOREL(addr, val) _mm_storel_pi((__m64 *)(addr), val)
#define UNPCKH _mm_unpackhi_ps
#define UNPCKL _mm_unpacklo_ps

#ifdef __GNUC__
#  define DVK(var, val) const V var = __extension__ ({		\
     static const union fvec _var = { {val, val, val, val} };	\
     _var.v;							\
   })
#  define LDK(x) x

  /* we use inline asm because gcc generates slow code for
     _mm_loadh_pi().  gcc insists upon having an existing variable for
     VAL, which is however never used.  Thus, it generates code to move
     values in and out the variable.  Worse still, gcc-4.0 stores VAL on
     the stack, causing valgrind to complain about uninitialized reads.
  */   

  static inline V LD(const R *x, INT ivs, const R *aligned_like)
  {
       V var;
       (void)aligned_like; /* UNUSED */
       __asm__("movlps %1, %0\n\tmovhps %2, %0"
	       : "=x"(var) : "m"(x[0]), "m"(x[ivs]));
       return var;
  }

#else

# define DVK(var, val) const R var = K(val)
# define LDK(x) _mm_set_ps1(x)
# define LOADH(addr, val) _mm_loadh_pi(val, (const __m64 *)(addr))
# define LOADL0(addr, val) _mm_loadl_pi(val, (const __m64 *)(addr))

  static inline V LD(const R *x, INT ivs, const R *aligned_like)
  {
       V var;
       (void)aligned_like; /* UNUSED */
       var = LOADL0(x, var);
       var = LOADH(x + ivs, var);
       return var;
  }

#endif

union fvec {
     R f[4];
     V v;
};

union uvec {
     unsigned u[4];
     V v;
};

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)

#define SHUFVAL(fp0,fp1,fp2,fp3) \
   (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))


static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return *(const V *)x;
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     /* WARNING: the extra_iter hack depends upon STOREL occurring
	after STOREH */
     STOREH(x + ovs, v);
     STOREL(x, v);
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     *(V *)x = v;
}

#if 0
/* this should be faster but it isn't. */
static inline void STN2(R *x, V v0, V v1, INT ovs)
{
     STA(x, SHUFPS(v0, v1, SHUFVAL(0, 1, 0, 1)), ovs, 0);
     STA(x + ovs, SHUFPS(v0, v1, SHUFVAL(2, 3, 2, 3)), ovs, 0);
}
#endif
#define STM2 ST
#define STN2(x, v0, v1, ovs) /* nop */

#define STM4(x, v, ovs, aligned_like) /* no-op */

#ifdef VISUAL_CXX_DOES_NOT_SUCK
static inline void STN4(R *x, V v0, V v1, V v2, V v3, INT ovs)
{
     V x0, x1, x2, x3;
     x0 = UNPCKL(v0, v2);
     x1 = UNPCKH(v0, v2);
     x2 = UNPCKL(v1, v3);
     x3 = UNPCKH(v1, v3);
     STA(x, UNPCKL(x0, x2), 0, 0);
     STA(x + ovs, UNPCKH(x0, x2), 0, 0);
     STA(x + 2 * ovs, UNPCKL(x1, x3), 0, 0);
     STA(x + 3 * ovs, UNPCKH(x1, x3), 0, 0);
}
#else /* Visual C++ sucks */

/*
  Straight from the mouth of the horse:

     We "reserved" the possibility of aligning arguments with 
     __declspec(align(X)) passed by value by issuing this error. 

     The first 3 parameters of type __m64 (or other MMX types) are
     passed in registers.  The rest would be passed on the stack.  We
     decided aligning the stack was wasteful, especially for __m128
     parameters.  Also, we thought it would be infrequent that people
     would want to pass more than 3 by value.

     If we didn't issue an error, we would have to binary compatibility
     in the future if we decided to align the arguments.


     Hope that explains it. 
     -- 
     Jason Shirk, Visual C++ Compiler Team 
     This posting is provided AS IS with no warranties, and confers no rights
*/

#define STN4(x, v0, v1, v2, v3, ovs)			\
{							\
     V xxx0, xxx1, xxx2, xxx3;				\
     xxx0 = UNPCKL(v0, v2);				\
     xxx1 = UNPCKH(v0, v2);				\
     xxx2 = UNPCKL(v1, v3);				\
     xxx3 = UNPCKH(v1, v3);				\
     STA(x, UNPCKL(xxx0, xxx2), 0, 0);			\
     STA(x + ovs, UNPCKH(xxx0, xxx2), 0, 0);		\
     STA(x + 2 * ovs, UNPCKL(xxx1, xxx3), 0, 0);	\
     STA(x + 3 * ovs, UNPCKH(xxx1, xxx3), 0, 0);	\
}
#endif

static inline V FLIP_RI(V x)
{
     return SHUFPS(x, x, SHUFVAL(1, 0, 3, 2));
}

extern const union uvec X(sse_pmpm);
static inline V VCONJ(V x)
{
     return VXOR(X(sse_pmpm).v, x);
}

static inline V VBYI(V x)
{
     return FLIP_RI(VCONJ(x));
}

static inline V VZMUL(V tx, V sr)
{
     V tr = SHUFPS(tx, tx, SHUFVAL(0, 0, 2, 2));
     V ti = SHUFPS(tx, tx, SHUFVAL(1, 1, 3, 3));
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static inline V VZMULJ(V tx, V sr)
{
     V tr = SHUFPS(tx, tx, SHUFVAL(0, 0, 2, 2));
     V ti = SHUFPS(tx, tx, SHUFVAL(1, 1, 3, 3));
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

static inline V VZMULI(V tx, V sr)
{
     V tr = SHUFPS(tx, tx, SHUFVAL(0, 0, 2, 2));
     V ti = SHUFPS(tx, tx, SHUFVAL(1, 1, 3, 3));
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VSUB(VMUL(tr, sr), ti);
}

static inline V VZMULIJ(V tx, V sr)
{
     V tr = SHUFPS(tx, tx, SHUFVAL(0, 0, 2, 2));
     V ti = SHUFPS(tx, tx, SHUFVAL(1, 1, 3, 3));
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VADD(VMUL(tr, sr), ti);
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

/* twiddle storage #1: compact, slower */
#define VTW1(v,x)  \
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
#define TWVL1 (VL)

static inline V BYTW1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static inline V BYTWJ1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(v,x)							\
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},	\
  {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#define TWVL2 (2 * VL)

static inline V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VADD(VMUL(tr, sr), VMUL(ti, si));
}

static inline V BYTWJ2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VSUB(VMUL(tr, sr), VMUL(ti, si));
}

/* twiddle storage #3 */
#define VTW3(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#define TWVL3 (VL)

/* twiddle storage for split arrays */
#define VTWS(v,x)							\
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x},	\
  {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}	
#define TWVLS (2 * VL)

#endif /* __SSE__ */
