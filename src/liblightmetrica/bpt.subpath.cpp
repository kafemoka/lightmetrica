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

#include "pch.h"
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/bpt.config.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/emitter.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/random.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>

LM_NAMESPACE_BEGIN

BPTPathVertex::BPTPathVertex()
	: type(BPTPathVertexType::None)
	, emitter(nullptr)
	, bsdf(nullptr)
	, areaLight(nullptr)
	, areaCamera(nullptr)
{

}

void BPTPathVertex::DebugPrint() const
{
	LM_LOG_DEBUG("Type : " +
		std::string(
			type == BPTPathVertexType::EndPoint ? "EndPoint" :
			type == BPTPathVertexType::IntermediatePoint ? "IntermediatePoint" : "None"));

	if (type == BPTPathVertexType::None)
	{
		return;
	}

	LM_LOG_DEBUG("Transport direction : " + std::string(transportDir == TransportDirection::EL ? "EL" : "LE"));

	{
		LM_LOG_DEBUG("Surface geometry");
		LM_LOG_INDENTER();
		LM_LOG_DEBUG("Degenerated : " + std::string(geom.degenerated ? "True" : "False"));
		LM_LOG_DEBUG(boost::str(boost::format("Position : (%f, %f, %f)") % geom.p.x % geom.p.y % geom.p.z));
		if (!geom.degenerated)
		{
			LM_LOG_DEBUG(boost::str(boost::format("Geometry normal : (%f, %f, %f)") % geom.gn.x % geom.gn.y % geom.gn.z));
			LM_LOG_DEBUG(boost::str(boost::format("Shading normal : (%f, %f, %f)") % geom.sn.x % geom.sn.y % geom.sn.z));
		}
	}

	static const std::string ProbabilityMeasureNames[] =
	{
		"None",
		"SolidAngle",
		"ProjectedSolidAngle",
		"Area",
		"Discrete"
	};

	if (type == BPTPathVertexType::EndPoint)
	{
		LM_LOG_DEBUG("Emitter type : " + emitter->ComponentImplTypeName() + " (" + emitter->ComponentInterfaceTypeName() + ")");
	}
	else if (type == BPTPathVertexType::IntermediatePoint)
	{
		LM_LOG_DEBUG("Generalized BSDF type : " + bsdf->ComponentImplTypeName() + " (" + bsdf->ComponentImplTypeName() + ")");
	}

	{
		LM_LOG_DEBUG("PDF (positional component)");
		LM_LOG_INDENTER();
		LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfP.measure)]);
		LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfP.v));
	}
	{
		LM_LOG_DEBUG("PDF (directional component, E->L)");
		LM_LOG_INDENTER();
		LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfD[TransportDirection::EL].measure)]);
		LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfD[TransportDirection::EL].v));
	}
	{
		LM_LOG_DEBUG("PDF (directional component, L->E)");
		LM_LOG_INDENTER();
		LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfD[TransportDirection::LE].measure)]);
		LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfD[TransportDirection::LE].v));
	}
}

// --------------------------------------------------------------------------------

BPTSubpath::BPTSubpath( TransportDirection transportDir )
	: transportDir(transportDir)
{

}

void BPTSubpath::Release( BPTPathVertexPool& pool )
{
	for (auto* vertex : vertices)
	{
		pool.Release(vertex);
	}

	vertices.clear();
}

void BPTSubpath::DebugPrint() const
{
	DebugPrint(vertices.size());
}

void BPTSubpath::DebugPrint( size_t n ) const
{
	for (size_t i = 0; i < n; i++)
	{
		LM_LOG_DEBUG("Vertex #" + std::to_string(i));
		LM_LOG_INDENTER();
		vertices[i]->DebugPrint();
	}
}

int BPTSubpath::NumVertices() const
{
	return static_cast<int>(vertices.size());
}

BPTPathVertex* BPTSubpath::GetVertex( int i ) const
{
	return vertices[i];
}

