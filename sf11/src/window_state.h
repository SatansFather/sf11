#pragma once

namespace sf11
{

enum class EWindowState
{
	Windowed,   // standard window
	Borderless, // fullscreen borderless window
	Fullscreen, // fullscreen, takes over desktop
	Headless    // no window, hidden
};

}