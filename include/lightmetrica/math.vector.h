/*
	L I G H T  M E T R I C A

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

#pragma once
#ifndef __LIB_LIGHTMETRICA_VECTOR_H__
#define __LIB_LIGHTMETRICA_VECTOR_H__

#include "math.common.h"
#include "math.simd.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> struct TVec2;
template <typename T> struct TVec3;
template <typename T> struct TVec4;

/*!
	2D vector.
	Generic 2-dimensional vector.
	\tparam T Internal value type.
*/
template <typename T>
struct TVec2
{

	T x, y;

	LM_FORCE_INLINE TVec2();
	LM_FORCE_INLINE TVec2(const TVec2<T>& v);
	LM_FORCE_INLINE TVec2(const TVec3<T>& v);
	LM_FORCE_INLINE TVec2(const TVec4<T>& v);
	LM_FORCE_INLINE TVec2(const T& v);
	LM_FORCE_INLINE TVec2(const T& x, const T& y);
	LM_FORCE_INLINE T& operator[](int i);
	LM_FORCE_INLINE const T& operator[](int i) const;
	LM_FORCE_INLINE TVec2<T>& operator=(const TVec2<T>& v);
	LM_FORCE_INLINE TVec2<T>& operator+=(const TVec2<T>& v);
	LM_FORCE_INLINE TVec2<T>& operator-=(const TVec2<T>& v);
	LM_FORCE_INLINE TVec2<T>& operator*=(const TVec2<T>& v);
	LM_FORCE_INLINE TVec2<T>& operator*=(const T& s);
	LM_FORCE_INLINE TVec2<T>& operator/=(const TVec2<T>& v);
	LM_FORCE_INLINE TVec2<T>& operator/=(const T& s);

};

template <typename T> LM_FORCE_INLINE TVec2<T> operator+(const TVec2<T>& v1, const TVec2<T>& v2);
template <typename T> LM_FORCE_INLINE TVec2<T> operator-(const TVec2<T>& v1, const TVec2<T>& v2);
template <typename T> LM_FORCE_INLINE TVec2<T> operator*(const TVec2<T>& v, const T& s);
template <typename T> LM_FORCE_INLINE TVec2<T> operator*(const T& s, const TVec2<T>& v);
template <typename T> LM_FORCE_INLINE TVec2<T> operator*(const TVec2<T>& v1, const TVec2<T>& v2);
template <typename T> LM_FORCE_INLINE TVec2<T> operator/(const TVec2<T>& v, const T& s);
template <typename T> LM_FORCE_INLINE TVec2<T> operator/(const TVec2<T>& v1, const TVec2<T>& v2);
template <typename T> LM_FORCE_INLINE TVec2<T> operator-(const TVec2<T>& v);
template <typename T> LM_FORCE_INLINE bool operator==(const TVec2<T>& v1, const TVec2<T>& v2);
template <typename T> LM_FORCE_INLINE bool operator!=(const TVec2<T>& v1, const TVec2<T>& v2);

template <typename T> LM_FORCE_INLINE T Length(const TVec2<T>& v);
template <typename T> LM_FORCE_INLINE T Length2(const TVec2<T>& v);
template <typename T> LM_FORCE_INLINE TVec2<T> Normalize(const TVec2<T>& v);
template <typename T> LM_FORCE_INLINE T Dot(const TVec2<T>& v1, const TVec2<T>& v2);

template <typename T> LM_FORCE_INLINE TVec2<T> Min(const TVec2<T>& v1, const TVec2<T>& v2);
template <typename T> LM_FORCE_INLINE TVec2<T> Max(const TVec2<T>& v1, const TVec2<T>& v2);

typedef TVec2<float> Vec2f;
typedef TVec2<double> Vec2d;
typedef TVec2<int> Vec2i;

// --------------------------------------------------------------------------------

/*!
	3D vector.
	Generic 3-dimensional vector.
	\tparam T Internal value type.
*/
template <typename T>
struct TVec3
{

	T x, y, z;

