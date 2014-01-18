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
#ifndef __LIB_LIGHTMETRICA_DIFFUSE_H__
#define __LIB_LIGHTMETRICA_DIFFUSE_H__

#include "bsdf.h"

LM_NAMESPACE_BEGIN

/*!
*/
class LM_PUBLIC_API DiffuseBSDF : public BSDF
{
public:

	DiffuseBSDF(const std::string& id);
	~DiffuseBSDF();

public:

	virtual std::string Type() const { return "diffuse"; }
	virtual BSDFType GetBSDFType() const { return BSDFType::DiffuseReflection; }
	bool DiffuseBSDF::LoadAsset( const pugi::xml_node& node, const Assets& assets );
	virtual bool SampleWo( const BSDFSampleQuery& query, BSDFSampledData& sampled ) const;
	virtual Math::Vec3 Evaluate( const BSDFEvaluateQuery& query, const Intersection& isect ) const;
	virtual PDF Pdf( const BSDFEvaluateQuery& query ) const;

private:

	Math::Vec3 diffuseReflectance;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_DIFFUSE_H__