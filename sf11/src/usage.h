#pragma once

#include "d3d11_include.h"

namespace sf11
{

struct SfUsage
{
	enum
	{
		Static,    // slower cpu write, fast gpu read
		Dynamic,   // fast cpu write, slower gpu read
		Staging,   // can be copied from gpu to cpu
		Immutable
	};

	int Value = Static;

	SfUsage() = default;
	SfUsage(int value) : Value(value) {}

	D3D11_USAGE GetUsage() const;
};
	
}
