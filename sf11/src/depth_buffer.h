#pragma once

#include "texture.h"

namespace sf11
{

enum class EDepthState
{
	ReadWrite,
	ReadOnly,
	WriteOnly,
	Disabled,
};

class SfDepthStencilState
{
	friend class SfInstance;
	friend class SfContext;

	struct DepthStencilStateData
	{
		class SfInstance* Instance = nullptr;
		const D3D11_DEPTH_STENCIL_DESC Desc;
		ComPtr<ID3D11DepthStencilState> State;

		DepthStencilStateData(class SfInstance* instance, const D3D11_DEPTH_STENCIL_DESC& desc) :
			Instance(instance), Desc(desc) {}
	};

	std::shared_ptr<DepthStencilStateData> Data;

	SfDepthStencilState(class SfInstance* instance, const D3D11_DEPTH_STENCIL_DESC& desc);

public:
	D3D11_DEPTH_STENCIL_DESC GetDesc() { return Data->Desc; }
	SF_DEF_OPERATORS_AND_DEFAULT(SfDepthStencilState)
};

class SfDepthBuffer : public SfTexture2D
{
	friend class SfInstance;
	friend class SfContext;

	SfDepthBuffer(class SfInstance* instance, UINT width, UINT height, bool enableStencil = false);

public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfDepthBuffer)
};

};