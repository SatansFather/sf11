#include "window.h"
#include "instance.h"
#include "sfassert.h"
#include "context.h"
#include <mutex>

namespace sf11
{

SfWindow::SfWindow(const WindowCreationParams& params, SfInstance* instance)
	: Data(std::make_shared<WindowData>())
{
	Data->Instance = instance;
	Data->Params = params;

	// RegisterClassEx cannot register the same class twice
	// allow multiple threads to create a window at the same time
	static std::mutex mut;
	static unsigned int counter = 0;
	std::string className;
	mut.lock();
	className = std::to_string(counter);
	counter++;
	mut.unlock();

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = className.c_str();
	wc.hIconSm = nullptr;
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "RegisterClassEx failed", "sf11", NULL);
		exit(1);
	}

	int monX = GetSystemMetrics(SM_CXSCREEN);
	int monY = GetSystemMetrics(SM_CYSCREEN);
	int resX = params.Width == 0 ? monX : params.Width;
	int resY = params.Height == 0 ? monY : params.Height;

	RECT wr;
	wr.left = 0;
	wr.top = 0;
	wr.right = resX;
	wr.bottom = resY;

	DWORD winFlags = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME;


	AdjustWindowRect(&wr, winFlags, FALSE);

	Data->Handle = CreateWindowEx(
		0,
		className.c_str(),
		params.Name,
		winFlags,
		(monX - resX) / 2, // centered
		(monY - resY) / 2, // on monitor
		wr.right - wr.left,
		wr.bottom - wr.top,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		nullptr
	);

	SetWindowLongPtr(Data->Handle, GWLP_USERDATA, (LONG_PTR)Data.get());

	if (params.State != EWindowState::Headless)
	{
		ShowWindow(Data->Handle, SW_SHOW);
	}

	Data->CreateSwapChain();

	Data->RawMouseCallback = params.RawMouseCallback;
	Data->MouseScrollCallback = params.MouseScrollCallback;
	Data->MouseButtonCallback = params.MouseButtonCallback;
	Data->KeyCallback = params.KeyCallback;
	Data->CharCallback = params.CharCallback;
}

void SfWindow::Present(UINT syncInterval, UINT flags)
{
	Data->SwapChain->Present(syncInterval, flags);
}

UINT SfWindow::GetWidth() const
{
	RECT rect;
	GetWindowRect(GetHandle(), &rect);
	return rect.right - rect.left;
}

UINT SfWindow::GetHeight() const
{
	RECT rect;
	GetWindowRect(GetHandle(), &rect);
	return rect.bottom - rect.top;
}

void SfWindow::LockCursor()
{
	if (CursorClippingWindow.lock())
	{
		UnlockCursor();
		CursorClippingWindow = Data;
	}

	RECT rect;
	GetClientRect(GetHandle(), &rect);

	POINT ul;
	ul.x = rect.left;
	ul.y = rect.top;

	POINT lr;
	lr.x = rect.right;
	lr.y = rect.bottom;

	MapWindowPoints(GetHandle(), nullptr, &ul, 1);
	MapWindowPoints(GetHandle(), nullptr, &lr, 1);

	rect.left = ul.x;
	rect.top = ul.y;

	rect.right = lr.x;
	rect.bottom = lr.y;

	ClipCursor(&rect);
}

void SfWindow::UnlockCursor()
{
	ClipCursor(nullptr);
}

void SfWindow::HideCursor()
{
	::ShowCursor(false);
}

void SfWindow::ShowCursor()
{
	::ShowCursor(true);
}

bool SfWindow::KeyIsPressed(unsigned short keyCode)
{
	return GetActiveWindow() == GetHandle() && (GetAsyncKeyState(keyCode) & 0x8000);
}

void SfWindow::WindowData::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC scd;
	DXGI_MODE_DESC buffDesc;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	ZeroMemory(&buffDesc, sizeof(DXGI_MODE_DESC));

	buffDesc.Width = Params.Width;
	buffDesc.Height = Params.Height;
	buffDesc.RefreshRate.Numerator = 0;
	buffDesc.RefreshRate.Denominator = 0;
	buffDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	buffDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	buffDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.BufferDesc = buffDesc;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = Handle;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	ComPtr<IDXGIDevice> dxgiDevice;
	Instance->GetDevice()->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	ComPtr<IDXGIFactory> dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);

	sfAssertHR(dxgiFactory->CreateSwapChain(Instance->GetDevice(), &scd, &SwapChain));

	CreateSwapChainRenderTarget();
}