void BPTSubpath::Sample( const BPTConfig& config, const Scene& scene, Random& rng, BPTPathVertexPool& pool )
{
	LM_ASSERT(vertices.empty());

	BPTPathVertex* v;

	// Initial vertex
	v = pool.Construct();
	v->type = BPTPathVertexType::EndPoint;
	v->transportDir = transportDir;

	// Positional component
	if (transportDir == TransportDirection::EL)
	{
		// EyePosition
		v->emitter = scene.MainCamera();
		v->emitter->SamplePosition(rng.NextVec2(), v->geom, v->pdfP);
		if (!v->geom.degenerated)
		{
			v->areaCamera = dynamic_cast<const Camera*>(v->emitter);
		}
	}
	else
	{
		// LightPosition
		auto lightSampleP = rng.NextVec2();
		Math::PDFEval lightSelectionPdf;
		v->emitter = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
		v->emitter->SamplePosition(lightSampleP, v->geom, v->pdfP);
		v->pdfP.v *= lightSelectionPdf.v;
		if (!v->geom.degenerated)
		{
			v->areaLight = dynamic_cast<const Light*>(v->emitter);
		}
	}

	// Directional component
	v->bsdf = v->emitter;

	GeneralizedBSDFSampleQuery bsdfSQE;
	bsdfSQE.sample = rng.NextVec2();
	bsdfSQE.transportDir = transportDir;
	bsdfSQE.type = GeneralizedBSDFType::AllEmitter;

	GeneralizedBSDFSampleResult bsdfSRE;
	v->bsdf->SampleDirection(bsdfSQE, v->geom, bsdfSRE);
	v->pdfD[transportDir] = bsdfSRE.pdf;
	v->pdfD[1-transportDir] = Math::PDFEval();
	v->wo = bsdfSRE.wo;

	// # of vertices is always greater than 1
	v->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);

	vertices.push_back(v);

	// --------------------------------------------------------------------------------

	int depth = 1;
	while (true)
	{
		// Previous vertex
		auto* pv = vertices.back();

		// Create ray
		Ray ray;
		ray.d = pv->wo;
		ray.o = pv->geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

		// --------------------------------------------------------------------------------

		// Create path vertex
		v = pool.Construct();
		v->type = BPTPathVertexType::IntermediatePoint;
		v->transportDir = transportDir;
		v->bsdf = isect.primitive->bsdf;
		v->geom = isect.geom;
		v->wi = -pv->wo;

		// Area light or camera
		v->areaLight = isect.primitive->light;
		v->areaCamera = isect.primitive->camera;

		// Area light or camera should not be associated with the same surfaces
		LM_ASSERT(v->areaLight == nullptr || v->areaCamera == nullptr);
		if (v->areaLight)
		{
			v->emitter = v->areaLight;
		}
		if (v->areaCamera)
		{
			v->emitter = v->areaCamera;
		}
		if (v->emitter)
		{
			// Calculate #pdfP for intersected emitter
			v->pdfP = v->emitter->EvaluatePositionPDF(v->geom);
		}
		else
		{
			v->pdfP = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::Area);
		}

		// --------------------------------------------------------------------------------

		// Apply RR
		int rrDepthT = config.rrDepth;
#if LM_ENABLE_BPT_EXPERIMENTAL
		if (config.enableExperimentalMode)
		{
			// At least #maxSubpathNumVertices vertices are sampled in the experimental mode
			rrDepthT = Math::Max(rrDepthT, config.maxSubpathNumVertices);
		}
#endif

		if (++depth >= rrDepthT)
		{
			// TODO : Replace with the more efficient one
			auto p = Math::Float(0.5);
			if (rng.Next() > p)
			{
				vertices.push_back(v);
				break;
			}

			// RR probability
			v->pdfRR = Math::PDFEval(p, Math::ProbabilityMeasure::Discrete);
		}
		else
		{
			v->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
		}

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.uComp = rng.Next();
		bsdfSQ.transportDir = transportDir;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = -pv->wo;

		GeneralizedBSDFSampleResult bsdfSR;
#if 0
		v->fs_Estimated[transportDir] = v->bsdf->SampleAndEstimateDirection(bsdfSQ, v->geom, bsdfSR);
		if (Math::IsZero(v->fs_Estimated[transportDir]))
		{
			vertices.push_back(v);
			break;
		}
