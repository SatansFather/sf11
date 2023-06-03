#include "surface.h"
#include "gdi.h"
#include "color.h"
#include "sfassert.h"
#include <vector>
#include <string>

namespace sf11
{


void SfSurface2D::LoadPNG(const std::string& name, ESurfacePadMethod pad)
{
	PadMethod = pad;

	Gdiplus::Bitmap bitmap(std::wstring(name.begin(), name.end()).c_str());
	Gdiplus::Status s = bitmap.GetLastStatus();
	sfAssert(s == Gdiplus::Status::Ok, ("could not load texture \'" + name + "\' with gdiplus - error " + std::to_string(s)).c_str());
	
	Width = bitmap.GetWidth();
	Height = bitmap.GetHeight();
	Data = std::make_unique<SfColor8[]>(Width * Height);


	for (unsigned int y = 0; y < Height; y++)
	{
		for (unsigned int x = 0; x < Width; x++)
		{
			Gdiplus::Color c;
			bitmap.GetPixel(x, y, &c);
			Data[y * Width + x] = c.GetValue();
			if (c.GetAlpha() < 255) bHasTransparency = true;
		}
	}

	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		//PadMethod = ESurfacePadMethod::Repeat;

		// create another surface to temporarily store the data
		std::unique_ptr<SfSurface2D> temp = CopySurface();
		temp->PadMethod = ESurfacePadMethod::NoPadding;

		ClearAndResizeData(Width, Height, PadMethod);

		for (unsigned int i = 0; i < Width - 2; i++)
		  for (unsigned int j = 0; j < Height - 2; j++)
		    SetPixel(i, j, temp->GetPixel(i, j));
	}
}

std::unique_ptr<SfSurface2D> SfSurface2D::CopySurface() const
{
	std::unique_ptr<SfSurface2D> copy = std::make_unique<SfSurface2D>();

	copy->Width = Width;
	copy->Height = Height;
	copy->bHasTransparency = bHasTransparency;
	copy->PadMethod = PadMethod;

	copy->Data = std::make_unique<SfColor8[]>(Width * Height);
	memcpy(copy->Data.get(), Data.get(), Width * Height * 4);

	return copy;
}

SfColor8* SfSurface2D::GetPixelPointer(unsigned int x, unsigned int y) const
{
	sfAssert(Data.get(), "cannot get pixel offset from null data");
	sfAssert(Width > x && Height > y, "cannot get pixel out of bounds");

	return Data.get() + (y * Width + x);
}

SfSurface2D::SfSurface2D(unsigned int w, unsigned int h, ESurfacePadMethod pad)
{
	Width = w;
	Height = h;

	if (pad != ESurfacePadMethod::NoPadding)
	{
		w += h;
		h += 2;
	}

	Data = std::make_unique<SfColor8[]>(w * h);
}

SfSurface2D::SfSurface2D(unsigned int w, unsigned int h, std::unique_ptr<SfColor8[]> buffer)
{
	Width = w;
	Height = h;
	Data = std::move(buffer);
}

void SfSurface2D::ClearAndResizeData(unsigned int w, unsigned int h, ESurfacePadMethod pad)
{
	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		w += 2;
		h += 2;
	}

	Data.reset();
	Width = w;
	Height = h;
	Data = std::make_unique<SfColor8[]>(Width * Height);
}

unsigned int SfSurface2D::GetContentWidth() const
{
	return Width - (PadMethod == ESurfacePadMethod::NoPadding ? 0 : 2);
}

unsigned int SfSurface2D::GetContentHeight() const
{
	return Height - (PadMethod == ESurfacePadMethod::NoPadding ? 0 : 2);
}

SfColor8 SfSurface2D::GetPixel(unsigned int x, unsigned int y) const
{
	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		x++;
		y++;
	}

	return *GetPixelPointer(x, y);
}

SfColor8 SfSurface2D::GetPaddedPixel(unsigned int x, unsigned int y) const
{
	return *GetPixelPointer(x, y);
}

void SfSurface2D::SetPixel(unsigned int x, unsigned int y, BYTE r, BYTE g, BYTE b, BYTE a)
{
	SetPixel(x, y, SfColor8(r, g, b, a));
}

void SfSurface2D::SetPixel(unsigned int x, unsigned int y, SfColor8 color)
{
	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		x++;
		y++;
	}

	SfColor8* pixel = GetPixelPointer(x, y);

	if (color.GetA() < 255) bHasTransparency = true;

	*pixel = color;

	if (x == 1 || x == Width - 2 || y == 1 || y == Height - 2)
	{
		std::vector<SfColor8*> pixels;
		switch (PadMethod)
		{
		case ESurfacePadMethod::Repeat:
		{
			if (x == 1)			pixels.push_back(GetPixelPointer(Width - 1, y));
			if (x == Width - 2) pixels.push_back(GetPixelPointer(0, y));

			if (y == 1)			 pixels.push_back(GetPixelPointer(x, Height - 1));
			if (y == Height - 2) pixels.push_back(GetPixelPointer(x, 0));

			if (x == 1 && y == 1)				   pixels.push_back(GetPixelPointer(Width - 1, Height - 1));
			if (x == Width - 2 && y == Height - 2) pixels.push_back(GetPixelPointer(0, 0));
			if (x == Width - 2 && y == 1)		   pixels.push_back(GetPixelPointer(0, Height - 1));
			if (x == 1 && y == Height - 2)		   pixels.push_back(GetPixelPointer(Width - 1, 0));

			break;
		}
		case ESurfacePadMethod::Clamp:
		{
			if (x == 1)			pixels.push_back(GetPixelPointer(0, y));
			if (x == Width - 2) pixels.push_back(GetPixelPointer(Width - 1, y));

			if (y == 1)			 pixels.push_back(GetPixelPointer(x, 0));
			if (y == Height - 2) pixels.push_back(GetPixelPointer(x, Height - 1));

			if (x == 1 && y == 1) pixels.push_back(GetPixelPointer(0, 0));
			if (x == Width - 2 && y == Height - 2) pixels.push_back(GetPixelPointer(Width - 1, Height - 1));
			if (x == Width - 2 && y == 1)  pixels.push_back(GetPixelPointer(Width - 1, 0));
			if (x == 1 && y == Height - 2) pixels.push_back(GetPixelPointer(0, Height - 1));

			break;
		}
		}
		for (SfColor8* p : pixels)
		{
			*p = color;
		}
	}
}


}