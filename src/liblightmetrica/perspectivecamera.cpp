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
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/math.functions.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN

/*!
	Perspective camera.
	A camera with perspective projection. a.k.a. pinhole camera.
*/
class PerspectiveCamera final : public Camera
{
public:

	LM_COMPONENT_IMPL_DEF("perspective");

public:

	PerspectiveCamera() {}
	~PerspectiveCamera() {}

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) override;

public:

	virtual bool SampleDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual Math::Vec3 SampleAndEstimateDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual bool SampleAndEstimateDirectionBidir(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result) const override;
	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual Math::PDFEval EvaluateDirectionPDF(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual int BSDFTypes() const override { return GeneralizedBSDFType::NonDeltaEyeDirection; }

public:

	virtual void SamplePosition(const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf) const override;
	virtual Math::Vec3 EvaluatePosition(const SurfaceGeometry& geom) const override;
	virtual Math::PDFEval EvaluatePositionPDF(const SurfaceGeometry& geom) const override;
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives) override;
	virtual void PostConfigure(const Scene& scene) override {}
	virtual EmitterShape* CreateEmitterShape() const override { return nullptr; }
	virtual AABB GetAABB() const override { return AABB(position); }

public:

	virtual bool RayToRasterPosition(const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos) const override;
	virtual Film* GetFilm() const override { return film; }

private:

	/*
		Calculate importance W_e(z_0\to y_{s-1}),
		i.e., sensitivity of the sensor
	*/
	Math::Float EvaluateImportance(Math::Float cosTheta) const;

private:

	Film* film;
	Math::Float invA;
	Math::Vec3 position;
	Math::Mat4 viewMatrix;
	Math::Mat4 invViewMatrix;
	Math::Mat4 projectionMatrix;
	Math::Mat4 invProjectionMatrix;

};

bool PerspectiveCamera::Load( const ConfigNode& node, const Assets& assets )
{
	// 'fovy'
	Math::Float fovy(0);
	if (!node.ChildValue("fovy", fovy)) return false;

	// Aspect ratio
	Math::Float aspect(0);

	// For testing configuration
	// TODO : This is ugly
	auto testingNode = node.Child("testing");
	if (!testingNode.Empty())
	{
		LM_LOG_WARN("Testing configuration is enabled");
		if (!testingNode.ChildValue("aspect", aspect)) return false;

		// View matrix : use 'lookat'
		auto lookAtNode = testingNode.Child("lookat");
		if (!lookAtNode.Empty())
		{
			// Position, center, up
			Math::Vec3 position, center, up;
			if (!lookAtNode.ChildValue("position", position))	return false;
			if (!lookAtNode.ChildValue("center", center))		return false;
			if (!lookAtNode.ChildValue("up", up))				return false;

			// Create transform
			viewMatrix = Math::LookAt(position, center, up);
			invViewMatrix = Math::Inverse(viewMatrix);

			// Position of the camera (in world coordinates)
			position = Math::Vec3(invViewMatrix * Math::Vec4(0, 0, 0, 1));
		}
		else
		{
			LM_LOG_WARN("Missing 'lookat' element");
			return false;
		}
	}
	else
	{
		// Resolve reference to film
		film = assets.ResolveReferenceToAsset<Film>(node.Child("film"));
		if (!film)
		{
			return false;
		}

		aspect = Math::Float(film->Width()) / Math::Float(film->Height());
	}

	// --------------------------------------------------------------------------------

	// Projection matrix and its inverse
	projectionMatrix = Math::Perspective(fovy, aspect, Math::Float(1), Math::Float(1000));
	invProjectionMatrix = Math::Inverse(projectionMatrix);

	// Calculate area of the sensor used for SampleAndEvaluate
	auto ndcP1 = Math::Vec3(-1, -1, 0);
	auto ndcP2 = Math::Vec3(1, 1, 0);

	auto camP1_4 = invProjectionMatrix * Math::Vec4(ndcP1, 1);
	auto camP2_4 = invProjectionMatrix * Math::Vec4(ndcP2, 1);

	auto camP1 = Math::Vec3(camP1_4) / camP1_4.w;
	auto camP2 = Math::Vec3(camP2_4) / camP1_4.w;

	camP1 /= Math::Vec3(camP1.z);
	camP2 /= Math::Vec3(camP2.z);

	Math::Float A = (camP2.x - camP1.x) * (camP2.y - camP1.y);
	invA = Math::Float(1) / A;

	return true;
}

