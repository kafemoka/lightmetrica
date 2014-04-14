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
#include <lightmetrica/pssmlt.h>
#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/hdrfilm.h>
#include <lightmetrica/random.h>
#include <lightmetrica/randomfactory.h>
#include <lightmetrica/align.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/light.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/align.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/bitmaptexture.h>
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>

#define LM_ENABLE_PSSMLT_EXPERIMENTAL

LM_NAMESPACE_BEGIN

/*
	Light path seed.
	Required data to generate a seed light path.
*/
struct PSSMLTPathSeed
{

	int index;			// Sample index of restorable sampler
	Math::Float I;		// Luminance of the sampled light path (used for debugging)

	PSSMLTPathSeed()
	{

	}

	PSSMLTPathSeed(int index, const Math::Float& I)
		: index(index)
		, I(I)
	{

	}

};

/*
	Light path sample record.
	Contains some information of sampled light paths.
*/
struct PSSMLTPathSampleRecord : public Object
{

	Math::Vec2 rasterPos;		// Raster position
	Math::Vec3 L;				// Sampled radiance

};

// --------------------------------------------------------------------------------

/*
	Per-thread data.
	Contains data associated with a thread.
*/
struct PSSMLTThreadContext : public SIMDAlignedType
{
	
	std::unique_ptr<Random> rng;						// Random number generator
	std::unique_ptr<Film> film;							// Film
	std::unique_ptr<PSSMLTPrimarySample> sampler;		// Kelemen's lazy sampler
	PSSMLTPathSampleRecord records[2];					// Path sample records (current or proposed)
	int current;										// Index of current record

	PSSMLTThreadContext(Random* rng, Film* film, PSSMLTPrimarySample* sampler)
		: rng(rng)
		, film(film)
		, sampler(sampler)
	{

	}

	PSSMLTThreadContext(PSSMLTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, sampler(std::move(context.sampler))
	{

	}

};

// --------------------------------------------------------------------------------

enum class PSSMLTEstimatorMode
{
	Normal,
	MeanValueSubstitution,
	MeanValueSubstitution_LargeStepMIS
};

class PSSMLTRenderer::Impl
{
public:

