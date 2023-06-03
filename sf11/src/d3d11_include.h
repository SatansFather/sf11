#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#include <wrl/internal.h>
#include <wrl/client.h>
#include <memory>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#define SF_NULL {}

#define SF_DEF_OPERATORS_AND_DEFAULT(className) \
bool operator==(const className& other) const   \
{                                               \
	return Data.get() == other.Data.get();      \
}                                               \
bool operator!=(const className& other) const   \
{                                               \
	return Data.get() != other.Data.get();      \
}                                               \
operator bool() const { return Data.get(); }    \
className() = default;


//#define DELETE_COPY_CONSTRUCTOR(className)         \
//className (const className&) = delete;             \
//className& operator= (const className&) = delete; 