#pragma once

#include "color.h"
#include <string>
#include <memory>

namespace sf11
{

enum class ESurfacePadMethod
{
	NoPadding,
	Clamp,
	Repeat
};

// container for a 2d array intended for textures
class SfSurface2D
{
	std::unique_ptr<SfColor8[]> Data;
	unsigned int Width, Height;
	bool bHasTransparency = false;
	unsigned int DisplayWidth = 0;
	unsigned int DisplayHeight = 0;
	ESurfacePadMethod PadMethod = ESurfacePadMethod::NoPadding;

	SfColor8* GetPixelPointer(unsigned int x, unsigned int y) const;

public:

	SfSurface2D() = default;
	SfSurface2D(unsigned int w, unsigned int h, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);
	SfSurface2D(unsigned int w, unsigned int h, std::unique_ptr<SfColor8[]> buffer);

	SfColor8* GetData() { return Data.get(); }

	void ClearAndResizeData(unsigned int w, unsigned int h, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);

	unsigned int GetPaddedWidth() const { return Width; }
	unsigned int GetPaddedHeight() const  { return Height; }
	unsigned int GetContentWidth() const;
	unsigned int GetContentHeight() const;
	SfColor8 GetPixel(unsigned int x, unsigned int y) const;
	SfColor8 GetPaddedPixel(unsigned int x, unsigned int y) const;
	void SetPixel(unsigned int x, unsigned int y, BYTE r, BYTE g, BYTE b, BYTE a);
	void SetPixel(unsigned int x, unsigned int y, SfColor8 color);

	void SetDisplayWidth(unsigned int w) { DisplayWidth = w; }
	void SetDisplayHeight(unsigned int h) { DisplayHeight = h; }
	unsigned int GetDisplayWidth() { return DisplayWidth; }
	unsigned int GetDisplayHeight() { return DisplayHeight; }

	bool HasTransparency() const { return bHasTransparency; }
	bool IsPadded() const { return PadMethod != ESurfacePadMethod::NoPadding; }

	ESurfacePadMethod GetPadMethod() { return PadMethod; }
	
	void LoadPNG(const std::string& path, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);

	std::unique_ptr<SfSurface2D> CopySurface() const;

};

}