	Impl(PSSMLTRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	void GenerateAndSampleSeeds(const Scene& scene, PSSMLTRestorableSampler& restorableSampler, Math::Float& B, std::vector<PSSMLTPathSeed>& seeds) const;
	void SampleAndEvaluatePath(const Scene& scene, PSSMLTSampler& sampler, Math::Vec3& L, Math::Vec2& rasterPos) const;

private:

	PSSMLTRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;						// Number of sample mutations
	int rrDepth;								// Depth of beginning RR
	int numThreads;								// Number of threads
	long long samplesPerBlock;					// Samples to be processed per block
	std::string rngType;						// Type of random number generator

	PSSMLTEstimatorMode estimatorMode;			// Estimator mode
	long long numSeedSamples;					// Number of seed samples
	Math::Float largeStepProb;					// Large step mutation probability
	Math::Float kernelSizeS1;					// Minimum kernel size
	Math::Float kernelSizeS2;					// Maximum kernel size

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

PSSMLTRenderer::Impl::Impl( PSSMLTRenderer* self )
	: self(self)
{
	
}

bool PSSMLTRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	// Check type
	if (node.AttributeValue("type") != self->Type())
	{
		LM_LOG_ERROR("Invalid renderer type '" + node.AttributeValue("type") + "'");
		return false;
	}

	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
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
	if (!RandomFactory::CheckSupport(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

	// Estimator mode
	auto estimatorModeNode = node.Child("estimator_mode");
	if (estimatorModeNode.Empty())
	{
		estimatorMode = PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS;
		LM_LOG_WARN("Missing 'estimator_mode' element. Using default value.");
	}
	else
	{
		if (estimatorModeNode.Value() == "normal")
		{
			estimatorMode = PSSMLTEstimatorMode::Normal;
		}
		else if (estimatorModeNode.Value() == "mvs")
		{
			estimatorMode = PSSMLTEstimatorMode::MeanValueSubstitution;
		}
		else if (estimatorModeNode.Value() == "mvs_mis")
		{
			estimatorMode = PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS;
		}
		else
		{
			LM_LOG_ERROR("Invalid estimator mode '" + estimatorModeNode.Value() + "'");
			return false;
		}
	}

	node.ChildValueOrDefault("num_seed_samples", 1LL, numSeedSamples);
	node.ChildValueOrDefault("large_step_prob", Math::Float(0.1), largeStepProb);
	node.ChildValueOrDefault("kernel_size_s1", Math::Float(1.0 / 1024.0), kernelSizeS1);
	node.ChildValueOrDefault("kernel_size_s2", Math::Float(1.0 / 64.0), kernelSizeS2);

#if LM_EXPERIMENTAL_MODE
	// Experiments
	auto experimentsNode = node.Child("experiments");
	if (!experimentsNode.Empty())
	{
		LM_LOG_INFO("Configuring experiments");
		LM_LOG_INDENTER();

		if (!expts.Configure(experimentsNode, assets))
		{
			LM_LOG_ERROR("Failed to configure experiments");
			return false;
		}

		if (numThreads != 1)
		{
			LM_LOG_WARN("Number of thread must be 1 in experimental mode, forced 'num_threads' to 1");
			numThreads = 1;
		}
	}
#endif

	return true;
}

bool PSSMLTRenderer::Impl::Render( const Scene& scene )
{
	// Set number of threads
	omp_set_num_threads(numThreads);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Preprocess
	Math::Float B;
	std::vector<PSSMLTPathSeed> seeds;
	int seed = static_cast<int>(std::time(nullptr));
	PSSMLTRestorableSampler restorableSampler(RandomFactory::Create(rngType), seed++);

	{
		LM_LOG_INFO("Preprocessing");
		LM_LOG_INDENTER();
		LM_LOG_INFO("Generating " + std::to_string(numSeedSamples) + " seed samples");
		GenerateAndSampleSeeds(scene, restorableSampler, B, seeds);
		LM_LOG_INFO("Completed");
	}

	// --------------------------------------------------------------------------------

	// Setup thread context
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::vector<std::unique_ptr<PSSMLTThreadContext>> contexts;
	for (int i = 0; i < numThreads; i++)
	{
		// Add a entry to the context
		contexts.emplace_back(
			new PSSMLTThreadContext(
				RandomFactory::Create(rngType),
				masterFilm->Clone(),
				new PSSMLTPrimarySample(kernelSizeS1, kernelSizeS2)));
		contexts.back()->rng->SetSeed(seed++);
		
		// Setup initial state of the primary sample space sampler
		// by restoring the state of the initial path.

		// Set the sample index to the restorable sampler
		// and prepare for restoring the sampled light path.
		auto& context = contexts.back();
		restorableSampler.SetIndex(seeds[i].index);
		context->sampler->SetRng(restorableSampler.Rng());

		// Sample a seed light path and initialize the state of Kelemen's lazy sampler
		context->current = 0;
		auto& current = context->records[context->current];
		SampleAndEvaluatePath(scene, *context->sampler, current.L, current.rasterPos);
		LM_ASSERT(Math::Abs(seeds[i].I - Math::Luminance(current.L)) < Math::Constants::Eps());

		// Get back to the normal generator
		context->sampler->SetRng(context->rng.get());
	}

	// --------------------------------------------------------------------------------

	// Rendering
	std::atomic<long long> processedBlocks(0);								// # of processes blocks
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;	// # of blocks
	signal_ReportProgress(0, false);

	LM_LOG_INFO("Rendering");

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& context = contexts[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		LM_EXPT_UPDATE_PARAM(expts, "film", context->film.get());
		LM_EXPT_UPDATE_PARAM(expts, "pssmlt_primary_sample", context->sampler.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			auto& current = context->records[context->current];
			auto& proposed = context->records[1-context->current];

			// Enable flag for large step mutation
			// Note that the probability is not related to the state
			bool enableLargeStep = context->rng->Next() < largeStepProb;
			context->sampler->SetLargeStep(enableLargeStep);

			// Sample and evaluate proposed path
			SampleAndEvaluatePath(scene, *context->sampler, proposed.L, proposed.rasterPos);

			// Compute acceptance ratio
			auto currentI = Math::Luminance(current.L);
			auto proposedI = Math::Luminance(proposed.L);
			auto a = currentI > Math::Float(0)
				? Math::Min(Math::Float(1), proposedI / currentI)
				: Math::Float(1);

			if (context->rng->Next() < a)
			{
				// Accepted
				context->sampler->Accept();
				context->current = 1 - context->current;
			}
			else
			{
				// Rejected
				context->sampler->Reject();
			}

			// Accumulate contribution
			if (estimatorMode == PSSMLTEstimatorMode::MeanValueSubstitution)
			{
				context->film->AccumulateContribution(
					current.rasterPos,
					current.L * (1 - a) * B / currentI);
				context->film->AccumulateContribution(
					proposed.rasterPos,
					proposed.L * a * B / proposedI);
			}
			else if (estimatorMode == PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS)
			{
				context->film->AccumulateContribution(
					current.rasterPos,
					current.L * (1 - a) / (currentI / B + largeStepProb));
				context->film->AccumulateContribution(
					proposed.rasterPos,
					proposed.L * (a + (enableLargeStep ? Math::Float(1) : Math::Float(0))) / (proposedI / B + largeStepProb));
			}
			else if (estimatorMode == PSSMLTEstimatorMode::Normal)
			{
				auto& current = context->records[context->current];
				auto currentI = Math::Luminance(current.L);
				context->film->AccumulateContribution(
					current.rasterPos,
					current.L * B / currentI);
			}

			LM_EXPT_UPDATE_PARAM(expts, "sample", &sample);
			LM_EXPT_NOTIFY(expts, "SampleFinished");
		}

		processedBlocks++;
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(*context->film.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

	return true;
}

void PSSMLTRenderer::Impl::GenerateAndSampleSeeds( const Scene& scene, PSSMLTRestorableSampler& restorableSampler, Math::Float& B, std::vector<PSSMLTPathSeed>& seeds ) const
{
	// Generate candidates for seeds
	std::vector<PSSMLTPathSeed> candidates;
	Math::Float sumI(0);

	signal_ReportProgress(0, false);

	for (long long sample = 0; sample < numSeedSamples; sample++)
	{
		// Current sample index of #restorableSampler
		int index = restorableSampler.Index();

		// Sample a light path and evaluate radiance
		Math::Vec3 L;
		Math::Vec2 _;
		SampleAndEvaluatePath(scene, restorableSampler, L, _);

		if (!Math::IsZero(L))
		{
			auto I = Math::Luminance(L);
			sumI += I;
			candidates.emplace_back(index, I);
		}

		signal_ReportProgress(static_cast<double>(sample) / numSeedSamples, false);
	}
	
	// Compute #B
	B = sumI / Math::Float(numSeedSamples);

	// Sample seeds according to I

	// Create CDF
	std::vector<Math::Float> cdf;
	cdf.push_back(Math::Float(0));
	for (auto& candidate : candidates)
	{
		cdf.push_back(cdf.back() + candidate.I);
	}

	// Normalize
	auto sum = cdf.back();
	for (auto& v : cdf)
	{
		v /= sum;
	}

	// Sample seeds for each thread
	seeds.clear();
	LM_ASSERT(candidates.size() >= numThreads);
	for (int i = 0; i < numThreads; i++)
	{
		double u = restorableSampler.Next();
		int idx =
			Math::Clamp(
				static_cast<int>(std::upper_bound(cdf.begin(), cdf.end(), u) - cdf.begin()) - 1,
				0, static_cast<int>(cdf.size()) - 1);

		seeds.push_back(candidates[idx]);
	}
}

void PSSMLTRenderer::Impl::SampleAndEvaluatePath( const Scene& scene, PSSMLTSampler& sampler, Math::Vec3& L, Math::Vec2& rasterPos ) const
{
	// Raster position
	rasterPos = sampler.NextVec2();

	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfP;
	scene.MainCamera()->SamplePosition(sampler.NextVec2(), geomE, pdfP);

	// Sample ray direction
	GeneralizedBSDFSampleQuery bsdfSQ;
	GeneralizedBSDFSampleResult bsdfSR;
	bsdfSQ.sample = rasterPos;
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	scene.MainCamera()->SampleDirection(bsdfSQ, geomE, bsdfSR);

	// Construct initial ray
	Ray ray;
	ray.o = geomE.p;
	ray.d = bsdfSR.wo;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf();

	// Evaluate importance
	auto We =
		scene.MainCamera()->EvaluatePosition(geomE) *
		scene.MainCamera()->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geomE);

	L = Math::Vec3();
	Math::Vec3 throughput = We / bsdfSR.pdf.v / pdfP.v; // = 1 !!
	int depth = 0;

	while (true)
	{
		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

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

		// Sample BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler.NextVec2();
		bsdfSQ.type = GeneralizedBSDFType::AllBSDF;
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.wi = -ray.d;

		GeneralizedBSDFSampleResult bsdfSR;
		if (!isect.primitive->bsdf->SampleDirection(bsdfSQ, isect.geom, bsdfSR))
		{
			break;
		}

		auto bsdf = isect.primitive->bsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), isect.geom);
		if (Math::IsZero(bsdf))
		{
			break;
		}

		// Update throughput
		LM_ASSERT(bsdfSR.pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
		throughput *= bsdf / bsdfSR.pdf.v;

		// Setup next ray
		ray.d = bsdfSR.wo;
		ray.o = isect.geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// --------------------------------------------------------------------------------

		if (++depth >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}
	}
}

// --------------------------------------------------------------------------------

PSSMLTRenderer::PSSMLTRenderer()
	: p(new Impl(this))
{

}

PSSMLTRenderer::~PSSMLTRenderer()
{
	LM_SAFE_DELETE(p);
}

bool PSSMLTRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool PSSMLTRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection PSSMLTRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END