void SfWindow::WindowData::CreateSwapChainRenderTarget()
{
	BackBuffer.Data = std::make_shared<SfResource::ResourceData>(Instance);
	BackBuffer.Data->Instance = Instance;

	// get the address of the swap chain's back buffer 
	ID3D11Texture2D* backBuffer = nullptr;
	sfAssertHR(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer), "could not get back buffer texture");
	sfAssert(backBuffer, "cannot fill swap chain render target texture");

	// assign the render target view of our new back buffer object to the swap chain's buffer
	Instance->GetDevice()->CreateRenderTargetView(backBuffer, NULL, &BackBuffer.Data->Texture.RenderTargetView);

	D3D11_TEXTURE2D_DESC td;
	backBuffer->GetDesc(&td);

	BackBuffer.Data->Texture.Width = td.Width;
	BackBuffer.Data->Texture.Height = td.Height;

	backBuffer->Release();
}

void SfWindow::WindowData::OnResize(UINT w, UINT h)
{
	if (!SwapChain) return;

	// remove all render targets from output
	SfRenderTarget targets[8] = { SF_NULL };
	Instance->GetImmediateContext().BindRenderTargets(targets, 8);
	//Instance->GetImmediateContext().ClearState();
	CreateSwapChainRenderTarget();
}

LRESULT CALLBACK SfWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto win = (SfWindow::WindowData*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (win) return win->DoWndProc(hWnd, msg, wParam, lParam);
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK SfWindow::WindowData::DoWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
		{
			UINT w = LOWORD(lParam);
			UINT h = HIWORD(lParam);
			OnResize(w, h);
			return 0;
		}
		case WM_SIZING:
		{
			RECT rect = *(RECT*)lParam;
			UINT w = rect.right - rect.left;
			UINT h = rect.bottom - rect.top;
			OnResize(w, h);
			return 0;
		}
		case WM_INPUT:
		{
			if (RawMouseCallback)
			{
				BYTE* buffer = RawMouseBuffer;
				UINT bufferSize = 60;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID)buffer, &bufferSize, sizeof(RAWINPUTHEADER));
				RAWINPUT* raw = (RAWINPUT*)buffer;

				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					if (raw->data.mouse.lLastX != NULL || raw->data.mouse.lLastY != NULL)
					{
						SfWindow win;
						win.Data = shared_from_this();
						RawMouseCallback(win, raw->data.mouse.lLastX, raw->data.mouse.lLastY);
					}
				}
			}
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			if (MouseScrollCallback)
			{
				SfWindow win;
				win.Data = shared_from_this();
				MouseScrollCallback(win, GET_WHEEL_DELTA_WPARAM(wParam));
			}
			return 0;
		}
		case WM_SYSCHAR:
		case WM_CHAR:
		{
			if (CharCallback)
			{												// TODO what are these constants
				if (wParam == VK_RETURN || wParam == VK_BACK || wParam == 10 || wParam == 127)
					return 0;

				SfWindow win;
				win.Data = shared_from_this();
				CharCallback(win, (UINT)wParam);
			}
			return 0;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (KeyCallback)
			{
				SfWindow win;
				win.Data = shared_from_this();
				KeyCallback(win, (UINT)wParam, false);
			}
			return 0;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (KeyCallback)
			{
				SfWindow win;
				win.Data = shared_from_this();
				KeyCallback(win, (UINT)wParam, true);
			}
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			if (MouseButtonCallback)
			{
				SfWindow win;
				win.Data = shared_from_this();
				MouseButtonCallback(win, (UINT)wParam, false);
			}
			return 0;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			if (MouseButtonCallback)
			{
				SfWindow win;
				win.Data = shared_from_this();
				MouseButtonCallback(win, (UINT)wParam, true);
			}
			return 0;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

SfWindow::WindowData::~WindowData()
{
	DestroyWindow(Handle);
}

}
