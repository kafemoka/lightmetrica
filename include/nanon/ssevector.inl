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

#include "ssevector.h"

NANON_NAMESPACE_BEGIN

NANON_FORCE_INLINE TVec4<float>::TVec4()
	: v(_mm_setzero_ps())
{

}

NANON_FORCE_INLINE TVec4<float>::TVec4(const Vec4f& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE TVec4<float>::TVec4(__m128 v)
	: v(v)
{

}

NANON_FORCE_INLINE TVec4<float>::TVec4(float x, float y, float z, float w)
	: v(_mm_set_ps(w, z, y, x))
{

}

NANON_NAMESPACE_END