	LM_FORCE_INLINE TVec3();
	LM_FORCE_INLINE TVec3(const TVec2<T>& v);
	LM_FORCE_INLINE TVec3(const TVec3<T>& v);
	LM_FORCE_INLINE TVec3(const TVec4<T>& v);
	LM_FORCE_INLINE TVec3(const T& v);
	LM_FORCE_INLINE TVec3(const T& x, const T& y, const T& z);
	LM_FORCE_INLINE TVec3(const TVec2<T>& v, const T& z);
	LM_FORCE_INLINE T& operator[](int i);
	LM_FORCE_INLINE const T& operator[](int i) const;
	LM_FORCE_INLINE TVec3<T>& operator=(const TVec3<T>& v);
	LM_FORCE_INLINE TVec3<T>& operator+=(const TVec3<T>& v);
	LM_FORCE_INLINE TVec3<T>& operator-=(const TVec3<T>& v);
	LM_FORCE_INLINE TVec3<T>& operator*=(const TVec3<T>& v);
	LM_FORCE_INLINE TVec3<T>& operator*=(const T& s);
	LM_FORCE_INLINE TVec3<T>& operator/=(const TVec3<T>& v);
	LM_FORCE_INLINE TVec3<T>& operator/=(const T& s);

};

template <typename T> LM_FORCE_INLINE TVec3<T> operator+(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE TVec3<T> operator-(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE TVec3<T> operator*(const TVec3<T>& v, const T& s);
template <typename T> LM_FORCE_INLINE TVec3<T> operator*(const T& s, const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TVec3<T> operator*(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE TVec3<T> operator/(const TVec3<T>& v, const T& s);
template <typename T> LM_FORCE_INLINE TVec3<T> operator/(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE TVec3<T> operator-(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE bool operator==(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE bool operator!=(const TVec3<T>& v1, const TVec3<T>& v2);

template <typename T> LM_FORCE_INLINE T Length(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE T Length2(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TVec3<T> Normalize(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE T Dot(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE TVec3<T> Cross(const TVec3<T>& v1, const TVec3<T>& v2);

template <typename T> LM_FORCE_INLINE TVec3<T> Min(const TVec3<T>& v1, const TVec3<T>& v2);
template <typename T> LM_FORCE_INLINE TVec3<T> Max(const TVec3<T>& v1, const TVec3<T>& v2);

template <typename T> LM_FORCE_INLINE T Luminance(const TVec3<T>& v);

// Useful trigonometric functions for shading coordinates.
// Theta is the angle between z axis and the given vector.
template <typename T> LM_FORCE_INLINE T CosThetaZUp(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE T SinTheta2ZUp(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE T TanThetaZUp(const TVec3<T>& v);

typedef TVec3<float> Vec3f;
typedef TVec3<double> Vec3d;
typedef TVec3<int> Vec3i;

// --------------------------------------------------------------------------------

/*!
	4D vector.
	Generic 4-dimensional vector.
	\tparam T Internal value type
*/
template <typename T>
struct TVec4
{

	T x, y, z, w;

	LM_FORCE_INLINE TVec4();
	LM_FORCE_INLINE TVec4(const TVec2<T>& v);
	LM_FORCE_INLINE TVec4(const TVec3<T>& v);
	LM_FORCE_INLINE TVec4(const TVec4<T>& v);
	LM_FORCE_INLINE TVec4(const T& v);
	LM_FORCE_INLINE TVec4(const T& x, const T& y, const T& z, const T& w);
	LM_FORCE_INLINE TVec4(const TVec3<T>& v, const T& w);
	LM_FORCE_INLINE T& operator[](int i);
	LM_FORCE_INLINE const T& operator[](int i) const;
	LM_FORCE_INLINE TVec4<T>& operator=(const TVec4<T>& v);
	LM_FORCE_INLINE TVec4<T>& operator+=(const TVec4<T>& v);
	LM_FORCE_INLINE TVec4<T>& operator-=(const TVec4<T>& v);
	LM_FORCE_INLINE TVec4<T>& operator*=(const TVec4<T>& v);
	LM_FORCE_INLINE TVec4<T>& operator*=(const T& s);
	LM_FORCE_INLINE TVec4<T>& operator/=(const TVec4<T>& v);
	LM_FORCE_INLINE TVec4<T>& operator/=(const T& s);

};

template <typename T> LM_FORCE_INLINE TVec4<T> operator+(const TVec4<T>& v1, const TVec4<T>& v2);
template <typename T> LM_FORCE_INLINE TVec4<T> operator-(const TVec4<T>& v1, const TVec4<T>& v2);
template <typename T> LM_FORCE_INLINE TVec4<T> operator*(const TVec4<T>& v, const T& s);
template <typename T> LM_FORCE_INLINE TVec4<T> operator*(const T& s, const TVec4<T>& v);
template <typename T> LM_FORCE_INLINE TVec4<T> operator*(const TVec4<T>& v1, const TVec4<T>& v2);
template <typename T> LM_FORCE_INLINE TVec4<T> operator/(const TVec4<T>& v, const T& s);
template <typename T> LM_FORCE_INLINE TVec4<T> operator/(const TVec4<T>& v1, const TVec4<T>& v2);
template <typename T> LM_FORCE_INLINE TVec4<T> operator-(const TVec4<T>& v);
template <typename T> LM_FORCE_INLINE bool operator==(const TVec4<T>& v1, const TVec4<T>& v2);
template <typename T> LM_FORCE_INLINE bool operator!=(const TVec4<T>& v1, const TVec4<T>& v2);

template <typename T> LM_FORCE_INLINE T Length(const TVec4<T>& v);
template <typename T> LM_FORCE_INLINE T Length2(const TVec4<T>& v);
template <typename T> LM_FORCE_INLINE TVec4<T> Normalize(const TVec4<T>& v);
template <typename T> LM_FORCE_INLINE T Dot(const TVec4<T>& v1, const TVec4<T>& v2);

template <typename T> LM_FORCE_INLINE TVec4<T> Min(const TVec4<T>& v1, const TVec4<T>& v2);
template <typename T> LM_FORCE_INLINE TVec4<T> Max(const TVec4<T>& v1, const TVec4<T>& v2);

typedef TVec4<float> Vec4f;
typedef TVec4<double> Vec4d;
typedef TVec4<int> Vec4i;

// --------------------------------------------------------------------------------

#ifdef LM_USE_SSE2

/*!
	SSE optimized 3D vector.
	Specialized version of TVec3 optimized by SSE.
*/
template <>
struct LM_ALIGN_16 TVec3<float>
{

	union
	{
		__m128 v;
		struct { float x, y, z, _; };
	};
	
	LM_FORCE_INLINE TVec3();
	LM_FORCE_INLINE TVec3(const Vec2f& v);
	LM_FORCE_INLINE TVec3(const Vec3f& v);
	LM_FORCE_INLINE TVec3(const Vec4f& v);
	LM_FORCE_INLINE TVec3(float v);
	LM_FORCE_INLINE TVec3(__m128 v);
	LM_FORCE_INLINE TVec3(float x, float y, float z);
	LM_FORCE_INLINE TVec3(const Vec2f& v, float z);
	LM_FORCE_INLINE float operator[](int i) const;
	LM_FORCE_INLINE Vec3f& operator=(const Vec3f& v);
	LM_FORCE_INLINE Vec3f& operator+=(const Vec3f& v);
	LM_FORCE_INLINE Vec3f& operator-=(const Vec3f& v);
	LM_FORCE_INLINE Vec3f& operator*=(const Vec3f& v);
	LM_FORCE_INLINE Vec3f& operator*=(float s);
	LM_FORCE_INLINE Vec3f& operator/=(const Vec3f& v);
	LM_FORCE_INLINE Vec3f& operator/=(float s);

};

template <> LM_FORCE_INLINE Vec3f operator+(const Vec3f& v1, const Vec3f& v2);
template <> LM_FORCE_INLINE Vec3f operator-(const Vec3f& v1, const Vec3f& v2);
template <> LM_FORCE_INLINE Vec3f operator*(const Vec3f& v, const float& s);
template <> LM_FORCE_INLINE Vec3f operator*(const float& s, const Vec3f& v);
template <> LM_FORCE_INLINE Vec3f operator*(const Vec3f& v1, const Vec3f& v2);
template <> LM_FORCE_INLINE Vec3f operator/(const Vec3f& v, const float& s);
template <> LM_FORCE_INLINE Vec3f operator/(const Vec3f& v1, const Vec3f& v2);
template <> LM_FORCE_INLINE Vec3f operator-(const Vec3f& v);

template <> LM_FORCE_INLINE float Length(const Vec3f& v);
template <> LM_FORCE_INLINE float Length2(const Vec3f& v);
template <> LM_FORCE_INLINE Vec3f Normalize(const Vec3f& v);
template <> LM_FORCE_INLINE float Dot(const Vec3f& v1, const Vec3f& v2);
template <> LM_FORCE_INLINE Vec3f Cross(const Vec3f& v1, const Vec3f& v2);

template <> LM_FORCE_INLINE Vec3f Min(const Vec3f& v1, const Vec3f& v2);
template <> LM_FORCE_INLINE Vec3f Max(const Vec3f& v1, const Vec3f& v2);

// --------------------------------------------------------------------------------

/*!
	SSE optimized 4D vector.
	Specialized version of TVec4 optimized by SSE.
*/
template <>
struct LM_ALIGN_16 TVec4<float>
{

	union
	{
		__m128 v;
		struct { float x, y, z, w; };
	};
	
	LM_FORCE_INLINE TVec4();
	LM_FORCE_INLINE TVec4(const Vec2f& v);
	LM_FORCE_INLINE TVec4(const Vec3f& v);
	LM_FORCE_INLINE TVec4(const Vec4f& v);
	LM_FORCE_INLINE TVec4(float v);
	LM_FORCE_INLINE TVec4(__m128 v);
	LM_FORCE_INLINE TVec4(float x, float y, float z, float w);
	LM_FORCE_INLINE TVec4(const Vec3f& v, float w);
	LM_FORCE_INLINE float operator[](int i) const;
	LM_FORCE_INLINE Vec4f& operator=(const Vec4f& v);
	LM_FORCE_INLINE Vec4f& operator+=(const Vec4f& v);
	LM_FORCE_INLINE Vec4f& operator-=(const Vec4f& v);
	LM_FORCE_INLINE Vec4f& operator*=(const Vec4f& v);
	LM_FORCE_INLINE Vec4f& operator*=(float s);
	LM_FORCE_INLINE Vec4f& operator/=(const Vec4f& v);
	LM_FORCE_INLINE Vec4f& operator/=(float s);

};

template <> LM_FORCE_INLINE Vec4f operator+(const Vec4f& v1, const Vec4f& v2);
template <> LM_FORCE_INLINE Vec4f operator-(const Vec4f& v1, const Vec4f& v2);
template <> LM_FORCE_INLINE Vec4f operator*(const Vec4f& v, const float& s);
template <> LM_FORCE_INLINE Vec4f operator*(const float& s, const Vec4f& v);
template <> LM_FORCE_INLINE Vec4f operator*(const Vec4f& v1, const Vec4f& v2);
template <> LM_FORCE_INLINE Vec4f operator/(const Vec4f& v, const float& s);
template <> LM_FORCE_INLINE Vec4f operator/(const Vec4f& v1, const Vec4f& v2);
template <> LM_FORCE_INLINE Vec4f operator-(const Vec4f& v);

template <> LM_FORCE_INLINE float Length(const Vec4f& v);
template <> LM_FORCE_INLINE float Length2(const Vec4f& v);
template <> LM_FORCE_INLINE Vec4f Normalize(const Vec4f& v);
template <> LM_FORCE_INLINE float Dot(const Vec4f& v1, const Vec4f& v2);

template <> LM_FORCE_INLINE Vec4f Min(const Vec4f& v1, const Vec4f& v2);
template <> LM_FORCE_INLINE Vec4f Max(const Vec4f& v1, const Vec4f& v2);

#endif

// --------------------------------------------------------------------------------

#ifdef LM_USE_AVX

/*!
	AVX optimized 3D vector.
	Specialized version of TVec3 optimized by AVX.
*/
template <>
struct LM_ALIGN_32 TVec3<double>
{

	union
	{
		__m256d v;
		struct { double x, y, z, _; };
	};
	
	LM_FORCE_INLINE TVec3();
	LM_FORCE_INLINE TVec3(const Vec2d& v);
	LM_FORCE_INLINE TVec3(const Vec3d& v);
	LM_FORCE_INLINE TVec3(const Vec4d& v);
	LM_FORCE_INLINE TVec3(double v);
	LM_FORCE_INLINE TVec3(__m256d v);
	LM_FORCE_INLINE TVec3(double x, double y, double z);
	LM_FORCE_INLINE TVec3(const Vec2d& v, double z);
	LM_FORCE_INLINE double operator[](int i) const;
	LM_FORCE_INLINE Vec3d& operator=(const Vec3d& v);
	LM_FORCE_INLINE Vec3d& operator+=(const Vec3d& v);
	LM_FORCE_INLINE Vec3d& operator-=(const Vec3d& v);
	LM_FORCE_INLINE Vec3d& operator*=(const Vec3d& v);
	LM_FORCE_INLINE Vec3d& operator*=(double s);
	LM_FORCE_INLINE Vec3d& operator/=(const Vec3d& v);
	LM_FORCE_INLINE Vec3d& operator/=(double s);

};

template <> LM_FORCE_INLINE Vec3d operator+(const Vec3d& v1, const Vec3d& v2);
template <> LM_FORCE_INLINE Vec3d operator-(const Vec3d& v1, const Vec3d& v2);
template <> LM_FORCE_INLINE Vec3d operator*(const Vec3d& v, const double& s);
template <> LM_FORCE_INLINE Vec3d operator*(const double& s, const Vec3d& v);
template <> LM_FORCE_INLINE Vec3d operator*(const Vec3d& v1, const Vec3d& v2);
template <> LM_FORCE_INLINE Vec3d operator/(const Vec3d& v, const double& s);
template <> LM_FORCE_INLINE Vec3d operator/(const Vec3d& v1, const Vec3d& v2);
template <> LM_FORCE_INLINE Vec3d operator-(const Vec3d& v);

template <> LM_FORCE_INLINE double Length(const Vec3d& v);
template <> LM_FORCE_INLINE double Length2(const Vec3d& v);
template <> LM_FORCE_INLINE Vec3d Normalize(const Vec3d& v);
template <> LM_FORCE_INLINE double Dot(const Vec3d& v1, const Vec3d& v2);
template <> LM_FORCE_INLINE Vec3d Cross(const Vec3d& v1, const Vec3d& v2);

template <> LM_FORCE_INLINE Vec3d Min(const Vec3d& v1, const Vec3d& v2);
template <> LM_FORCE_INLINE Vec3d Max(const Vec3d& v1, const Vec3d& v2);

// --------------------------------------------------------------------------------

/*!
	AVX optimized 4D vector.
	Specialized version of TVec4 optimized by AVX.
*/
template <>
struct LM_ALIGN_32 TVec4<double>
{

	// Be careful: direct manipulation of the x to w components could incur a performance penalty.
	// http://www.gamedev.net/topic/434059-accessing-sse-__m128-vectors-fields/
	union
	{
		__m256d v;
		struct { double x, y, z, w; };
	};
	
	LM_FORCE_INLINE TVec4();
	LM_FORCE_INLINE TVec4(const Vec2d& v);
	LM_FORCE_INLINE TVec4(const Vec3d& v);
	LM_FORCE_INLINE TVec4(const Vec4d& v);
	LM_FORCE_INLINE TVec4(double v);
	LM_FORCE_INLINE TVec4(__m256d v);
	LM_FORCE_INLINE TVec4(double x, double y, double z, double w);
	LM_FORCE_INLINE TVec4(const Vec3d& v, double w);
	LM_FORCE_INLINE double operator[](int i) const;
	LM_FORCE_INLINE Vec4d& operator=(const Vec4d& v);
	LM_FORCE_INLINE Vec4d& operator+=(const Vec4d& v);
	LM_FORCE_INLINE Vec4d& operator-=(const Vec4d& v);
	LM_FORCE_INLINE Vec4d& operator*=(const Vec4d& v);
	LM_FORCE_INLINE Vec4d& operator*=(double s);
	LM_FORCE_INLINE Vec4d& operator/=(const Vec4d& v);
	LM_FORCE_INLINE Vec4d& operator/=(double s);

};

template <> LM_FORCE_INLINE Vec4d operator+(const Vec4d& v1, const Vec4d& v2);
template <> LM_FORCE_INLINE Vec4d operator-(const Vec4d& v1, const Vec4d& v2);
template <> LM_FORCE_INLINE Vec4d operator*(const Vec4d& v, const double& s);
template <> LM_FORCE_INLINE Vec4d operator*(const double& s, const Vec4d& v);
template <> LM_FORCE_INLINE Vec4d operator*(const Vec4d& v1, const Vec4d& v2);
template <> LM_FORCE_INLINE Vec4d operator/(const Vec4d& v, const double& s);
template <> LM_FORCE_INLINE Vec4d operator/(const Vec4d& v1, const Vec4d& v2);
template <> LM_FORCE_INLINE Vec4d operator-(const Vec4d& v);

template <> LM_FORCE_INLINE double Length(const Vec4d& v);
template <> LM_FORCE_INLINE double Length2(const Vec4d& v);
template <> LM_FORCE_INLINE Vec4d Normalize(const Vec4d& v);
template <> LM_FORCE_INLINE double Dot(const Vec4d& v1, const Vec4d& v2);

template <> LM_FORCE_INLINE Vec4d Min(const Vec4d& v1, const Vec4d& v2);
template <> LM_FORCE_INLINE Vec4d Max(const Vec4d& v1, const Vec4d& v2);

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.vector.inl"

#endif // __LIB_LIGHTMETRICA_VECTOR_H__