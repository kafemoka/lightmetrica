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

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

template <typename T>
struct MathMatrix3Test_Data : public Aligned<std::alignment_of<Math::TMat3<T>>::value>
{

	MathMatrix3Test_Data()
	{
		m1 = Math::TMat3<T>(
			T(1), T(2), T(3),
			T(4), T(5), T(6),
			T(7), T(8), T(9));

		m2 = Math::TMat3<T>(
			T(1), T(4), T(7),
			T(2), T(5), T(8),
			T(3), T(6), T(9));

		m1s2 = Math::TMat3<T>(
			T(2), T(4), T(6),
			T(8), T(10), T(12),
			T(14), T(16), T(18));

		m1m2 = Math::TMat3<T>(
			T(66), T(78), T(90),
			T(78), T(93), T(108),
			T(90), T(108), T(126));

		v1 = Math::TVec3<T>(T(3), T(2), T(1));
		m1v1 = Math::TVec3<T>(T(18), T(24), T(30));
	}

	Math::TMat3<T> zero, identity;
	Math::TMat3<T> m1, m2, m3;
	Math::TMat3<T> m1s2, m1m2;
	Math::TVec3<T> v1, m1v1;

};

template <typename T>
class MathMatrix3Test : public MathTestBase<T>
{
public:

	MathMatrix3Test() { d = new MathMatrix3Test_Data<T>(); }
	virtual ~MathMatrix3Test() { LM_SAFE_DELETE(d); }

protected:

	MathMatrix3Test_Data<T>* d;

};

TYPED_TEST_CASE(MathMatrix3Test, MathTestTypes);

TYPED_TEST(MathMatrix3Test, Constructor)
{
	typedef TypeParam T;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*3+j+1), this->d->m1.v[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix3Test, Conversion)
{
	typedef TypeParam T;
	Math::TMat4<T> t1(
		T(1), T(2), T(3), T(4),
		T(4), T(5), T(6), T(8),
		T(7), T(8), T(9), T(12),
		T(13), T(14), T(15), T(16));
	EXPECT_TRUE(ExpectMat3Near(this->d->m1, Math::TMat3<T>(t1)));
}

TYPED_TEST(MathMatrix3Test, Accessor)
{
	typedef TypeParam T;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*3+j+1), this->d->m1[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix3Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TMat3<T> t;

	t = this->d->m1; t *= T(2);
	EXPECT_TRUE(ExpectMat3Near(this->d->m1s2, t));

	t = this->d->m1; t *= this->d->m2;
	EXPECT_TRUE(ExpectMat3Near(this->d->m1m2, t));

	t = this->d->m1s2; t /= T(2);
	EXPECT_TRUE(ExpectMat3Near(this->d->m1, t));
}

TYPED_TEST(MathMatrix3Test, MultiplyDivide)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectMat3Near(this->d->m1s2, this->d->m1 * T(2)));
	EXPECT_TRUE(ExpectMat3Near(this->d->m1s2, T(2) * this->d->m1));
	EXPECT_TRUE(ExpectVec3Near(this->d->m1v1, this->d->m1 * this->d->v1));
	EXPECT_TRUE(ExpectMat3Near(this->d->m1m2, this->d->m1 * this->d->m2));
	EXPECT_TRUE(ExpectMat3Near(this->d->m1, this->d->m1s2 / T(2)));

	// Mat3 constructed from Mat4 may have non-zero fourth component
	// for each column vector when SSE/AVX is enabled.
	Math::TMat3<T> m1_2(Math::TMat4<T>(
		T(1), T(2), T(3), T(1),
		T(4), T(5), T(6), T(1),
		T(7), T(8), T(9), T(1),
		T(1), T(1), T(1), T(1)));
	
	Math::TMat3<T> m2_2(Math::TMat4<T>(
		T(1), T(4), T(7), T(1),
		T(2), T(5), T(8), T(1),
		T(3), T(6), T(9), T(1),
		T(1), T(1), T(1), T(1)));

	EXPECT_TRUE(ExpectVec3Near(this->d->m1v1, m1_2 * this->d->v1));
	EXPECT_TRUE(ExpectMat3Near(this->d->m1m2, m1_2 * m2_2));
}

TYPED_TEST(MathMatrix3Test, Transpose)
{
	EXPECT_TRUE(ExpectMat3Near(this->d->m2, Math::Transpose(this->d->m1)));
}

TYPED_TEST(MathMatrix3Test, Inverse)
{
	typedef TypeParam T;

	// The matrix is orthogonal, so A^-1 should be A^T
	Math::TMat3<T> A(
		T(1) / T(3), T(2) / T(3), T(-2) / T(3),
		T(2) / T(3), T(-2) / T(3), T(-1) / T(3),
		T(2) / T(3), T(1) / T(3), T(2) / T(3));

	Math::TMat3<T> AT(
		T(1) / T(3), T(2) / T(3), T(2) / T(3), 
		T(2) / T(3), T(-2) / T(3), T(1) / T(3),
		T(-2) / T(3), T(-1) / T(3), T(2) / T(3));

	EXPECT_TRUE(ExpectMat3Near(AT, Math::Inverse(A)));
}

