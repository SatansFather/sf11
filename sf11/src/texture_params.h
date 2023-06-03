#pragma once

#include "d3d11_include.h"
#include "format.h"
#include "usage.h"

namespace sf11
{

struct TextureParams1D
{
	UINT Width = 0;
	UINT MipLevels = 1;
	SfFormat TextureFormat = { SfFormat::UNorm8, 4 };
	SfUsage Usage = SfUsage::Static;
	bool AllowUnorderedAccess = false;
	bool AllowShaderResource = true;
};

struct TextureParams2D : public TextureParams1D
{
	UINT Height = 0;
	UINT SampleCount = 1;
	UINT SampleQuality = 0;
};

struct TextureParams3D : public TextureParams2D
{
	UINT Depth = 0;
};

}