#pragma once

#include "d3d11_include.h"

namespace sf11
{

enum class EFillMode
{
	Solid,
	Wireframe
};

// back = cull counter clockwise
// front = cull clockwise
enum class ECullMode
{
	CullBack,
	CullFront,
	CullNone
};

class SfRasterizer
{
	friend class SfInstance;
	friend class SfContext;

	struct RasterizerData
	{
		EFillMode FillMode;
		ECullMode CullMode;
		ComPtr<ID3D11RasterizerState> State;
	};

	std::shared_ptr<RasterizerData> Data;

public:
	EFillMode GetFillMode() const { return Data->FillMode; }
	ECullMode GetCullMode() const { return Data->CullMode; }

	SF_DEF_OPERATORS_AND_DEFAULT(SfRasterizer)

private:
	ID3D11RasterizerState* GetState() { return Data->State.Get(); }
	ID3D11RasterizerState** GetPtrAddress() { return &Data->State; }
};

}