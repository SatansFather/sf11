#pragma once

#include "d3d11_include.h"

namespace sf11
{

struct SfColor8
{
	unsigned int Color;

	SfColor8() : Color() {}
	SfColor8(const SfColor8& col) : Color(col.Color) {}
	SfColor8(unsigned int col) : Color(col) {}
	SfColor8(BYTE r, BYTE g, BYTE b, BYTE a) : Color((a << 24u) | (r << 16u) | (g << 8u) | b) {}
	SfColor8(BYTE r, BYTE g, BYTE b) : Color((r << 16u) | (g << 8u) | b) {}
	SfColor8(SfColor8 col, BYTE a) : SfColor8((a << 24u) | col.Color) {}

	BYTE GetA() const
	{
		return Color >> 24u;
	}
	BYTE GetR() const
	{
		return (Color >> 16u) & 0xFFu;
	}
	BYTE GetG() const
	{
		return (Color >> 8u) & 0xFFu;
	}
	BYTE GetB() const
	{
		return Color & 0xFFu;
	}
	void SetA(BYTE a)
	{
		Color = (Color & 0xFFFFFFu) | (a << 24u);
	}
	void SetR(BYTE r)
	{
		Color = (Color & 0xFF00FFFFu) | (r << 16u);
	}
	void SetG(BYTE g)
	{
		Color = (Color & 0xFFFF00FFu) | (g << 8u);
	}
	void SetB(BYTE b)
	{
		Color = (Color & 0xFFFFFF00u) | b;
	}

	SfColor8 Lerp(SfColor8 other, float alpha) const
	{
		const auto lerp = [](BYTE a, BYTE b, float alpha) -> BYTE
		{
			alpha = alpha > 1 ? 1 : (alpha < 0 ? 0 : alpha);
			return (BYTE)(a + (b - a) * alpha);
		};
		return SfColor8
		(
			lerp(GetR(), other.GetR(), alpha), 
			lerp(GetG(), other.GetG(), alpha), 
			lerp(GetB(), other.GetB(), alpha), 
			lerp(GetA(), other.GetA(), alpha)
		);
	}

	SfColor8 Inverse() const
	{
		return SfColor8
		(
			255 - GetR(),
			255 - GetG(),
			255 - GetB(),
			255 - GetA()
		);
	}
};

}