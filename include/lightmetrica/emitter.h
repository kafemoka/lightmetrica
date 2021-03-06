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

#pragma once
#ifndef LIB_LIGHTMETRICA_EMITTER_H
#define LIB_LIGHTMETRICA_EMITTER_H

#include "generalizedbsdf.h"
#include "aabb.h"

LM_NAMESPACE_BEGIN

struct Primitive;
struct SurfaceGeometry;
class Scene;
class EmitterShape;

/*!
	Emitter.
	The base class of Light and Camera.
*/
class Emitter : public GeneralizedBSDF
{
public:

	Emitter() {}
	virtual ~Emitter() {}

public:

	/*!
		Sample a position on the light.
		\param sample Position sample.
		\param geom Surface geometry at sampled position #geom.p.
		\param pdf Evaluated PDF (area measure).
	*/
	virtual void SamplePosition(const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf) const = 0;

	/*!
		Evaluate the positional component of the emitted quantity.
		\param geom Surface geometry.
		\return Positional component of the emitted quantity.
	*/
	virtual Math::Vec3 EvaluatePosition(const SurfaceGeometry& geom) const = 0;

	/*!
		Evaluate positional PDF.
		\param geom Surface geometry.
		\return Evaluated PDF.
	*/
	virtual Math::PDFEval EvaluatePositionPDF(const SurfaceGeometry& geom) const = 0;

	/*!
		Register an reference to the primitive.
		Some implementation of camera needs transformed mesh information for sampling.
		The function registers the reference to the primitive.
		The function is internally called.
		\param primitives An list instances of the primitive.
	*/
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives) = 0;

	/*!
		Post configuration after scene is built.
		Some emitter needs to be configured after scene is built.
		TODO : Replace RegisterPrimitive function with this function.
		\param scene Scene.
	*/
	virtual void PostConfigure(const Scene& scene) = 0;

	/*!
		Get associated emitter shapes.
		Some emitter is associated with intersectable shapes
		(e.g., sphere for environment lights) in order to integrate emitters
		with BPT based rendering techniques.
		Ownership of the created instance is delegated to the caller.
		\return Instance of emitter shape.
	*/
	virtual EmitterShape* CreateEmitterShape() const = 0;

	/*!
	*/
	virtual AABB GetAABB() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_EMITTER_H