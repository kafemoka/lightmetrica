/*
	nanon : A research-oriented renderer

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
#include "base.h"
#include "base.math.h"
#include "stub.bsdf.h"
#include "stub.trianglemesh.h"
#include <nanon/scenefactory.h>
#include <nanon/naivescene.h>
#include <nanon/primitive.h>
#include <nanon/ray.h>
#include <nanon/intersection.h>
#include <nanon/math.functions.h>
#include <random>

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

// {(x, y, z) : 0<=x,y<=1, z=0,-1}
class StubTriangleMesh_Simple : public StubTriangleMesh
{
public:

	StubTriangleMesh_Simple()
		: StubTriangleMesh("simple")
	{
		positions.push_back(Math::Vec3(0, 0, 0));
		positions.push_back(Math::Vec3(1, 0, 0));
		positions.push_back(Math::Vec3(1, 1, 0));
		positions.push_back(Math::Vec3(0, 1, 0));
		positions.push_back(Math::Vec3(0, 0, -1));
		positions.push_back(Math::Vec3(1, 0, -1));
		positions.push_back(Math::Vec3(1, 1, -1));
		positions.push_back(Math::Vec3(0, 1, -1));
		for (int i = 0; i < 8; i++) normals.push_back(Math::Vec3(0, 0, 1));
		for (int i = 0; i < 2; i++)
		{
			texcoords.push_back(Math::Vec2(0, 0));
			texcoords.push_back(Math::Vec2(1, 0));
			texcoords.push_back(Math::Vec2(1, 1));
			texcoords.push_back(Math::Vec2(0, 1));
		}
		faces.push_back(Math::Vec3i(0, 1, 2));
		faces.push_back(Math::Vec3i(0, 2, 3));
		faces.push_back(Math::Vec3i(4, 5, 6));
		faces.push_back(Math::Vec3i(4, 6, 7));
	}

};

// {(x, y, z) : 0<=x,y<=1, x=-z}
class StubTriangleMesh_Simple2 : public StubTriangleMesh
{
public:

	StubTriangleMesh_Simple2()
		: StubTriangleMesh("simple2")
	{
		positions.push_back(Math::Vec3(0, 0, 0));
		positions.push_back(Math::Vec3(1, 0, -1));
		positions.push_back(Math::Vec3(1, 1, -1));
		positions.push_back(Math::Vec3(0, 1, 0));
		normals.push_back(Math::Normalize(Math::Vec3(1, 0, 1)));
		for (int i = 0; i < 3; i++) normals.push_back(normals[0]);
		for (int i = 0; i < 2; i++)
		{
			texcoords.push_back(Math::Vec2(0, 0));
			texcoords.push_back(Math::Vec2(1, 0));
			texcoords.push_back(Math::Vec2(1, 1));
			texcoords.push_back(Math::Vec2(0, 1));
		}
		faces.push_back(Math::Vec3i(0, 1, 2));
		faces.push_back(Math::Vec3i(0, 2, 3));
	}

};

// Many triangles in [0, 1]^3
class StubTriangleMesh_Random : public StubTriangleMesh
{
public:

	StubTriangleMesh_Random()
		: StubTriangleMesh("random")
	{
		// Fix seed
		std::mt19937 gen(42);
		std::uniform_real_distribution<double> dist;

		const int FaceCount = 100;
		for (int i = 0; i < FaceCount; i++)
		{
			auto p1 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
			auto p2 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
			auto p3 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));

			positions.push_back(p1);
			positions.push_back(p2);
			positions.push_back(p3);

			auto n = Math::Cross(p2 - p1, p3 - p1);
			for (int j = 0; j < 3; j++) normals.push_back(n);

			faces.push_back(Math::Vec3i(3*i, 3*i+1, 3*i+2));
		}
	}

};

// --------------------------------------------------------------------------------

class SceneIntersectionTest : public TestBase
{
public:

	SceneIntersectionTest()
	{
		// List of scene types to be tested
		sceneTypes.push_back("naive");

		// BSDF
		bsdf = new StubBSDF("test");
	}

	~SceneIntersectionTest()
	{
		NANON_SAFE_DELETE(bsdf);
	}

protected:

	std::shared_ptr<Scene> CreateAndSetupScene(const std::string& type, const TriangleMesh* mesh)
	{
		// Create scene
		auto scene = factory.Create(type);

		// Primitives for this test
		std::vector<std::shared_ptr<Primitive>> primitives;
		primitives.push_back(std::make_shared<Primitive>(Math::Mat4::Identity(), mesh, bsdf));

		// Load & build
		EXPECT_TRUE(scene->LoadPrimitives(primitives));
		EXPECT_TRUE(scene->Build());

		return scene;
	}

protected:

	std::vector<std::string> sceneTypes;
	StubBSDF* bsdf;
	SceneFactory factory;

};

TEST_F(SceneIntersectionTest, Intersect_Simple)
{
	for (const auto& type : sceneTypes)
	{
		// Triangle mesh and scene
		std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Simple());
		auto scene = CreateAndSetupScene(type, mesh.get());

		// Trace rays in the region of [0, 1]^2
		Ray ray;
		Intersection isect;
		const int Steps = 10;
		const Math::Float Delta = Math::Float(1) / Math::Float(Steps);
		for (int i = 1; i < Steps; i++)
		{
			const Math::Float y = Delta * Math::Float(i);
			for (int j = 1; j < Steps; j++)
			{
				const Math::Float x = Delta * Math::Float(j);

				// Intersection query
				ray.o = Math::Vec3(0, 0, 1);
				ray.d = Math::Normalize(Math::Vec3(x, y, 0) - ray.o);
				ray.minT = Math::Constants::Zero;
				ray.maxT = Math::Constants::Inf;

				ASSERT_TRUE(scene->Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, 0), isect.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.uv));
			}
		}
	}
}

TEST_F(SceneIntersectionTest, Intersect_Simple2)
{
	for (const auto& type : sceneTypes)
	{
		// Triangle mesh and scene
		std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Simple2());
		auto scene = CreateAndSetupScene(type, mesh.get());

		// Trace rays in the region of [0, 1]^2
		Ray ray;
		Intersection isect;
		const int Steps = 10;
		const Math::Float Delta = Math::Float(1) / Math::Float(Steps);
		for (int i = 1; i < Steps; i++)
		{
			const Math::Float y = Delta * Math::Float(i);
			for (int j = 1; j < Steps; j++)
			{
				const Math::Float x = Delta * Math::Float(j);

				// Intersection query
				ray.o = Math::Vec3(x, y, 1);
				ray.d = Math::Vec3(0, 0, -1);
				ray.minT = Math::Constants::Zero;
				ray.maxT = Math::Constants::Inf;

				ASSERT_TRUE(scene->Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, -x), isect.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(1, 0, 1)), isect.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(1, 0, 1)), isect.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.uv));
			}
		}
	}
}

// Check if all implementation returns the same result
TEST_F(SceneIntersectionTest, Consistency)
{
	// This test requires at least two implementations
	if (sceneTypes.size() >= 2)
	{
		// Result for each type (primitive ID)
		std::vector<std::vector<int>> results(sceneTypes.size());

		Ray ray;
		Intersection isect;

		for (size_t typeIdx = 0; typeIdx < sceneTypes.size(); typeIdx++)
		{
			const int Steps = 10;
			const Math::Float Delta = Math::Float(1) / Math::Float(Steps);

			for (int i = 1; i < Steps; i++)
			{
				const Math::Float y = Delta * Math::Float(i);

				for (int j = 1; j < Steps; j++)
				{
					const Math::Float x = Delta * Math::Float(j);

					// Triangle mesh and scene
					std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Random());
					auto scene = CreateAndSetupScene(sceneTypes[typeIdx], mesh.get());

					// Intersection query
					ray.o = Math::Vec3(x, y, 1);
					ray.d = Math::Vec3(0, 0, -1);
					ray.minT = Math::Constants::Zero;
					ray.maxT = Math::Constants::Inf;

					if (scene->Intersect(ray, isect))
					{
						// Store the triangle index to the result
						results[typeIdx].push_back(isect.triangleIndex);
					}
				}
			}
		}

		// Check consistency
		for (size_t i = 0; i < sceneTypes.size(); i++)
		{
			for (size_t j = i+1; j < sceneTypes.size(); j++)
			{
				// Number of intersected triangles and its triangle IDs must be same
				EXPECT_EQ(results[i], results[j]);
			}
		}
	}
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END