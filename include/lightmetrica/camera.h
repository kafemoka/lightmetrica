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
#ifndef __LIB_LIGHTMETRICA_CAMERA_H__
#define __LIB_LIGHTMETRICA_CAMERA_H__

#include "asset.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

class Film;
struct Ray;
struct Primitive;

/*!
	Camera.
	A base class of the cameras.
*/
class LM_PUBLIC_API Camera : public Asset
{
public:

	Camera(const std::string& id);
	virtual ~Camera();

public:

	virtual std::string Name() const { return "camera"; }

public:

	/*!
		Sample a position on the camera.
		\param sampleP Position sample.
		\param p Sampled position.
		\param gn Geometry normal at #p (if defined).
		\param pdf Evaluated PDF (area measure).
	*/
	virtual void SamplePosition(const Math::Vec2& sampleP, Math::Vec3& p, Math::Vec3& gn, Math::PDFEval& pdf) const = 0;

	/*!
		Sample a outgoing direction.
		\param sampleD Direction sample (same as raster position).
		\param p Origin of the ray.
		\param gn Geometry normal at #p (if defined).
		\param d Sampled direction.
		\param pdf Evaluated PDF (solid angle measure).
	*/
	virtual void SampleDirection(const Math::Vec2& sampleD, const Math::Vec3& p, const Math::Vec3& gn, Math::Vec3& d, Math::PDFEval& pdf) const = 0;

	/*!
		Evaluate the importance.
		Evaluate the importance W_e(x_0\to x_1).
		\param p Position on the camera.
		\param d Outgoing direction x_0\to x_1 (must be normalized).
		\return Importance.
	*/
	virtual Math::Vec3 EvaluateWe(const Math::Vec3& p, const Math::Vec3& d) const = 0;

	/*!
		Evaluate the positional component of the importance.
		\param p Position on the camera.
		\return Positional component of the importance.
	*/
	virtual Math::Vec3 EvaluatePositionalWe(const Math::Vec3& p) const = 0;

	/*!
		Evaluate the directional component of the importance.
		\param p Position on the camera.
		\param d Outgoing direction at #p.
		\return Directional component of the importance.
	*/
	virtual Math::Vec3 EvaluateDirectionalWe(const Math::Vec3& p, const Math::Vec3& d) const = 0;

	/*!
		Convert a ray to a raster position.
		The function calculates the raster position from the outgoing ray.
		Returns false if calculated raster position is the outside of [0, 1]^2.
		\param p Position on the camera.
		\param d Outgoing direction from #p.
		\param rasterPos Raster position.
		\return true Succeeded to convert.
		\return false Failed to convert.
	*/
	virtual bool RayToRasterPosition(const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos) const = 0;

	/*!
		Get film.
		Returns the film referenced by the camera.
		\param Film.
	*/
	virtual Film* GetFilm() const = 0;
	
	/*!
		Register an reference to the primitive.
		Some implementation of camera needs transformed mesh information for sampling.
		The function registers the reference to the primitive.
		The function is internally called.
		\param primitives An instances of the primitive.
	*/
	virtual void RegisterPrimitive(const Primitive* primitive) = 0;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_CAMERA_H__