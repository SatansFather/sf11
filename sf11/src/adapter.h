#pragma once

#include <string>
#include "d3d11_include.h"

namespace sf11
{

// holds information about a physical graphics device inside the machine
class SfAdapter
{
	ComPtr<IDXGIAdapter> Adapter = nullptr;
	DXGI_ADAPTER_DESC Desc;

public:

	//SfAdapter(const SfAdapter&) = delete;
	//SfAdapter& operator=(const SfAdapter&) = delete;

	SfAdapter(IDXGIAdapter* adapter);
	std::string GetName();
	UINT GetVendorId();
	UINT GetDeviceId();
	UINT GetSubSysId();
	UINT GetRevision();
	size_t GetDedicatedVideoMemory();
	size_t GetDedicatedSystemMemory();
	size_t GetSharedSystemMemory();
	IDXGIAdapter* GetDXGI();
};

}