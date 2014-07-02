/*
	Lightmetrica : A research-oriented renderer

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
#ifndef LIB_LIGHTMETRICA_TEST_BASE_MATH_H
#define LIB_LIGHTMETRICA_TEST_BASE_MATH_H

#include "base.h"
#ifndef LM_ENABLE_MULTI_PRECISION
#define LM_ENABLE_MULTI_PRECISION
#endif
#include <lightmetrica/math.types.h>
#include <lightmetrica/math.basic.h>
#include <lightmetrica/align.h>

namespace mp = boost::multiprecision;

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

typedef ::testing::Types<float, double, Math::BigFloat> MathTestTypes;

template <typename T>
class MathTestBase : public TestBase {};

template <typename T>
inline ::testing::AssertionResult ExpectNearRelative(const T& expected, const T& actual, const T& epsilon)
{
	T diff = Math::Abs(expected - actual) / Math::Abs(expected);
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure()
			<< "Expected "	<< expected		<< ", "
			<< "Actual "	<< actual		<< ", "
			<< "Diff "		<< diff			<< ", "
			<< "Epsilon "	<< epsilon;
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
inline ::testing::AssertionResult ExpectNear(const T& expected, const T& actual, const T& epsilon)
{
	T diff = Math::Abs(expected - actual);
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure()
			<< "Expected "	<< expected		<< ", "
			<< "Actual "	<< actual		<< ", "
			<< "Diff "		<< diff			<< ", "
			<< "Epsilon "	<< epsilon;
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
inline ::testing::AssertionResult ExpectNear(const T& expected, const T& actual)
{
	return ExpectNear(expected, actual, Math::TConstants<T>::EpsLarge());
}

template <>
inline ::testing::AssertionResult ExpectNear<Math::BigFloat>(const Math::BigFloat& expected, const Math::BigFloat& actual)
{
	Math::BigFloat diff = mp::abs(expected - actual);
	Math::BigFloat epsilon = Math::TConstants<Math::BigFloat>::EpsLarge();
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure()
			<< "Expected "	<< expected.str()	<< ", "
			<< "Actual "	<< actual.str()		<< ", "
			<< "Diff "		<< diff.str()		<< ", "
			<< "Epsilon "	<< epsilon.str();
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
inline ::testing::AssertionResult ExpectVec2Near(const Math::TVec2<T>& expect, const Math::TVec2<T>& actual)
{
	for (int i = 0; i < 2; i++)
	{
		auto result = ExpectNear(expect[i], actual[i]);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
inline ::testing::AssertionResult ExpectVec3Near(const Math::TVec3<T>& expect, const Math::TVec3<T>& actual, const T& epsilon)
{
	for (int i = 0; i < 3; i++)
	{
		auto result = ExpectNear(expect[i], actual[i], epsilon);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
inline ::testing::AssertionResult ExpectVec3Near(const Math::TVec3<T>& expect, const Math::TVec3<T>& actual)
{
	for (int i = 0; i < 3; i++)
	{
		auto result = ExpectNear(expect[i], actual[i]);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
inline ::testing::AssertionResult ExpectVec4Near(const Math::TVec4<T>& expect, const Math::TVec4<T>& actual)
{
	for (int i = 0; i < 4; i++)
	{
		auto result = ExpectNear(expect[i], actual[i]);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
inline ::testing::AssertionResult ExpectMat3Near(const Math::TMat3<T>& expect, const Math::TMat3<T>& actual)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			auto result = ExpectNear(expect[i][j], actual[i][j]);
			if (!result)
			{
				return result;
			}
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
inline ::testing::AssertionResult ExpectMat4Near(const Math::TMat4<T>& expect, const Math::TMat4<T>& actual)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			auto result = ExpectNear(expect[i][j], actual[i][j]);
			if (!result)
			{
				return result;
			}
		}
	}

	return ::testing::AssertionSuccess();
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_BASE_MATH_H
