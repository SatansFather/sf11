#include "gdi.h"

namespace sf11
{

void sf11::InitGDI()
{
	if (GdiInit) return;
	Gdiplus::GdiplusStartupInput input;
	input.GdiplusVersion = 1;
	input.DebugEventCallback = nullptr;
	input.SuppressBackgroundThread = false;
	Gdiplus::GdiplusStartup(&GDIPlusToken, &input, nullptr);
	GdiInit = true;
}

}
