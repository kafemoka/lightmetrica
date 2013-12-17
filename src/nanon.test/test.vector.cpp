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

#include "pch.h"
#include "base.math.h"

using namespace nanon;

NANON_TEST_NAMESPACE_BEGIN

template <typename T>
class Vector2Test : public MathTestBase<T>
{
public:

	Vector2Test()
	{
		v1 = Math::TVec2<T>(T(1), T(2));
		v2 = Math::TVec2<T>(T(4), T(3));
		v1s2 = Math::TVec2<T>(T(2), T(4));
		v1v2 = Math::TVec2<T>(T(4), T(6));
	}

protected:

	Math::TVec2<T> v1, v2;
	Math::TVec2<T> v1s2, v1v2;

};

TYPED_TEST_CASE(Vector2Test, MathTestTypes);

TYPED_TEST(Vector2Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1.x));
	EXPECT_TRUE(ExpectNear(T(2), v1.y));
}

TYPED_TEST(Vector2Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), v1[1]));
}

TYPED_TEST(Vector2Test, Multiply)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectVec2Near(v1s2, v1 * T(2)));
	EXPECT_TRUE(ExpectVec2Near(v1s2, T(2) * v1));
	EXPECT_TRUE(ExpectVec2Near(v1v2, v1 * v2));
}

// --------------------------------------------------------------------------------

template <typename T>
class Vector3Test : public MathTestBase<T>
{
public:

	Vector3Test()
	{
		v1 = Math::TVec3<T>(T(1), T(2), T(3));
		v2 = Math::TVec3<T>(T(4), T(3), T(2));
		v1s2 = Math::TVec3<T>(T(2), T(4), T(6));
		v1v2 = Math::TVec3<T>(T(4), T(6), T(6));
	}

protected:

	Math::TVec3<T> v1, v2;
	Math::TVec3<T> v1s2, v1v2;

};

TYPED_TEST_CASE(Vector3Test, MathTestTypes);

TYPED_TEST(Vector3Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1.x));
	EXPECT_TRUE(ExpectNear(T(2), v1.y));
	EXPECT_TRUE(ExpectNear(T(3), v1.z));
}

TYPED_TEST(Vector3Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), v1[1]));
	EXPECT_TRUE(ExpectNear(T(3), v1[2]));
}

TYPED_TEST(Vector3Test, Multiply)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectVec3Near(v1s2, v1 * T(2)));
	EXPECT_TRUE(ExpectVec3Near(v1s2, T(2) * v1));
	EXPECT_TRUE(ExpectVec3Near(v1v2, v1 * v2));
}

// --------------------------------------------------------------------------------

template <typename T>
class Vector4Test : public MathTestBase<T>
{
public:

	Vector4Test()
	{
		v1 = Math::TVec4<T>(T(1), T(2), T(3), T(4));
		v2 = Math::TVec4<T>(T(4), T(3), T(2), T(1));
		v1s2 = Math::TVec4<T>(T(2), T(4), T(6), T(8));
		v1v2 = Math::TVec4<T>(T(4), T(6), T(6), T(4));
	}

protected:

	Math::TVec4<T> v1, v2;
	Math::TVec4<T> v1s2, v1v2;

};

TYPED_TEST_CASE(Vector4Test, MathTestTypes);

TYPED_TEST(Vector4Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1.x));
	EXPECT_TRUE(ExpectNear(T(2), v1.y));
	EXPECT_TRUE(ExpectNear(T(3), v1.z));
	EXPECT_TRUE(ExpectNear(T(4), v1.w));
}

TYPED_TEST(Vector4Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), v1[1]));
	EXPECT_TRUE(ExpectNear(T(3), v1[2]));
	EXPECT_TRUE(ExpectNear(T(4), v1[3]));
}

TYPED_TEST(Vector4Test, Multiply)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectVec4Near(v1s2, v1 * T(2)));
	EXPECT_TRUE(ExpectVec4Near(v1s2, T(2) * v1));
	EXPECT_TRUE(ExpectVec4Near(v1v2, v1 * v2));
}

NANON_TEST_NAMESPACE_END