#else
		if (!v->bsdf->SampleDirection(bsdfSQ, v->geom, bsdfSR))
		{
			vertices.push_back(v);
			break;
		}
#endif

		v->wo = bsdfSR.wo;
		v->pdfD[transportDir] = bsdfSR.pdf;

		// Evaluate PDF in the opposite transport direction
		if (!pv->geom.degenerated)
		{
			if ((bsdfSR.sampledType & GeneralizedBSDFType::Specular) != 0)
			{
				// For specular BSDF opposite PDF is evaluated directly because of numerical reason
				if ((bsdfSR.sampledType & GeneralizedBSDFType::SpecularReflection) != 0)
				{
					v->pdfD[1-transportDir] = v->pdfD[transportDir];
				}
				else if ((bsdfSR.sampledType & GeneralizedBSDFType::SpecularTransmission) != 0)
				{
					auto localWi = v->geom.worldToShading * v->wi;
					auto localWo = v->geom.worldToShading * v->wo;
					v->pdfD[1-transportDir].measure = Math::ProbabilityMeasure::ProjectedSolidAngle;
					v->pdfD[1-transportDir].v = v->pdfD[transportDir].v * (Math::AbsCosThetaZUp(localWi) / Math::AbsCosThetaZUp(localWo));
				}
				else
				{
					LM_LOG_ERROR("Invalid type");
					break;
				}
			}
			else
			{
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.type = bsdfSR.sampledType;
				bsdfEQ.transportDir = TransportDirection(1 - transportDir);
				bsdfEQ.wi = bsdfSR.wo;
				bsdfEQ.wo = bsdfSQ.wi;
				v->pdfD[1-transportDir] = v->bsdf->EvaluateDirectionPDF(bsdfEQ, v->geom);
			}
		}
		else
		{
			v->pdfD[1-transportDir] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}

		vertices.push_back(v);
	}
}

Math::Vec3 BPTSubpath::EvaluateSubpathAlpha( int vs, Math::Vec2& rasterPosition ) const
{
	Math::Vec3 alpha;

	if (vs == 0)
	{
		// \alpha_0 = 1
		alpha = Math::Vec3(Math::Float(1));
	}
	else
	{
		BPTPathVertex* v = vertices[0];

		LM_ASSERT(v->type == BPTPathVertexType::EndPoint);
		LM_ASSERT(v->emitter != nullptr);
		LM_ASSERT(v->pdfP.measure == Math::ProbabilityMeasure::Area);

		// Calculate raster position if transport direction is EL
		bool visible = true;
		if (transportDir == TransportDirection::EL)
		{
			visible = dynamic_cast<const Camera*>(v->emitter)->RayToRasterPosition(v->geom.p, v->wo, rasterPosition);
		}
		
		if (visible)
		{
			// Emitter
			// \alpha^L_1 = Le^0(y0) / p_A(y0) or \alpha^E_1 = We^0(z0) / p_A(z0)
			alpha = v->emitter->EvaluatePosition(v->geom) / v->pdfP.v;

			for (int i = 0; i < vs - 1; i++)
			{
				v = vertices[i];

				// f_s(y_{i-1}\to y_i\to y_{i+1}) or f_s(z_{i-1}\to z_i\to z_{i+1})
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.type = GeneralizedBSDFType::All;
				bsdfEQ.transportDir = transportDir;
				bsdfEQ.wi = v->wi;
				bsdfEQ.wo = v->wo;
				auto fs = v->bsdf->EvaluateDirection(bsdfEQ, v->geom);

				// Update #alphaL or #alphaE
				LM_ASSERT(v->pdfD[transportDir].measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
				alpha *= fs / v->pdfD[transportDir].v;

				// RR probability
				LM_ASSERT(v->pdfRR.measure == Math::ProbabilityMeasure::Discrete);
				alpha /= v->pdfRR.v;
			}
		}
	}

	return alpha;
}

LM_NAMESPACE_END