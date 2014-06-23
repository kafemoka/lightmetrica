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
#include <lightmetrica/renderer.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/random.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/film.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Photon.
	Represents single photon.
*/
struct Photon
{
	Math::Vec3 p;				// Surface point
	Math::Vec3 throughput;		// Current throughput
	Math::Vec3 wi;				// Incident ray direction
};

// --------------------------------------------------------------------------------

/*!
	Photon map.
	Interface for photon map.
*/
class PhotonMap
{
public:

	virtual void Build(const std::vector<Photon>& photons) = 0;
	virtual void CollectPhotons(int n, const Math::Vec3& p, std::vector<const Photon*>& collectedPhotons, Math::Float& maxDist2) const = 0;

};

/*!
	Naive photon map.
	Implements photon map in a naive way for debugging.
	This is VERY SLOW.
*/
class NaivePhotonMap : public PhotonMap
{
public:

	virtual void Build(const std::vector<Photon>& photons) { this->photons = photons; }
	virtual void CollectPhotons(int n, const Math::Vec3& p, std::vector<const Photon*>& collectedPhotons, Math::Float& maxDist2) const
	{
		std::vector<size_t> photonIdx(photons.size());
		for (size_t i = 0; i < photons.size(); i++)
		{
			photonIdx[i] = i;
		}

		std::sort(photonIdx.begin(), photonIdx.end(), [&](size_t v1, size_t v2)
		{
			return Math::Length2(photons[v1].p - p) < Math::Length2(photons[v2].p - p);
		});

		int found = 0;
		for (size_t i : photonIdx)
		{
			collectedPhotons.push_back(&photons.at(i));
			if (++found >= n)
			{
				break;
			}
		}

		maxDist2 = Math::Length2(collectedPhotons.back()->p - p);
	}

private:

	std::vector<Photon> photons;

};

// --------------------------------------------------------------------------------

/*!
	Photon mapping renderer.
	Implements photon mapping. This is unoptimized version.
	Reference:
		H. W. Jensen, Global illumination using photon maps,
		Procs. of the Eurographics Workshop on Rendering Techniques 96, pp.21-30, 1996.
*/
class PhotonMappingRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pm");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& scene );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	void TracePhotons(const Scene& scene, std::vector<Photon>& photons, long long& tracedPaths) const;
	void RenderProcessSingleSample(const Scene& scene, Random& rng, Film& film) const;
	Math::Float EvaluatePhotonDensityEstimationKernel(const Math::Vec3& p, const Photon* photon, const Math::Float& queryDist2) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;				// Number of samples
	long long numPhotonTraceSamples;	// Number of samples emitted in photon tracing step
	long long maxPhotons;				// Maximum number of photons stored in photon map
	int maxPhotonTraceDepth;			// Maximum depth in photon tracing step
	int numNNQueryPhotons;				// Number of photon to be collected in NN query
	//Math::Float maxNNQueryDist2;		// Maximum distance between query point and photons in photon map (squared)
	int numThreads;						// Number of threads
	long long samplesPerBlock;			// Samples to be processed per block
	std::string rngType;				// Type of random number generator

private:

	NaivePhotonMap photonMap;
	long long tracedLightPaths;

};

