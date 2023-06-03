#include "adapter.h"

sf11::SfAdapter::SfAdapter(IDXGIAdapter* adapter)
	: Adapter(adapter)
{
	// just get description from it, we dont need to keep the actual adapter
	adapter->GetDesc(&Desc);
}

std::string sf11::SfAdapter::GetName()
{
#pragma warning(disable: 4244)
	std::wstring w = std::wstring(Desc.Description);
	return std::string(w.begin(), w.end());
}

UINT sf11::SfAdapter::GetVendorId()
{
	return Desc.VendorId;
}

UINT sf11::SfAdapter::GetDeviceId()
{
	return Desc.DeviceId;
}

UINT sf11::SfAdapter::GetSubSysId()
{
	return Desc.SubSysId;
}

UINT sf11::SfAdapter::GetRevision()
{
	return Desc.Revision;
}

size_t sf11::SfAdapter::GetDedicatedVideoMemory()
{
	return Desc.DedicatedVideoMemory;
}

size_t sf11::SfAdapter::GetDedicatedSystemMemory()
{
	return Desc.DedicatedSystemMemory;
}

size_t sf11::SfAdapter::GetSharedSystemMemory()
{
	return Desc.SharedSystemMemory;
}

IDXGIAdapter* sf11::SfAdapter::GetDXGI()
{
	return Adapter.Get();
}