void PerspectiveCamera::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{
	LM_ASSERT(primitives.size() == 1);

	// View matrix and its inverse
	viewMatrix = primitives.front()->transform;
	invViewMatrix = Math::Inverse(viewMatrix);

	// Position of the camera (in world coordinates)
	position = Math::Vec3(invViewMatrix * Math::Vec4(0, 0, 0, 1));
}

void PerspectiveCamera::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{
	geom.p = position;
	geom.degenerated = true;
	pdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Area);
}

bool PerspectiveCamera::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & BSDFTypes()) == 0 || (query.transportDir != TransportDirection::EL))
	{
		return false;
	}

	// Raster position in [-1, 1]^2
	auto ndcRasterPos = Math::Vec3(query.sample * Math::Float(2) - Math::Vec2(Math::Float(1)), Math::Float(0));

	// Convert raster position to camera coordinates
	auto dirTCam4 = invProjectionMatrix * Math::Vec4(ndcRasterPos, Math::Float(1));
	auto dirTCam3 = Math::Normalize(Math::Vec3(dirTCam4) / dirTCam4.w);

	result.sampledType = GeneralizedBSDFType::NonDeltaEyeDirection;
	result.wo = Math::Normalize(Math::Vec3(invViewMatrix * Math::Vec4(dirTCam3, Math::Float(0))));
	result.pdf = Math::PDFEval(
		EvaluateImportance(-Math::CosThetaZUp(dirTCam3)),
		Math::ProbabilityMeasure::ProjectedSolidAngle);

	return true;
}

Math::Vec3 PerspectiveCamera::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & BSDFTypes()) == 0 || (query.transportDir != TransportDirection::EL))
	{
		return Math::Vec3();
	}

	// Raster position in [-1, 1]^2
	auto ndcRasterPos = Math::Vec3(query.sample * Math::Float(2) - Math::Vec2(Math::Float(1)), Math::Float(0));

	// Convert raster position to camera coordinates
	auto dirTCam4 = invProjectionMatrix * Math::Vec4(ndcRasterPos, Math::Float(1));
	auto dirTCam3 = Math::Normalize(Math::Vec3(dirTCam4) / dirTCam4.w);

	result.sampledType = GeneralizedBSDFType::NonDeltaEyeDirection;
	result.wo = Math::Normalize(Math::Vec3(invViewMatrix * Math::Vec4(dirTCam3, Math::Float(0))));
	result.pdf = Math::PDFEval(
		EvaluateImportance(-Math::CosThetaZUp(dirTCam3)),
		Math::ProbabilityMeasure::ProjectedSolidAngle);

	// TODO : Add explanation
	// We_D / p_{\sigma^\bot}
	// = 1
	return Math::Vec3(Math::Float(1));
}

bool PerspectiveCamera::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
{
	if ((query.type & BSDFTypes()) == 0 || (query.transportDir != TransportDirection::EL))
	{
		return false;
	}

	// Raster position in [-1, 1]^2
	auto ndcRasterPos = Math::Vec3(query.sample * Math::Float(2) - Math::Vec2(Math::Float(1)), Math::Float(0));

	// Convert raster position to camera coordinates
	auto dirTCam4 = invProjectionMatrix * Math::Vec4(ndcRasterPos, Math::Float(1));
	auto dirTCam3 = Math::Normalize(Math::Vec3(dirTCam4) / dirTCam4.w);

	result.sampledType = GeneralizedBSDFType::NonDeltaEyeDirection;
	result.wo = Math::Normalize(Math::Vec3(invViewMatrix * Math::Vec4(dirTCam3, Math::Float(0))));
	result.pdf[query.transportDir] = Math::PDFEval(EvaluateImportance(-Math::CosThetaZUp(dirTCam3)), Math::ProbabilityMeasure::ProjectedSolidAngle);
	result.pdf[1-query.transportDir] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	result.weight[query.transportDir] = Math::Vec3(Math::Float(1));
	result.weight[1-query.transportDir] = Math::Vec3();

	return true;
}

Math::Vec3 PerspectiveCamera::EvaluatePosition( const SurfaceGeometry& /*geom*/ ) const
{
	return Math::Vec3(Math::Float(1));
}

Math::PDFEval PerspectiveCamera::EvaluatePositionPDF( const SurfaceGeometry& /*geom*/ ) const
{
	return Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Area);
}