bool PhotonMappingRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("num_photon_trace_samples", 1LL, numPhotonTraceSamples);
	node.ChildValueOrDefault("max_photons", 1LL, maxPhotons);
	node.ChildValueOrDefault("max_photon_trace_depth", -1, maxPhotonTraceDepth);
	node.ChildValueOrDefault("num_nn_query_photons", 50, numNNQueryPhotons);

	//Math::Float maxNNQueryDist; 
	//node.ChildValueOrDefault("num_nn_query_dist", Math::Float(0.1), maxNNQueryDist);
	//maxNNQueryDist2 = maxNNQueryDist * maxNNQueryDist;

	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}
	node.ChildValueOrDefault("samples_per_block", 100LL, samplesPerBlock);
	if (samplesPerBlock <= 0)
	{
		LM_LOG_ERROR("Invalid value for 'samples_per_block'");
		return false;
	}
	node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
	if (!ComponentFactory::CheckRegistered<Random>(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

	return true;
}

bool PhotonMappingRenderer::Preprocess( const Scene& scene )
{
	signal_ReportProgress(0, false);

	// Photon tracing
	std::vector<Photon> photons;
	{
		LM_LOG_INFO("Tracing photons");
		LM_LOG_INDENTER();

		tracedLightPaths = 0;
		photons.reserve(maxPhotons);
		TracePhotons(scene, photons, tracedLightPaths);

		LM_LOG_INFO("Completed");
		LM_LOG_INFO("Traced " + std::to_string(tracedLightPaths) + " light paths");
		LM_LOG_INFO("Stored " + std::to_string(photons.size()) + " photons");
	}


	// Build photon map
	{
		LM_LOG_INFO("Building photon map");
		LM_LOG_INDENTER();

		photonMap.Build(photons);
		
		LM_LOG_INFO("Completed");
	}

	signal_ReportProgress(1, true);
	return true;
}

bool PhotonMappingRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<std::unique_ptr<Random>> rngs;
	std::vector<std::unique_ptr<Film>> films;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		rngs.emplace_back(ComponentFactory::Create<Random>(rngType));
		rngs.back()->SetSeed(seed + i);
		films.emplace_back(masterFilm->Clone());
	}

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	// --------------------------------------------------------------------------------

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& rng = rngs[threadId];
		auto& film = films[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			RenderProcessSingleSample(scene, *rng, *film);
		}

		processedBlocks++;
		auto progress = static_cast<double>(processedBlocks) / blocks;
		signal_ReportProgress(progress, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(*f.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	return true;
}

void PhotonMappingRenderer::RenderProcessSingleSample( const Scene& scene, Random& rng, Film& film ) const
{
	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE;
	scene.MainCamera()->SamplePosition(rng.NextVec2(), geomE, pdfPE);

	// Evaluate positional component of We
	auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

	auto throughput = positionalWe / pdfPE.v;
	auto currGeom = geomE;
	Math::Vec3 currWi;
	const GeneralizedBSDF* currBsdf = scene.MainCamera();
	Math::Vec2 rasterPos;
	Math::Vec3 L;
	std::vector<const Photon*> collectedPhotons;
	collectedPhotons.reserve(numNNQueryPhotons);

	while (true)
	{
		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.uComp = rng.Next();
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = currWi;

		GeneralizedBSDFSampleResult bsdfSR;
		if (!currBsdf->SampleDirection(bsdfSQ, currGeom, bsdfSR))
		{
			break;
		}

		// Evaluate generalized BSDF
		auto fs = currBsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), currGeom);
		if (Math::IsZero(fs))
		{
			break;
		}

		// Compute raster position if needed
		if (currBsdf == scene.MainCamera())
		{
			if (!scene.MainCamera()->RayToRasterPosition(currGeom.p, bsdfSR.wo, rasterPos))
			{
				break;
			}
		}

		// Update throughput
		throughput *= fs / bsdfSR.pdf.v;

		// --------------------------------------------------------------------------------

		// Setup next ray
		Ray ray;
		ray.d = bsdfSR.wo;
		ray.o = currGeom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// Intersection query
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

		// Intersected with light
		// ES*L paths are handled separately
		const auto* light = isect.primitive->light;
		if (light)
		{
			// Evaluate Le
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.type = GeneralizedBSDFType::LightDirection;
			bsdfEQ.wo = -ray.d;
			auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
			auto LeP = light->EvaluatePosition(isect.geom);
			L += throughput * LeD * LeP;
		}

		// --------------------------------------------------------------------------------

		// If intersected surface is non-specular, compute radiance from photon map
		if ((isect.primitive->bsdf->BSDFTypes() & GeneralizedBSDFType::Specular) == 0)
		{
			// Collect near photons
			Math::Float maxDist2;
			collectedPhotons.clear();
			photonMap.CollectPhotons(numNNQueryPhotons, isect.geom.p, collectedPhotons, maxDist2);

			// Density estimation
			for (const auto* photon : collectedPhotons)
			{
				auto k = EvaluatePhotonDensityEstimationKernel(isect.geom.p, photon, maxDist2);
				auto p = k / (maxDist2 * tracedLightPaths);

				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.type = GeneralizedBSDFType::AllBSDF;
				bsdfEQ.wi = -ray.d;
				bsdfEQ.wo = photon->wi;
				auto fs = isect.primitive->bsdf->EvaluateDirection(bsdfEQ, isect.geom);
				if (Math::IsZero(fs))
				{
					continue;
				}

				L += throughput * p * fs * photon->throughput;
			}

			break;
		}

		// --------------------------------------------------------------------------------

		// Update information
		currGeom = isect.geom;
		currWi = -ray.d;
		currBsdf = isect.primitive->bsdf;
	}

	// Record to film
	film.AccumulateContribution(rasterPos, L);
}

