#include "STDInclude.hpp"

GPU::GPU() : window(nullptr), d3d9(nullptr), device(nullptr), screenbuffer(nullptr), cpu(nullptr), mode(MODE_HBLANK), clock(0)
{
	ZeroObject(this->mem);

	WNDCLASSEX wc;
	ZeroObject(wc);
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = GPU::WindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = HBRUSH(COLOR_WINDOW);
	wc.lpszClassName = L"GBAWindow";
	RegisterClassEx(&wc);

	this->window = CreateWindowExA(NULL, "GBAWindow", "GB-EMU", WS_OVERLAPPEDWINDOW, 300, 300, 500, 400, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	ShowWindow(this->window, SW_SHOW);

	if (SUCCEEDED(Direct3DCreate9Ex(D3D_SDK_VERSION, &this->d3d9)))
	{
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroObject(d3dpp);
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = this->window;

		if (SUCCEEDED(this->d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, nullptr, &this->device)))
		{
			this->device->CreateTexture(160, 144, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &this->screenbuffer, nullptr);
		}

		this->device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
	}
}

void GPU::connectCPU(CPU* _cpu)
{
	this->cpu = _cpu;
}

void GPU::renderToTexture()
{
	if (this->device)
	{
		this->device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
		this->device->BeginScene();

		D3DVIEWPORT9 vp;
		this->device->GetViewport(&vp);
		float width = float(vp.Width);
		float height = float(vp.Height);

		auto color = D3DCOLOR_XRGB(255, 255, 255);

		GPU::D3DTLVERTEX qV[4] = {
			{ 0, 0 , 0.0f, 1.0f, color },
			{ width, 0 , 0.0f, 1.0f, color },
			{ width, height, 0.0f, 1.0f, color },
			{ 0, height, 0.0f, 1.0f, color },
		};

		this->device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		this->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		this->device->SetTexture(0, this->screenbuffer);
		this->device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, qV, sizeof(D3DTLVERTEX));

		this->device->EndScene();
		this->device->Present(nullptr, nullptr, nullptr, nullptr);
	}


	// VRAM offset for the tile map
	unsigned short mapoffs = (this->mem.flags & FLAG_ALT_TILE_MAP) ? 0x1C00 : 0x1800;

	// Which line of tiles to use in the map
	mapoffs += ((this->mem.curline + this->mem.yscrl) & 255) >> 3;

	// Which tile to start with in the map line
	unsigned char lineoffs = (this->mem.xscrl >> 3);

	// Which line of pixels to use in the tiles
	//unsigned char y = (this->mem.curline + this->mem.yscrl) & 7;

	// Where in the tileline to start
	//unsigned char x = this->mem.xscrl & 7;

	// Where to render on the canvas
	auto canvasoffs = this->mem.curline * 160;

	// Read tile index from the background map
	unsigned short tile = this->cpu->getMMU()->vram[mapoffs + lineoffs];

	// If the tile data set in use is #1, the
	// indices are signed; calculate a real tile offset
	if ((this->mem.flags & FLAG_ALT_TILE_SET) && tile < 128) tile += 256;

	D3DLOCKED_RECT lockedRect;
	this->screenbuffer->LockRect(0, &lockedRect, nullptr, 0);

	DWORD* buffer = static_cast<DWORD*>(lockedRect.pBits);
	for (int i = 0; i < 160; i+=4)
	{
		GBCPixelQuad quad;
		std::memcpy(&quad,&this->cpu->getMMU()->vram[tile], sizeof(quad));

		// Plot the pixel to canvas
		buffer[canvasoffs + 0] = GPU::GetGBAColor(quad._1);
		buffer[canvasoffs + 1] = GPU::GetGBAColor(quad._2);
		buffer[canvasoffs + 2] = GPU::GetGBAColor(quad._3);
		buffer[canvasoffs + 3] = GPU::GetGBAColor(quad._4);
		canvasoffs += 4;

		// When this tile ends, read another
		lineoffs = (lineoffs + 1) & 31;
		tile = this->cpu->getMMU()->vram[mapoffs + lineoffs];
		if ((this->mem.flags & FLAG_ALT_TILE_SET) && tile < 128) tile += 256;

		this->screenbuffer->UnlockRect(0);
	}
}

void GPU::frame()
{
	this->clock += this->cpu->registers.m;

	switch (this->mode)
	{
		case MODE_HBLANK:
		{
			if(this->clock >= 51)
			{
				this->clock = 0;
				this->mem.curline++;

				if(this->mem.curline == 143)
				{
					this->mode = MODE_VBLANK;
					this->renderToTexture();
				}
				else
				{
					this->mode = MODE_OAM;
				}
			}
			break;
		}

		case MODE_VBLANK:
		{
			if(this->clock >= 114)
			{
				this->clock = 0;
				this->mem.curline++;

				if(this->mem.curline > 153)
				{
					this->mode = MODE_OAM;
					this->mem.curline = 0;
				}
			}
			break;
		}

		case MODE_OAM:
		{
			if(this->clock >= 20)
			{
				this->clock = 0;
				this->mode = MODE_VRAM;
			}
			break;
		}

		case MODE_VRAM:
		{
			if(this->clock >= 43)
			{
				this->clock = 0;
				this->mode = MODE_HBLANK;

				if(this->mem.flags & FLAG_DISPLAY_ON)
				{
					if(this->mem.flags & FLAG_BACKGROUND_ON)
					{
						
					}
					if(this->mem.flags & FLAG_SPRITES_ON)
					{
						
					}
				}
			}
			break;
		}
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

	if(address >= 7 && address <= 9)
	{
		OutputDebugStringA("");
	}

	if (address < sizeof(this->mem))
	{
		return reinterpret_cast<unsigned char*>(&this->mem) + address;
	}

	return nullptr;
}

DWORD GPU::getColorFromPalette(unsigned int palette, unsigned int index)
{
	if (palette > 3 || index > 4) return 0;

	TODO!
}

DWORD GPU::GetGBAColor(GPU::GBColor pixel)
{
	switch(pixel)
	{
	case GBC_BLACK: return D3DCOLOR_XRGB(0, 0, 0);
	case GBC_DARK_GRAY: return D3DCOLOR_XRGB(192, 192, 192);
	case GBC_LIGHT_GRAY: return D3DCOLOR_XRGB(96, 96, 96);
	case GBC_WHITE: return D3DCOLOR_XRGB(255, 255, 255);
	}
	return 0;
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
