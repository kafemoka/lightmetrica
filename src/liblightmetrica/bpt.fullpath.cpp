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
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/transportdirection.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/renderutils.h>

LM_NAMESPACE_BEGIN

BPTFullPath::BPTFullPath( int s, int t, const BPTSubpath& lightSubpath, const BPTSubpath& eyeSubpath ) : s(s)
	, t(t)
	, lightSubpath(lightSubpath)
	, eyeSubpath(eyeSubpath)
{
	LM_ASSERT(s > 0 || t > 0);
	LM_ASSERT(s + t >= 2);

	// Compute #pdfDL and #pdfDE
	if (s == 0 && t > 0)
	{
		// Compute #pdfDE[LE]
		auto* z = eyeSubpath.vertices[t-1];
		if (z->areaLight)
		{
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.type = GeneralizedBSDFType::LightDirection;
			bsdfEQ.wo = z->wi;
			pdfDE[TransportDirection::LE] = z->areaLight->EvaluateDirectionPDF(bsdfEQ, z->geom);
		}
	}
	else if (s > 0 && t == 0)
	{
		// Compute #pdfDL[EL]
		auto* y = lightSubpath.vertices[s-1];
		if (y->areaCamera)
		{
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
			bsdfEQ.wo = y->wi;
			pdfDL[TransportDirection::EL] = y->areaCamera->EvaluateDirectionPDF(bsdfEQ, y->geom);
		}
	}
	else if (s > 0 && t > 0)
	{
		auto* y = lightSubpath.vertices[s-1];
		auto* z = eyeSubpath.vertices[t-1];

		GeneralizedBSDFEvaluateQuery bsdfEQ;
		bsdfEQ.type = GeneralizedBSDFType::All;

		auto yz = Math::Normalize(z->geom.p - y->geom.p);
		auto zy = -yz;

		// Compute #pdfDL[EL]
		if (s > 1)
		{
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = yz;
			bsdfEQ.wo = y->wi;
			pdfDL[TransportDirection::EL] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);
		}

		// Compute #pdfDL[LE]
		bsdfEQ.transportDir = TransportDirection::LE;
		bsdfEQ.wi = y->wi;
		bsdfEQ.wo = yz;
		pdfDL[TransportDirection::LE] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);

		// Compute #pdfDE[LE]
		if (t > 1)
		{
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = zy;
			bsdfEQ.wo = z->wi;
			pdfDE[TransportDirection::LE] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
		}

		// Compute #pdfDE[EL]
		bsdfEQ.transportDir = TransportDirection::EL;
		bsdfEQ.wi = z->wi;
		bsdfEQ.wo = zy;
		pdfDE[TransportDirection::EL] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
	}
}

Math::Vec3 BPTFullPath::EvaluateUnweightContribution( const Scene& scene, Math::Vec2& rasterPosition ) const
{
	// Evaluate \alpha^L_s
	auto alphaL = lightSubpath.EvaluateSubpathAlpha(s, rasterPosition);
	if (Math::IsZero(alphaL))
	{
		return Math::Vec3();
	}

	// Evaluate \alpha^E_t
	auto alphaE = eyeSubpath.EvaluateSubpathAlpha(t, rasterPosition);
	if (Math::IsZero(alphaE))
	{
		return Math::Vec3();
	}
	
	// --------------------------------------------------------------------------------

	// Evaluate c_{s,t}
	Math::Vec3 cst;
	
	if (s == 0 && t > 0)
	{
		// z_{t-1} is area light
		auto* v = eyeSubpath.vertices[t-1];
		if (v->areaLight)
		{
			// Camera emitter cannot be an light
			LM_ASSERT(t >= 1);

			// Evaluate Le^0(z_{t-1})
			cst = v->areaLight->EvaluatePosition(v->geom);

			// Evaluate Le^1(z_{t-1}\to z_{t-2})
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::AllEmitter;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wo = v->wi;
			cst *= v->areaLight->EvaluateDirection(bsdfEQ, v->geom);
		}
	}
	else if (s > 0 && t == 0)
	{
		// y_{s-1} is area camera
		auto* v = lightSubpath.vertices[s-1];
		if (v->areaCamera)
		{
			// Light emitter cannot be an camera
			LM_ASSERT(s >= 1);

			// Raster position
			if (v->areaCamera->RayToRasterPosition(v->geom.p, v->wi, rasterPosition))
			{
				// Evaluate We^0(y_{s-1})
				cst = v->areaCamera->EvaluatePosition(v->geom);

				// Evaluate We^1(y_{s-1}\to y_{s-2})
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.type = GeneralizedBSDFType::AllEmitter;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.wo = v->wi;
				cst *= v->areaCamera->EvaluateDirection(bsdfEQ, v->geom);
			}
		}
	}
	else if (s > 0 && t > 0)
	{
		auto* vL = lightSubpath.vertices[s-1];
		auto* vE = eyeSubpath.vertices[t-1];

		// Check connectivity between #vL->geom.p and #vE->geom.p
		Ray shadowRay;
		auto pLpE = vE->geom.p - vL->geom.p;
		auto pLpE_Length = Math::Length(pLpE);
		shadowRay.d = pLpE / pLpE_Length;
		shadowRay.o = vL->geom.p;
		shadowRay.minT = Math::Constants::Eps();
		shadowRay.maxT = pLpE_Length * (Math::Float(1) - Math::Constants::Eps());

		// Update raster position if #t = 1
		bool visible = true;
		if (t == 1)
		{
			visible = scene.MainCamera()->RayToRasterPosition(vE->geom.p, -shadowRay.d, rasterPosition);
		}

		Intersection shadowIsect;
		if (visible && !scene.Intersect(shadowRay, shadowIsect))
		{			
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::All;

			// fsL
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = vL->wi;
			bsdfEQ.wo = shadowRay.d;
			auto fsL = vL->bsdf->EvaluateDirection(bsdfEQ, vL->geom);

			// fsE
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = vE->wi;
			bsdfEQ.wo = -shadowRay.d;
			auto fsE = vE->bsdf->EvaluateDirection(bsdfEQ, vE->geom);

			// Geometry term
			auto G = RenderUtils::GeneralizedGeometryTerm(vL->geom, vE->geom);

			cst = fsL * G * fsE;
		}
	}

	if (Math::IsZero(cst))
	{
		return Math::Vec3();
	}

	// --------------------------------------------------------------------------------

	// Evaluate contribution C^*_{s,t} = \alpha^L_s * c_{s,t} * \alpha^E_t
	return alphaL * cst * alphaE;
}

LM_NAMESPACE_END