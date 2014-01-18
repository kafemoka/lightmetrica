/*
	L I G H T  M E T R I C A

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
#include <lightmetrica/bvhscene.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/triaccel.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/aabb.h>
#include <thread>

LM_NAMESPACE_BEGIN

struct BVHNode
{

	enum class NodeType
	{
		Leaf,
		Internal
	};

	BVHNode(int begin, int end, const AABB& bound)
		: type(NodeType::Leaf)
		, begin(begin)
		, end(end)
		, bound(bound)
	{

	}

	BVHNode(int splitAxis, const std::shared_ptr<BVHNode>& left, const std::shared_ptr<BVHNode>& right)
		: type(NodeType::Internal)
		, splitAxis(splitAxis)
		, left(left)
		, right(right)
	{
		bound = left->bound.Union(right->bound);
	}

	NodeType type;
	AABB bound;

	// Leaf node data
	// Primitives index in [begin, end)
	int begin, end;

	// Internal node data
	std::shared_ptr<BVHNode> left, right;
	int splitAxis;

};

struct BVHBuildData
{
	std::vector<AABB> triBounds;					// Bounds of the triangles
	std::vector<Math::Vec3> triBoundCentroids;		// Centroids of the bounds of the triangles
};

class CompareToBucket
{
public:

	CompareToBucket(int splitAxis, int numBuckets, int minCostIdx, const BVHBuildData& data, const AABB& centroidBound)
		: splitAxis(splitAxis)
		, numBuckets(numBuckets)
		, minCostIdx(minCostIdx)
		, data(data)
		, centroidBound(centroidBound)
	{

	}

	bool operator()(int i) const
	{
		int bucketIdx = 
			Math::Cast<int>(Math::Float(
				Math::Float(numBuckets) * ((data.triBoundCentroids[i][splitAxis] - centroidBound.min[splitAxis])
					/ (centroidBound.max[splitAxis] - centroidBound.min[splitAxis]))));

		bucketIdx = Math::Min(bucketIdx, numBuckets - 1);
		return bucketIdx <= minCostIdx;
	}

private:

	int splitAxis;
	int numBuckets;
	int minCostIdx;
	const BVHBuildData& data;
	const AABB& centroidBound;

};

struct BVHTraversalData
{

	BVHTraversalData(Ray& ray)
		: ray(ray)
	{
		invRayDir = Math::Vec3(
			Math::Float(Math::Float(1) / ray.d.x),
			Math::Float(Math::Float(1) / ray.d.y),
			Math::Float(Math::Float(1) / ray.d.z));

		rayDirNegative = Math::Vec3i(
			ray.d.x < Math::Float(0),
			ray.d.y < Math::Float(0),
			ray.d.z < Math::Float(0));
	}

	Ray& ray;
	Math::Vec3i rayDirNegative;			// Each component of the rayDir is negative
	Math::Vec3 invRayDir;				// Inverse of the rayDir

	// The following data is filled when intersected
	unsigned int intersectedTriIdx;		// Intersected triangle index
	Math::Vec2 intersectedTriB;			// Intersected triangle's barycentric coordinates

};

// --------------------------------------------------------------------------------

class BVHScene::Impl : public Object
{
public:

	Impl(BVHScene* self);
	bool Build();
	bool Intersect(Ray& ray, Intersection& isect) const;
	boost::signals2::connection Connect_ReportBuildProgress(const std::function<void (double, bool)>& func) { return signal_ReportBuildProgress.connect(func); }

private:

	bool Intersect(const std::shared_ptr<BVHNode>& node, BVHTraversalData& data) const;
	bool Intersect(const AABB& bound, BVHTraversalData& data) const;
	std::shared_ptr<BVHNode> Build(const BVHBuildData& data, int begin, int end);
	void LoadPrimitives(const std::string& scenePath);
	
private:

	// The function is called when a leaf node is created
	// report progress w.r.t. # of triangles fixed as leafs
	void ReportProgress(int begin, int end);
	void ResetProgress();

private:

	BVHScene* self;
	const int maxTriInNode;
	std::vector<int> bvhTriIndices;
	std::shared_ptr<BVHNode> root;
	std::vector<TriAccel> triAccels;
	boost::signals2::signal<void (double, bool)> signal_ReportBuildProgress;
	int numProcessedTris;

};

BVHScene::Impl::Impl( BVHScene* self )
	: self(self)
	, maxTriInNode(255)
{

}

bool BVHScene::Impl::Build()
{
	BVHBuildData data;

	{
		LM_LOG_INFO("Creating triaccels");
		LM_LOG_INDENTER();

		for (int i = 0; i < self->NumPrimitives(); i++)
		{
			const auto* primitive = self->PrimitiveByIndex(i);
			const auto* mesh = primitive->mesh;
			if (mesh)
			{
				// Enumerate all triangles and create triaccels
				const auto* positions = mesh->Positions();
				const auto* faces = mesh->Faces();
				for (int j = 0; j < mesh->NumFaces() / 3; j++)
				{
					int triIdx = static_cast<int>(triAccels.size());

					// Create triaccel
					triAccels.push_back(TriAccel());
					triAccels.back().shapeIndex = j;
					triAccels.back().primIndex = i;
					unsigned int i1 = faces[3*j  ];
					unsigned int i2 = faces[3*j+1];
					unsigned int i3 = faces[3*j+2];
					Math::Vec3 p1(primitive->transform * Math::Vec4(positions[3*i1], positions[3*i1+1], positions[3*i1+2], Math::Float(1)));
					Math::Vec3 p2(primitive->transform * Math::Vec4(positions[3*i2], positions[3*i2+1], positions[3*i2+2], Math::Float(1)));
					Math::Vec3 p3(primitive->transform * Math::Vec4(positions[3*i3], positions[3*i3+1], positions[3*i3+2], Math::Float(1)));
					triAccels.back().Load(p1, p2, p3);

					// Initial index
					bvhTriIndices.push_back(triIdx);

					// Create primitive bound from points
					AABB triBound(p1, p2);
					triBound = triBound.Union(p3);

					data.triBounds.push_back(triBound);
					data.triBoundCentroids.push_back((triBound.min + triBound.max) * Math::Float(0.5));
				}
			}
		}

		LM_LOG_INFO("Successfully created " + std::to_string(triAccels.size()) + " triaccels");
	}

	// Build BVH
	{
		LM_LOG_INFO("Building BVH");
		LM_LOG_INDENTER();

		ResetProgress();

		auto start = std::chrono::high_resolution_clock::now();
		root = Build(data, 0, static_cast<int>(triAccels.size()));
		auto end = std::chrono::high_resolution_clock::now();

		double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.0;
		LM_LOG_INFO("Completed in " + std::to_string(elapsed) + " seconds");
	}

	return true;
}

std::shared_ptr<BVHNode> BVHScene::Impl::Build( const BVHBuildData& data, int begin, int end )
{
	std::shared_ptr<BVHNode> node;	

	// Bound of the primitive [begin, end)
	AABB bound;
	for (int i = begin; i < end; i++)
	{
		bound = bound.Union(data.triBounds[bvhTriIndices[i]]);
	}

	// Number of primitives in the node
	int numPrimitives = end - begin;
	if (numPrimitives == 1)
	{
		// Leaf node
		node = std::make_shared<BVHNode>(begin, end, bound);
		ReportProgress(begin, end);
	}
	else
	{
		// Internal node

		// Choose the axis to split
		AABB centroidBound;
		for (int i = begin; i < end; i++)
		{
			centroidBound = centroidBound.Union(data.triBoundCentroids[bvhTriIndices[i]]);
		}

		// Select split axis
		int splitAxis = centroidBound.LongestAxis();

		// If the centroid bound according to the split axis is degenerated, take the node as a leaf.
		if (centroidBound.min[splitAxis] == centroidBound.max[splitAxis])
		{
			node = std::make_shared<BVHNode>(begin, end, bound);
			ReportProgress(begin, end);
		}
		else
		{
			// Split primitives using SAH, surface area heuristic.
			// Considering all possible partitions is rather heavy in the computation cost,
			// so in the application the primitives is separated to some buckets according to the split axis
			// and reduce the combination of the partitions.
			
			// Create buckets
			const int numBuckets = 12;
			AABB bucketTriBound[numBuckets];
			int bucketTriCount[numBuckets] = {0};
			for (int i = begin; i < end; i++)
			{
				int bucketIdx = Math::Cast<int>(Math::Float(
					Math::Float(numBuckets) * ((data.triBoundCentroids[bvhTriIndices[i]][splitAxis] - centroidBound.min[splitAxis])
						/ (centroidBound.max[splitAxis] - centroidBound.min[splitAxis]))));

				bucketIdx = Math::Min(bucketIdx, numBuckets - 1);
				bucketTriCount[bucketIdx]++;
				bucketTriBound[bucketIdx] = bucketTriBound[bucketIdx].Union(data.triBounds[bvhTriIndices[i]]);
			}

			// For each partition compute the cost
			// Note that the number of possible partitions is numBuckets - 1
			Math::Float costs[numBuckets - 1];
			for (int i = 0; i < numBuckets - 1; i++)
			{
				AABB b1;
				AABB b2;
				int count1 = 0;
				int count2 = 0;

				// [0, i]
				for (int j = 0; j <= i; j++)
				{
					b1 = b1.Union(bucketTriBound[j]);
					count1 += bucketTriCount[j];
				}

				// (i, numBuckets - 1]
				for (int j = i + 1; j < numBuckets; j++)
				{
					b2 = b2.Union(bucketTriBound[j]);
					count2 += bucketTriCount[j];
				}

				// Assume the intersection cost is 1 and traversal cost is 1/8.
				costs[i] = Math::Float(0.125) + (Math::Float(count1) * b1.SurfaceArea() + Math::Float(count2) * b2.SurfaceArea()) / bound.SurfaceArea();
			}

			// Find minimum partition
			int minCostIdx = 0;
			double minCost = costs[0];
			for (int i = 1; i < numBuckets - 1; i++)
			{
				if (minCost > costs[i])
				{
					minCost = costs[i];
					minCostIdx = i;
				}
			}

			// Partition if the minimum cost is lower than the leaf cost (numPrimitives)
			// or the current number of primitives is higher than the limit.
			if (minCost < Math::Float(numPrimitives) || numPrimitives > maxTriInNode)
			{
				int mid = static_cast<int>(std::partition(&bvhTriIndices[begin], &bvhTriIndices[end - 1] + 1, CompareToBucket(splitAxis, numBuckets, minCostIdx, data, centroidBound)) - &bvhTriIndices[0]);
				node = std::make_shared<BVHNode>(splitAxis, Build(data, begin, mid), Build(data, mid, end));
			}
			else
			{
				// Otherwise make leaf node
				node = std::make_shared<BVHNode>(begin, end, bound);
				ReportProgress(begin, end);
			}
		}
	}

	return node;
}

bool BVHScene::Impl::Intersect( Ray& ray, Intersection& isect ) const
{
	BVHTraversalData data(ray);

	if (Intersect(root, data))
	{
		// Store required data for the intersection structure
		auto& triAccel = triAccels[data.intersectedTriIdx];
		self->StoreIntersectionFromBarycentricCoords(triAccel.primIndex, triAccel.shapeIndex, ray, data.intersectedTriB, isect);
		return true;
	}

	return false;
}

bool BVHScene::Impl::Intersect( const std::shared_ptr<BVHNode>& node, BVHTraversalData& data ) const
{
	bool intersected = false;

	// Check intersection to the node bound
	if (Intersect(node->bound, data))
	{
		if (node->type == BVHNode::NodeType::Leaf)
		{
			// Leaf node
			// Intersection with the primitives hold in the node
			for (int i = node->begin; i < node->end; i++)
			{
				Math::Float t;
				Math::Vec2 b;
				if (triAccels[bvhTriIndices[i]].Intersect(data.ray, data.ray.minT, data.ray.maxT, b[0], b[1], t))
				{
					data.ray.maxT = t;
					data.intersectedTriIdx = bvhTriIndices[i];
					data.intersectedTriB = b;
					intersected = true;
				}
			}
		}
		else
		{
			// Internal node
			// Traverse the right node first if the ray direction according to the
			// split axis is negative.
			if (data.rayDirNegative[node->splitAxis])
			{
				intersected |= Intersect(node->right, data);
				intersected |= Intersect(node->left, data);
			}
			else
			{
				intersected |= Intersect(node->left, data);
				intersected |= Intersect(node->right, data);
			}
		}
	}

	return intersected;
}

bool BVHScene::Impl::Intersect( const AABB& bound, BVHTraversalData& data ) const
{
	auto& rayDirNegative = data.rayDirNegative;
	auto& invRayDir = data.invRayDir;
	auto& ray = data.ray;

	Math::Float tmin =  (bound[    rayDirNegative[0]].x - ray.o.x) * invRayDir.x;
	Math::Float tmax =  (bound[1 - rayDirNegative[0]].x - ray.o.x) * invRayDir.x;
	Math::Float tymin = (bound[    rayDirNegative[1]].y - ray.o.y) * invRayDir.y;
	Math::Float tymax = (bound[1 - rayDirNegative[1]].y - ray.o.y) * invRayDir.y;

	if ((tmin > tymax) || (tymin > tmax)) return false;
	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	// Check for ray intersection against z slab
	Math::Float tzmin = (bound[    rayDirNegative[2]].z - ray.o.z) * invRayDir.z;
	Math::Float tzmax = (bound[1 - rayDirNegative[2]].z - ray.o.z) * invRayDir.z;

	if ((tmin > tzmax) || (tzmin > tmax)) return false;
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;

	return (tmin < ray.maxT) && (tmax > ray.minT);
}

void BVHScene::Impl::ReportProgress( int begin, int end )
{
	numProcessedTris += end - begin;
	signal_ReportBuildProgress(static_cast<double>(numProcessedTris) / triAccels.size(), numProcessedTris == triAccels.size());
}

void BVHScene::Impl::ResetProgress()
{
	numProcessedTris = 0;
	signal_ReportBuildProgress(0, false);
}

// --------------------------------------------------------------------------------

BVHScene::BVHScene()
	: p(new Impl(this))
{

}

BVHScene::~BVHScene()
{
	LM_SAFE_DELETE(p);
}

bool BVHScene::Build()
{
	return p->Build();
}

bool BVHScene::Intersect( Ray& ray, Intersection& isect ) const
{
	return p->Intersect(ray, isect);
}

boost::signals2::connection BVHScene::Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportBuildProgress(func);
}

LM_NAMESPACE_END