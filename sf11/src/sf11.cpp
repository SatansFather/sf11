#include "../sf11.h"
#include "sfassert.h"
#include <list>

namespace sf11
{

std::unique_ptr<SfInstance> CreateInstance(const InstanceCreationParams& params)
{
	return std::make_unique<SfInstance>(params);
}

std::vector<SfAdapter> EnumerateAdapters()
{
	std::vector<SfAdapter> adapters;
	IDXGIFactory* factory = NULL;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) return adapters;

	IDXGIAdapter* adapter;
	for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		SfAdapter a(std::move(adapter));
		adapters.push_back(std::move(a));
	}

	if (factory) factory->Release();

	return std::move(adapters);
}

}