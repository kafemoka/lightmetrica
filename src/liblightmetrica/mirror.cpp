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

#include "pch.h"
#include <lightmetrica/bsdf.h>
#include <lightmetrica/align.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN

/*!
	Perfect mirror BSDF.
	Implements perfect mirror BSDF.
*/
class PerfectMirrorBSDF : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("mirror");

public:

	PerfectMirrorBSDF() {}
	~PerfectMirrorBSDF() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual bool SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual bool Degenerated() const { return true; }
	virtual int BSDFTypes() const { return GeneralizedBSDFType::SpecularReflection; }

private:

	Math::Vec3 R;	// Specular reflectance

};

bool PerfectMirrorBSDF::Load( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("specular_reflectance", Math::Vec3(Math::Float(1)), R);
	return true;
}

bool PerfectMirrorBSDF::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return false;
	}

	auto localWo = Math::ReflectZUp(localWi);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::SpecularReflection;
	result.pdf = Math::PDFEval(Math::Float(1) / Math::CosThetaZUp(localWo), Math::ProbabilityMeasure::ProjectedSolidAngle);

	return true;
}

Math::Vec3 PerfectMirrorBSDF::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return Math::Vec3();
	}

	auto localWo = Math::ReflectZUp(localWi);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::SpecularReflection;
	result.pdf = Math::PDFEval(Math::Float(1) / Math::CosThetaZUp(localWo), Math::ProbabilityMeasure::ProjectedSolidAngle);

	auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	// f / p_{\sigma^\bot}
	// R / \cos(w_o) / (p_\sigma / \cos(w_o))
	// R
	return R * sf;
}

bool PerfectMirrorBSDF::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return false;
	}

	auto localWo = Math::ReflectZUp(localWi);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::SpecularReflection;
	result.pdf[query.transportDir] = Math::PDFEval(Math::Float(1) / Math::CosThetaZUp(localWo), Math::ProbabilityMeasure::ProjectedSolidAngle);
	result.pdf[1-query.transportDir] = result.pdf[query.transportDir];

	auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
	if (Math::IsZero(sf))
	{
		return false;
	}

	auto sfInv = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
	if (Math::IsZero(sfInv))
	{
		return false;
	}

	result.weight[query.transportDir] = R * sf;
	result.weight[1-query.transportDir] = R * sfInv;

	return true;
}

Math::Vec3 PerfectMirrorBSDF::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::Vec3();
	}

	if (Math::LInfinityNorm(Math::ReflectZUp(localWi) - localWo) > Math::Constants::EpsLarge())
	{
		return Math::Vec3();
	}

	auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	// f(wi, wo) = R / cos(theta)
	return R * (sf / Math::CosThetaZUp(localWi));
}

Math::PDFEval PerfectMirrorBSDF::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	if (Math::LInfinityNorm(Math::ReflectZUp(localWi) - localWo) > Math::Constants::EpsLarge())
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return Math::PDFEval(Math::Float(1) / Math::CosThetaZUp(localWi), Math::ProbabilityMeasure::ProjectedSolidAngle);
}

LM_COMPONENT_REGISTER_IMPL(PerfectMirrorBSDF, BSDF);

LM_NAMESPACE_END