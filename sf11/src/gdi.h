#pragma once

#include "d3d11_include.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace sf11
{

static bool GdiInit = false;
static ULONG_PTR GDIPlusToken;
void InitGDI();

}