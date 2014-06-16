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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/math.linalgebra.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/light.h>
#include <lightmetrica/bsdf.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

/*
	Checks if the condition W1
	[Veach 1997, p.260] is preserved.
*/
//TEST_F(BPTMISTest, Condition_W1)
//{
//	//for (const auto& type : misWeightTypes)
//	//{
//	//	// Create MIS weighting function
//	//	std::unique_ptr<BPTMISWeight> misWeight(ComponentFactory::Create<BPTMISWeight>(type));
//	//	EXPECT_TRUE(misWeight->Configure(ConfigNode(), assets));
//
//	//	for (const auto& subpath : bptSubpaths)
//	//	{
//	//		const auto& lightSubpath = subpath.lightSubpath;
//	//		const auto& eyeSubpath = subpath.eyeSubpath;
//	//		
//	//		Math::Float sumWeights(0);
//
//	//		for (size_t s = 0; s <= lightSubpath.vertices.size(); s++)
//	//		{
//	//			for (size_t t = 0; t <= eyeSubpath.vertices.size(); t++)
//	//			{
//	//				sumWeights += misWeight->Evaluate(BPTFullPath(s, t, lightSubpath, eyeSubpath));
//	//			}
//	//		}
//
//	//		EXPECT_TRUE(ExpectNear(Math::Float(1), sumWeights)) << "Weight type : " << type;
//	//	}
//	//}
//}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END