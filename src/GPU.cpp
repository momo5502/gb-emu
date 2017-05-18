#include "STDInclude.hpp"

GPU::GPU() : window(nullptr), d3d9(nullptr), device(nullptr)
{
	ZeroObject(this->mem);

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = GPU::WindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = HBRUSH(COLOR_WINDOW);
	wc.lpszClassName = L"GBAWindow";

	// register the window class
	RegisterClassEx(&wc);

	// create the window and use the result as the handle
	this->window = CreateWindowEx(NULL,
		L"GBAWindow",    // name of the window class
		L"GB-EMU",   // title of the window
		WS_OVERLAPPEDWINDOW,    // window style
		300,    // x-position of the window
		300,    // y-position of the window
		500,    // width of the window
		400,    // height of the window
		nullptr,    // we have no parent window, NULL
		nullptr,    // we aren't using menus, NULL
		GetModuleHandle(nullptr),    // application handle
		nullptr);    // used with multiple windows, NULL

	ShowWindow(this->window, SW_SHOW);

	if (SUCCEEDED(Direct3DCreate9Ex(D3D_SDK_VERSION, &this->d3d9)))
	{
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroObject(d3dpp);
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = this->window;

		this->d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, nullptr, &this->device);
	}
}

void GPU::frame()
{
	if (this->device)
	{
		this->device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		this->device->BeginScene();

		this->device->EndScene();
		this->device->Present(nullptr, nullptr, nullptr, nullptr);
	}

	MSG msg;
	while (this->working() && PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool GPU::working()
{
	return (IsWindow(this->window) != FALSE);
}

unsigned char* GPU::getMemoryPtr(unsigned short address)
{
	address -= 0xFF40;

	if(address == 0)
	{
		throw std::runtime_error("Not implemented!");
	}
	else if(address == 1)
	{
		throw std::runtime_error("Not implemented!");
	}
	else if(address < sizeof(this->mem))
	{
		return reinterpret_cast<unsigned char*>(&this->mem) + address;
	}

	return nullptr;
}

LRESULT CALLBACK GPU::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			DestroyWindow(hWnd);
			return 0;
		}

		default: 
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

GPU::~GPU()
{
	if (this->device) this->device->Release();
	if (this->d3d9) this->d3d9->Release();
	if (this->working()) DestroyWindow(this->window);
}
