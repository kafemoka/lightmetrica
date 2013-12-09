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
#include <nanon/asset.h>
#include <nanon/assets.h>
#include <nanon/assetfactory.h>
#include <nanon/config.h>
#include <nanon/logger.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class Assets::Impl
{
public:

	bool Load(const pugi::xml_node& node);
	bool RegisterAssetFactory(const AssetFactoryEntry& entry);

private:

	void InitializeAssetFactories();

private:

	std::vector<AssetFactoryEntry> assetFactoryEntries;
	boost::unordered_map<std::string, size_t> assetFactoryMap;
	boost::unordered_map<std::string, std::shared_ptr<Asset>> assetInstanceMap;

};

bool Assets::Impl::RegisterAssetFactory( const AssetFactoryEntry& entry )
{
	// Check if the asset with same name is already registered
	auto it = std::find_if(assetFactoryEntries.begin(), assetFactoryEntries.end(),
		[&entry](const AssetFactoryEntry& o) { return entry.name == o.name; });
	
	if (it != assetFactoryEntries.end())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Asset factory '%s' is already registered") % entry.name));
		return false;
	}

	assetFactoryEntries.push_back(entry);
	return true;
}

void Assets::Impl::InitializeAssetFactories()
{
	// Sort by priority
	std::sort(assetFactoryEntries.begin(), assetFactoryEntries.end(),
		[](const AssetFactoryEntry& a, const AssetFactoryEntry& b) { return a.priority < b.priority; });
	
	// Create a map for the search query by name
	assetFactoryMap.clear();
	for (size_t i = 0; i < assetFactoryEntries.size(); i++)
	{
		assetFactoryMap[assetFactoryEntries[i].name] = i;
	}
}

bool Assets::Impl::Load( const pugi::xml_node& node )
{
	// Initialize asset factories
	InitializeAssetFactories();

	// Element name must be 'assets'
	if (std::strcmp(node.name(), "assets") != 0)
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid element name : %s (expected : assets)") % node.name()));
		return false;
	}

	// By priority, find the child element under 'assets', and
	// find corresponding asset factory and create asset instances.
	for (auto& factoryEntry : assetFactoryEntries)
	{
		// Find the element under 'assets'
		auto assetGroupNode = node.child(factoryEntry.name.c_str());
		if (assetGroupNode)
		{
			NANON_LOG_INFO(boost::str(boost::format("Processing asset group '%s'") % factoryEntry.name));

			// For each child of the node, create an instance of the asset
			for (auto assetNode : assetGroupNode.children())
			{
				// Check asset name
				auto name = assetNode.name();
				if (name != factoryEntry.child)
				{
					NANON_LOG_ERROR(boost::str(boost::format("Invlaid element name '%s'") % factoryEntry.child));
					return false;
				}

				// Type of the asset
				auto typeAttribute = assetNode.attribute("type");
				if (!typeAttribute)
				{
					NANON_LOG_ERROR("Missing attribute 'type'.");
					return false;
				}

				auto idAttribute = assetNode.attribute("id");
				if (!idAttribute)
				{
					NANON_LOG_ERROR("Missing attribute 'id'.");
					return false;
				}

				NANON_LOG_INFO(boost::str(boost::format("Processing asset (id : '%s', type : '%s')") % idAttribute.value() % typeAttribute.value()));

				// Check if the 'id' is already registered
				std::string id = idAttribute.value();
				if (assetInstanceMap.find(id) != assetInstanceMap.end())
				{
					NANON_LOG_ERROR(boost::str(boost::format("ID '%s' is already registered.") % id));
					return false;
				}

				auto asset = factoryEntry.factory->Create(id, typeAttribute.value());
				if (asset == nullptr)
				{
					NANON_LOG_ERROR("Failed to create the asset.");
					return false;
				}

				// Load asset
				if (!asset->Load(assetNode))
				{
					NANON_LOG_ERROR("Failed to load the asset.");
					return false;
				}

				// Register the instance
				assetInstanceMap[id] = asset;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------

Assets::Assets()
	: p(new Impl)
{

}

Assets::~Assets()
{
	NANON_SAFE_DELETE(p);
}

bool Assets::Load( const pugi::xml_node& node )
{
	return p->Load(node);
}

bool Assets::Load( const NanonConfig& config )
{
	return p->Load(config.AssetsElement());
}

bool Assets::RegisterAssetFactory( const AssetFactoryEntry& entry )
{
	return p->RegisterAssetFactory(entry);
}

NANON_NAMESPACE_END