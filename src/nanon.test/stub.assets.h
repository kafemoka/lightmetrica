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

#ifndef __NANON_TEST_STUB_ASSETS_H__
#define __NANON_TEST_STUB_ASSETS_H__

#include "common.h"
#include <nanon/assets.h>
#include <string>
#include <memory>
#include <boost/unordered_map.hpp>

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class StubAssets : public Assets
{
public:

	virtual Asset* GetAssetByName(const std::string& name) const { return assetInstanceMap.at(name).get(); }
	void Add(const std::string& id, const std::shared_ptr<Asset>& asset) { assetInstanceMap[id] = asset; }

protected:

	boost::unordered_map<std::string, std::shared_ptr<Asset>> assetInstanceMap;

};

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END

#endif // __NANON_TEST_STUB_ASSETS_H__