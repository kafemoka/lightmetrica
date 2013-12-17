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

#ifndef __LIB_NANON_MATH_BASIC_H__
#define __LIB_NANON_MATH_BASIC_H__

#include "math.common.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

template <typename T> NANON_FORCE_INLINE T Radians(const T& v);
template <typename T> NANON_FORCE_INLINE T Degrees(const T& v);
template <typename T> NANON_FORCE_INLINE T Cos(const T& v);
template <typename T> NANON_FORCE_INLINE T Sin(const T& v);
template <typename T> NANON_FORCE_INLINE T Tan(const T& v);
template <typename T> NANON_FORCE_INLINE T Abs(const T& v);
template <typename T> NANON_FORCE_INLINE T Sqrt(const T& v);
template <typename T> NANON_FORCE_INLINE T Min(const T& v1, const T& v2);
template <typename T> NANON_FORCE_INLINE T Max(const T& v1, const T& v2);

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END

#include "math.basic.inl"

#endif // __LIB_NANON_MATH_BASIC_H__