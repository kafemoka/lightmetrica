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
#ifndef __LIB_LIGHTMETRICA_LIGHT_H__
#define __LIB_LIGHTMETRICA_LIGHT_H__

#include "asset.h"
#include "pdf.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

struct Primitive;

//! Query structure for BSDF::Sample.
struct LightSampleQuery
{
	Math::Vec2 sampleD;		//!< Uniform random numbers for direction sampling.
	Math::Vec2 sampleP;		//!< Uniform random numbers for position sampling.
};

//! Sample data of BSDF::Sample.
struct LightSampleResult
{
	Math::Vec3 p;			//!< Sampled position.
	Math::Vec3 d;			//!< Sampled direction.
	Math::Vec3 gn;			//!< Geometry normal vector of the sampled position.
	PDFEval pdfP;			//!< Evaluation of PDF according to #p.
	PDFEval pdfD;			//!< Evaluation of PDF according to #d.
};

/*!
	Light.
	A base class of the lights.
*/
class LM_PUBLIC_API Light : public Asset
{
public:

	Light(const std::string& id);
	virtual ~Light();

public:

	virtual std::string Name() const { return "light"; }

public:

	/*!
	*/
	virtual void Sample(const LightSampleQuery& query, LightSampleResult& result) const = 0;

	/*!
		Evaluate the emitted radiance.
		Evaluate the emitted radiance L_e(x_n\to x_{n-1}).
		\param d Outgoing direction x_n\to x_{n-1}.
		\param gn Geometry normal.
	*/
	virtual Math::Vec3 EvaluateLe(const Math::Vec3& d, const Math::Vec3& gn) const = 0;
	
	/*!
		Register an reference to the primitive.
		Some implementation of camera needs transformed mesh information for sampling.
		The function registers the reference to the primitive.
		The function is internally called.
		\param primitives An list instances of the primitive.
	*/
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives) = 0;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_LIGHT_H__