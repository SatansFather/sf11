#pragma once

#include "d3d11_include.h"

namespace sf11
{
	void sfAssert(bool condition, const char* message = nullptr);
	void sfAssertHR(HRESULT hr, const char* message = nullptr);
};
