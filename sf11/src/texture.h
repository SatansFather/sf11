#pragma once

#include "d3d11_include.h"
#include "surface.h"
#include <memory>
#include "resource.h"

namespace sf11
{
class SfTexture : public SfResource
{
	friend class SfInstance;
	friend class SfContext;
	friend class SfWindow;
protected:

	SfTexture() = default;
	SfTexture(SfInstance* instance);
};

class SfTexture1D : public SfTexture
{
	friend class SfInstance;
	friend class SfContext;

	SfTexture1D(SfInstance* instance, const TextureParams1D& params, void* data);

public:

	UINT GetWidth() const { return Data->Texture.Width; }

	SF_DEF_OPERATORS_AND_DEFAULT(SfTexture1D)
};

class SfTexture2D : public SfTexture
{
	friend class SfInstance;
	friend class SfContext;

protected:
	SfTexture2D(SfInstance* instance, const TextureParams2D& params, void* data = nullptr);
	SfTexture2D(SfInstance* instance, const TextureParams2D& params, bool renderTarget, std::unique_ptr<SfSurface2D> surface);
public:

	UINT GetWidth() const { return Data->Texture.Width; }
	UINT GetHeight() const { return Data->Texture.Height; }
	SF_DEF_OPERATORS_AND_DEFAULT(SfTexture2D)
};

class SfTexture3D : public SfTexture
{
	friend class SfInstance;
	friend class SfContext;

	SfTexture3D(SfInstance* instance, const TextureParams3D& params, void* data = nullptr);

public:

	UINT GetWidth() const { return Data->Texture.Width; }
	UINT GetHeight() const { return Data->Texture.Height; }
	UINT GetDepth() const { return Data->Texture.Depth; }
	SF_DEF_OPERATORS_AND_DEFAULT(SfTexture3D)
};

}