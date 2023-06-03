#pragma once
#include "window_state.h"
#include "d3d11_include.h"
#include "render_target.h"
#include "resource.h"
#include <vector>
#include <memory>
#include <functional>

namespace sf11
{

struct WindowCreationParams
{
	// the name to display the toolbar
	const char* Name = "sf11 Window";

	// height of the window to create, leave at 0 for monitor dimension
	UINT Height = 0;

	// width of the window to create, leave at 0 for monitor dimension
	UINT Width = 0;

	// default state of the window
	EWindowState State = EWindowState::Windowed;

	// monitor index to start the window on, leave at -1 for primary
	int MonitorIndex = -1;

	// function to be called when raw mouse movement is detected
	// int param 1: the x mouse movement
	// int param 2: the y mouse movement
	std::function<void(class SfWindow&, int, int)> RawMouseCallback;

	// function to be called when the mouse is scrolled
	// short param: the value of the scroll
	std::function<void(class SfWindow&, short)> MouseScrollCallback;

	// function to be called when a mouse button is clicked
	// UINT param: VK_LBUTTON, VK_RBUTTON, or VK_MBUTTON
	// bool param: true if the button was pressed, false if released
	std::function<void(class SfWindow&, UINT, bool)> MouseButtonCallback;

	// function to be called when a key is pressed
	// UINT param: the VK identifier of the key
	// bool param: true if the key was pressed, false if released
	std::function<void(class SfWindow&, UINT, bool)> KeyCallback;

	// function to be called when a char input is detected
	// UINT param: value of character
	std::function<void(class SfWindow&, UINT)> CharCallback;
};

class SfWindow
{
	friend class SfInstance;
	friend class SfContext;

	struct WindowData : public std::enable_shared_from_this<WindowData>
	{
		HWND Handle;
		class SfInstance* Instance = nullptr;
		WindowCreationParams Params;
		ComPtr<IDXGISwapChain> SwapChain;
		SfRenderTarget BackBuffer; // swap chain target

		// textures and render targets that are resized with the window
		std::vector<std::weak_ptr<SfResource::ResourceData>> AttachedTextures;

		BYTE RawMouseBuffer[60];
		std::function<void(class SfWindow&, int, int)> RawMouseCallback;
		std::function<void(class SfWindow&, short)> MouseScrollCallback;
		std::function<void(class SfWindow&, UINT, bool)> MouseButtonCallback;
		std::function<void(class SfWindow&, UINT, bool)> KeyCallback;
		std::function<void(class SfWindow&, UINT)> CharCallback;

		~WindowData();

		LRESULT CALLBACK DoWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


		void CreateSwapChain();
		void CreateSwapChainRenderTarget();
		void OnResize(UINT w, UINT h);
	};

	std::shared_ptr<WindowData> Data;
	static inline std::weak_ptr<WindowData> CursorClippingWindow;

	SfWindow(const WindowCreationParams& params, class SfInstance* instance);

public:

	SF_DEF_OPERATORS_AND_DEFAULT(SfWindow)
	HWND GetHandle() const { return Data->Handle; }

	// get the render target associated with this window
	SfRenderTarget GetBackBuffer() const { return Data->BackBuffer; }

	// presents the render target associated with this window
	void Present(UINT syncInterval, UINT flags);

	UINT GetWidth() const;
	UINT GetHeight() const;

	// lock the cursor to this window
	void LockCursor();

	// TODO make these functions specific to this window and then document
	void UnlockCursor();
	void HideCursor();
	void ShowCursor();

	// use VK_KEY to check https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	bool KeyIsPressed(unsigned short keyCode);

public:
	
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};

}