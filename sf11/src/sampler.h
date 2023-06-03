#pragma once

#include "d3d11_include.h"

namespace sf11
{

class SfSamplerState
{
	friend class SfInstance;
	friend class SfContext;

	struct SamplerStateData
	{
		SfInstance* Instance = nullptr;
		D3D11_SAMPLER_DESC Desc;
		ComPtr<ID3D11SamplerState> State;
	};

	std::shared_ptr<SamplerStateData> Data;
public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfSamplerState)
};

}