// --------------------------------------------------------------------------------


template <typename T>
struct MathMatrix4Test_Data : public Aligned<std::alignment_of<Math::TMat4<T>>::value>
{

	MathMatrix4Test_Data()
	{
		m1 = Math::TMat4<T>(
			T(1), T(2), T(3), T(4),
			T(5), T(6), T(7), T(8),
			T(9), T(10), T(11), T(12),
			T(13), T(14), T(15), T(16));

		m2 = Math::TMat4<T>(
			T(1), T(5), T(9), T(13),
			T(2), T(6), T(10), T(14),
			T(3), T(7), T(11), T(15),
			T(4), T(8), T(12), T(16));

		m1s2 = Math::TMat4<T>(
			T(2), T(4), T(6), T(8),
			T(10), T(12), T(14), T(16),
			T(18), T(20), T(22), T(24),
			T(26), T(28), T(30), T(32));

		m1m2 = Math::TMat4<T>(
			T(276), T(304), T(332), T(360),
			T(304), T(336), T(368), T(400),
			T(332), T(368), T(404), T(440),
			T(360), T(400), T(440), T(480));

		v1 = Math::TVec4<T>(T(4), T(3), T(2), T(1));
		m1v1 = Math::TVec4<T>(T(50), T(60), T(70), T(80));
	}

	Math::TMat4<T> zero, identity;
	Math::TMat4<T> m1, m2, m3;
	Math::TMat4<T> m1s2, m1m2;
	Math::TVec4<T> v1, m1v1;

};

template <typename T>
class MathMatrix4Test : public MathTestBase<T>
{
public:

	MathMatrix4Test() { d = new MathMatrix4Test_Data<T>(); }
	virtual ~MathMatrix4Test() { LM_SAFE_DELETE(d); }

protected:

	MathMatrix4Test_Data<T>* d;

};

TYPED_TEST_CASE(MathMatrix4Test, MathTestTypes);

TYPED_TEST(MathMatrix4Test, Constructor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*4+j+1), this->d->m1.v[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix4Test, Conversion)
{
	typedef TypeParam T;
	Math::TMat3<T> t1(
		T(1), T(2), T(3),
		T(4), T(5), T(6),
		T(7), T(8), T(9));
	Math::TMat4<T> expect(
		T(1), T(2), T(3), T(0),
		T(4), T(5), T(6), T(0),
		T(7), T(8), T(9), T(0),
		T(0), T(0), T(0), T(1));
	EXPECT_TRUE(ExpectMat4Near(expect, Math::TMat4<T>(t1)));
}

TYPED_TEST(MathMatrix4Test, Accessor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*4+j+1), this->d->m1[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix4Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TMat4<T> t;

	t = this->d->m1; t *= T(2);
	EXPECT_TRUE(ExpectMat4Near(this->d->m1s2, t));

	t = this->d->m1; t *= this->d->m2;
	EXPECT_TRUE(ExpectMat4Near(this->d->m1m2, t));

	t = this->d->m1s2; t /= T(2);
	EXPECT_TRUE(ExpectMat4Near(this->d->m1, t));
}

TYPED_TEST(MathMatrix4Test, MultiplyDivide)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectMat4Near(this->d->m1s2, this->d->m1 * T(2)));
	EXPECT_TRUE(ExpectMat4Near(this->d->m1s2, T(2) * this->d->m1));
	EXPECT_TRUE(ExpectVec4Near(this->d->m1v1, this->d->m1 * this->d->v1));
	EXPECT_TRUE(ExpectMat4Near(this->d->m1m2, this->d->m1 * this->d->m2));
	EXPECT_TRUE(ExpectMat4Near(this->d->m1, this->d->m1s2 / T(2)));
}

TYPED_TEST(MathMatrix4Test, Transpose)
{
	EXPECT_TRUE(ExpectMat4Near(this->d->m2, Math::Transpose(this->d->m1)));
}

TYPED_TEST(MathMatrix4Test, Inverse)
{
	typedef TypeParam T;
	
	// The matrix is orthogonal, so A^-1 should be A^T
	Math::TMat4<T> A(
		T( 0.5), T( 0.5), T( 0.5), T(-0.5),
		T(-0.5), T( 0.5), T( 0.5), T( 0.5),
		T( 0.5), T(-0.5), T( 0.5), T( 0.5),
		T( 0.5), T( 0.5), T(-0.5), T( 0.5));

	Math::TMat4<T> AT(
		T( 0.5), T(-0.5), T( 0.5), T( 0.5),
		T( 0.5), T( 0.5), T(-0.5), T( 0.5),
		T( 0.5), T( 0.5), T( 0.5), T(-0.5),
		T(-0.5), T( 0.5), T( 0.5), T( 0.5));

	EXPECT_TRUE(ExpectMat4Near(AT, Math::Inverse(A)));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END