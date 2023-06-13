#pragma once

#include "d3d11_include.h"

namespace sf11
{

enum class	EBlendFactor
{
	Zero = 1,
	One = 2,
	SrcColor = 3,
	InvSrcColor = 4,
	SrcAlpha = 5,
	InvSrcAlpha = 6,
	DestAlpha = 7,
	InvDestAlpha = 8,
	DestColor = 9,
	InvDestColor = 10,
	SrcAlphaSat = 11,
	BlendFactor = 14,
	InvBlendFactor = 15,
	Src1Color = 16,
	InvSrc1Color = 17,
	Src1Alpha = 18,
	InvSrc1Alpha = 19
};

enum class EBlendOp
{
	Add = 1,
	Subtract = 2,
	RevSubtract = 3,
	Min = 4,
	Max = 5
};

struct SfBlendData
{
	bool BlendEnable = true;
	EBlendFactor SrcBlend;
	EBlendFactor DestBlend;
	EBlendOp BlendOp;
	EBlendFactor SrcBlendAlpha;
	EBlendFactor DestBlendAlpha;
	EBlendOp BlendOpAlpha;
};

class SfBlendState
{
	friend class SfInstance;
	friend class SfContext;

	struct BlendStateData
	{
		class SfInstance* Instance = nullptr;
		ComPtr<ID3D11BlendState> State;
		BlendStateData(SfInstance* instance) : Instance(instance) {}
	};

	std::shared_ptr<BlendStateData> Data;

	explicit SfBlendState(SfInstance* instance, const SfBlendData& data);
	explicit SfBlendState(SfInstance* instance, SfBlendData* data, UINT count);
public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBlendState)
};

}