Math::Vec3 PerspectiveCamera::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	if ((query.type & BSDFTypes()) == 0 || (query.transportDir != TransportDirection::EL))
	{
		return Math::Vec3();
	}

	// Reference point in camera coordinates
	auto refCam4 = viewMatrix * Math::Vec4(geom.p + query.wo, Math::Float(1));
	auto refCam3 = Math::Vec3(refCam4);

	// Reference point in NDC
	auto refNdc4 = projectionMatrix * refCam4;
	auto refNdc3 = Math::Vec3(refNdc4) / refNdc4.w;

	// Raster position in [0, 1]^2
	auto rasterPos = (Math::Vec2(refNdc3.x, refNdc3.y) + Math::Vec2(Math::Float(1))) / Math::Float(2);

	// Check visibility
	if (rasterPos.x < 0 || rasterPos.x > 1 || rasterPos.y < 0 || rasterPos.y > 1)
	{
		return Math::Vec3();
	}

	// Importance
	return Math::Vec3(EvaluateImportance(-Math::CosThetaZUp(Math::Normalize(refCam3))));
}

bool PerspectiveCamera::RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const
{
	// Reference point in camera coordinates
	auto refCam4 = viewMatrix * Math::Vec4(p + d, Math::Float(1));

	// Reference point in NDC
	auto refNdc4 = projectionMatrix * refCam4;
	auto refNdc3 = Math::Vec3(refNdc4) / refNdc4.w;

	// Raster position in [0, 1]^2
	rasterPos = (Math::Vec2(refNdc3.x, refNdc3.y) + Math::Vec2(Math::Float(1))) / Math::Float(2);

	// Check visibility
	if (rasterPos.x < 0 || rasterPos.x > 1 || rasterPos.y < 0 || rasterPos.y > 1)
	{
		return false;
	}

	return true;
}

Math::Float PerspectiveCamera::EvaluateImportance( Math::Float cosTheta ) const
{
	// Assume hypothetical sensor on z=-d in camera coordinates.
	// Then the sensitivity is 1/Ad^2 where A is area of the sensor when d=1.
	// Converting the measure, the sensitivity is
	//  W_e(z_0\to y_{s-1})
	//   = dA/d\omega 1/Ad^2
	//   = \| p - z_0 \|^2 / \cos{(\theta)} / Ad^2
	//   = 1 / (A * \cos^3{(\theta)}),
	// where p is the raster position on the sensor,
	// \theta is the angle between the normal on p and p - z_0.

	if (cosTheta <= Math::Float(0))
	{
		// p is on behind the camera
		return Math::Float(0);
	}

	if (Math::Abs(Math::Float(cosTheta - Math::Float(1))) < Math::Constants::EpsLarge())
	{
		// Clamp to one to avoid nasty error introduced by 1/cos^3
		// cf. BPTFullpathTest - EvaluateFullpathPDFRatio (3) fails without this compensation
		// TODO : Check if this is valid
		cosTheta = Math::Float(1);
	}

	Math::Float invCosTheta = Math::Float(1) / cosTheta;
	return invA * invCosTheta * invCosTheta * invCosTheta;
}

Math::PDFEval PerspectiveCamera::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	if ((query.type & BSDFTypes()) == 0 || (query.transportDir != TransportDirection::EL))
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	// Reference point in camera coordinates
	auto refCam4 = viewMatrix * Math::Vec4(geom.p + query.wo, Math::Float(1));
	auto refCam3 = Math::Vec3(refCam4);

	// Reference point in NDC
	auto refNdc4 = projectionMatrix * refCam4;
	auto refNdc3 = Math::Vec3(refNdc4) / refNdc4.w;

	// Raster position in [0, 1]^2
	auto rasterPos = (Math::Vec2(refNdc3.x, refNdc3.y) + Math::Vec2(Math::Float(1))) / Math::Float(2);

	// Check visibility
	if (rasterPos.x < 0 || rasterPos.x > 1 || rasterPos.y < 0 || rasterPos.y > 1)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	// NOTE : This PDF evaluation is vulnerable with FP precision
	return Math::PDFEval(
		EvaluateImportance(-Math::CosThetaZUp(Math::Normalize(refCam3))),
		Math::ProbabilityMeasure::ProjectedSolidAngle);
}

LM_COMPONENT_REGISTER_IMPL(PerspectiveCamera, Camera);

LM_NAMESPACE_END