Math::Float PhotonMappingRenderer::EvaluatePhotonDensityEstimationKernel( const Math::Vec3& p, const Photon* photon, const Math::Float& queryDist2 ) const
{
	auto s = (Math::Float(1) - Math::Length2(photon->p - p) / queryDist2);
	return Math::Float(3) * Math::Constants::InvPi() * s * s;
}

void PhotonMappingRenderer::TracePhotons( const Scene& scene, std::vector<Photon>& photons, long long& tracedPaths ) const
{
	// Random number generators
	std::unique_ptr<Random> rng(ComponentFactory::Create<Random>(rngType));
	rng->SetSeed(static_cast<unsigned int>(std::time(nullptr)));

	for (long long sample = 0; sample < numPhotonTraceSamples && static_cast<long long>(photons.size()) < maxPhotons; sample++)
	{
		tracedPaths++;

		SurfaceGeometry geomL;
		Math::PDFEval pdfPL;

		// Sample a position on the light
		auto lightSampleP = rng->NextVec2();
		Math::PDFEval lightSelectionPdf;
		const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
		light->SamplePosition(lightSampleP, geomL, pdfPL);
		pdfPL.v *= lightSelectionPdf.v;

		// Evaluate positional component of Le
		auto positionalLe = light->EvaluatePosition(geomL);

		// Trace light particle and evaluate importance
		auto throughput = positionalLe / pdfPL.v;
		auto currGeom = geomL;
		Math::Vec3 currWi;
		const GeneralizedBSDF* currBsdf = light;
		int depth = 0;

		while (true)
		{
			// Sample generalized BSDF
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = TransportDirection::LE;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = currWi;

			GeneralizedBSDFSampleResult bsdfSR;
			if (!currBsdf->SampleDirection(bsdfSQ, currGeom, bsdfSR))
			{
				break;
			}

			// Evaluate generalized BSDF
			auto fs = currBsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), currGeom);
			if (Math::IsZero(fs))
			{
				break;
			}

			auto nextThroughput = throughput * fs / bsdfSR.pdf.v;

			// Russian roulette for path termination
			if (depth >= 1)
			{
				auto continueProb = Math::Min(Math::Float(1), Math::Luminance(nextThroughput) / Math::Luminance(throughput));
				if (rng->Next() > continueProb)
				{
					break;
				}

				throughput = nextThroughput / continueProb;
			}
			else
			{
				throughput = nextThroughput;
			}

			// --------------------------------------------------------------------------------

			// Setup next ray
			Ray ray;
			ray.d = bsdfSR.wo;
			ray.o = currGeom.p;
			ray.minT = Math::Constants::Eps();
			ray.maxT = Math::Constants::Inf();

			// Intersection query
			Intersection isect;
			if (!scene.Intersect(ray, isect))
			{
				break;
			}

			// --------------------------------------------------------------------------------

			// If intersected surface is non-specular, store the photon into photon map
			if ((isect.primitive->bsdf->BSDFTypes() & GeneralizedBSDFType::Specular) == 0)
			{
				Photon photon;
				photon.p = isect.geom.p;
				photon.throughput = throughput;
				photon.wi = -ray.d;
				photons.push_back(photon);
				if (photons.size() == maxPhotons)
				{
					break;
				}
			}

			// --------------------------------------------------------------------------------

			// Update information
			currGeom = isect.geom;
			currWi = -ray.d;
			currBsdf = isect.primitive->bsdf;
			depth++;
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(PhotonMappingRenderer, Renderer);

LM_NAMESPACE_END