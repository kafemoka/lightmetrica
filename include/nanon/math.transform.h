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

#ifndef __LIB_NANON_MATH_TRANSFORM_H__
#define __LIB_NANON_MATH_TRANSFORM_H__

#include "math.matrix.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

// Transform
template <typename T> NANON_FORCE_INLINE TMat4<T> Translate(const TMat4<T>& m, const TVec3<T>& v);
template <typename T> NANON_FORCE_INLINE TMat4<T> Translate(const TVec3<T>& v);
template <typename T> NANON_FORCE_INLINE TMat4<T> Rotate(const TMat4<T>& m, T angle, const TVec3<T>& axis);
template <typename T> NANON_FORCE_INLINE TMat4<T> Rotate(T angle, const TVec3<T>& axis);
template <typename T> NANON_FORCE_INLINE TMat4<T> Scale(const TMat4<T>& m, const TVec3<T>& v);
template <typename T> NANON_FORCE_INLINE TMat4<T> Scale(const TVec3<T>& v);
template <typename T> NANON_FORCE_INLINE TMat4<T> LookAt(const TVec3<T>& eye, const TVec3<T>& center, const TVec3<T>& up);
template <typename T> NANON_FORCE_INLINE TMat4<T> Perspective(T fovy, T aspect, T zNear, T zFar);

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END

#include "math.transform.inl"

#endif // __LIB_NANON_MATH_TRANSFORM_H__