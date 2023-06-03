#pragma once

#include "texture.h"

namespace sf11
{

class SfRenderTarget : public SfTexture2D
{
	friend class SfInstance;
	friend class SfContext;
	friend class SfWindow;
	
public:
	SfRenderTarget(class SfInstance* instance, const TextureParams2D& params = TextureParams2D());
	SF_DEF_OPERATORS_AND_DEFAULT(SfRenderTarget)
};

}