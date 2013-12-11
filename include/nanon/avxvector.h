/*
	nanon : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#ifndef __LIB_NANON_AVX_VECTOR_H__
#define __LIB_NANON_AVX_VECTOR_H__

#include "vector.h"
#include "simdsupport.h"

#if !defined(NANON_USE_AVX)
	#error "avxvector.h requires the support of AVX"
#endif

#include <immintrin.h>

NANON_NAMESPACE_BEGIN

/*!
	AVX optimized 4D vector.
	Specialized version of TVec4 optimized by AVX.
*/
template <>
struct NANON_ALIGN_16 TVec4<double>
{

	union
	{
		__m256d v;
		struct { double x, y, z, w; };
		struct { double r, g, b, a; };
		struct { double s, t, p, q; };
	};
	
	NANON_FORCE_INLINE TVec4();
	NANON_FORCE_INLINE TVec4(const Vec4d& v);
	NANON_FORCE_INLINE TVec4(__m256d v);
	NANON_FORCE_INLINE TVec4(double x, double y, double z, double w);
	NANON_FORCE_INLINE double operator[](int i) const;
	NANON_FORCE_INLINE Vec4d& operator=(const Vec4d& v);

};

NANON_NAMESPACE_END

#include "avxvector.inl"

#endif // __LIB_NANON_AVX_VECTOR_H__