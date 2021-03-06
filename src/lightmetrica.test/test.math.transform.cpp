/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pch.test.h"
#include <lightmetrica.test/base.math.h>
#include <lightmetrica/math.transform.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

template <typename T>
class MathTransformTest : public MathTestBase<T> {};

TYPED_TEST_CASE(MathTransformTest, MathTestTypes);

TYPED_TEST(MathTransformTest, Translate)
{
	typedef TypeParam T;

	Math::TVec4<T> v1(T(1), T(2), T(3), T(1));
	Math::TVec3<T> v2(T(3), T(2), T(1));
	Math::TVec4<T> expect(T(4), T(4), T(4), T(1));

	EXPECT_TRUE(ExpectVec4Near(expect, Math::Translate(v2) * v1));
}

TYPED_TEST(MathTransformTest, Rotate)
{
	typedef TypeParam T;

	Math::TVec4<T> v(T(1), T(0), T(0), T(1));
	Math::TVec3<T> axis(T(0), T(0), T(1));
	T angle(90);
	Math::TVec4<T> expect(T(0), T(1), T(0), T(1));
	
	EXPECT_TRUE(ExpectVec4Near(expect, Math::Rotate(angle, axis) * v));
}

TYPED_TEST(MathTransformTest, Scale)
{
	typedef TypeParam T;

	Math::TVec4<T> v(T(1), T(2), T(3), T(1));
	Math::TVec3<T> scale(T(2));
	Math::TVec4<T> expect(T(2), T(4), T(6), T(1));
	EXPECT_TRUE(ExpectVec4Near(expect, Math::Scale(scale) * v));
}

TYPED_TEST(MathTransformTest, LookAt)
{
	typedef TypeParam T;

	auto V1 = Math::LookAt(
		Math::TVec3<T>(T(0), T(1), T(0)),
		Math::TVec3<T>(T(0)),
		Math::TVec3<T>(T(0), T(0), T(1)));

	auto V2 = Math::LookAt(
		Math::TVec3<T>(T(1)),
		Math::TVec3<T>(T(0)),
		Math::TVec3<T>(T(0), T(0), T(1)));

	Math::TVec4<T> t;
	Math::TVec4<T> expect;

	// (0, 0, 0) in world coords. -> (0, 0, -1) in eye coords.
	t = V1 * Math::TVec4<T>(T(0), T(0), T(0), T(1));
	expect = Math::TVec4<T>(T(0), T(0), T(-1), T(1));
	EXPECT_TRUE(ExpectVec4Near(expect, t));

	// (0, 0, 0) in world coords. -> (0, 0, -sqrt(3)) in eye coords.
	t = V2 * Math::TVec4<T>(T(0), T(0), T(0), T(1));
	expect = Math::TVec4<T>(T(0), T(0), -Math::Sqrt(T(3)), T(1));
	EXPECT_TRUE(ExpectVec4Near(expect, t));
}

TYPED_TEST(MathTransformTest, Perspective)
{
	typedef TypeParam T;

	T fovy(90), aspect(1.5), zNear(1), zFar(1000);
	auto P = Math::Perspective(fovy, aspect, zNear, zFar);

	Math::TVec4<T> t;
	Math::TVec3<T> expect;

	// (0, 0, -1) -> (0, 0, -1) in NDC
	t = P * Math::TVec4<T>(T(0), T(0), T(-1), T(1));
	expect = Math::TVec3<T>(T(0), T(0), T(-1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));

	// (0, 0, -1000) -> (0, 0, 1) in NDC
	t = P * Math::TVec4<T>(T(0), T(0), T(-1000), T(1));
	expect = Math::TVec3<T>(T(0), T(0), T(1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));
	
	// (1.5, 1, -1) -> (1, 1, -1) in NDC
	t = P * Math::TVec4<T>(T(1.5), T(1), T(-1), T(1));
	expect = Math::TVec3<T>(T(1), T(1), T(-1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));

	// (-1500, -1000, -1000) -> (-1, -1, 1) in NDC
	t = P * Math::TVec4<T>(T(-1500), T(-1000), T(-1000), T(1));
	expect = Math::TVec3<T>(T(-1), T(-1), T(1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END