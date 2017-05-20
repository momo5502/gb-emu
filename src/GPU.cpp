#include "STDInclude.hpp"

GPU::GPU() : window(nullptr), d3d9(nullptr), device(nullptr), screenTexture(nullptr), cpu(nullptr), mode(MODE_HBLANK), clock(0)
{
	ZeroObject(this->mem);
	ZeroObject(this->tiles);
	ZeroObject(this->screenBuffer);

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
			this->device->CreateTexture(160, 144, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &this->screenTexture, nullptr);
		}

		this->device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
	}
}

void GPU::connectCPU(CPU* _cpu)
{
	this->cpu = _cpu;
}

void GPU::renderTexture()
{
	if (this->device)
	{
		this->device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
		this->device->BeginScene();

		D3DVIEWPORT9 vp;
		this->device->GetViewport(&vp);
		float width = float(vp.Width);
		float height = float(vp.Height);

		GPU::D3DTLVERTEX qV[4] = {
			{ 0,     height, 0.0f, 1.0f, 0.0, 1.0 },
			{ 0,     0,      0.0f, 1.0f, 0.0, 0.0 },
			{ width, height, 0.0f, 1.0f, 1.0, 1.0 },
			{ width, 0,      0.0f, 1.0f, 1.0, 0.0 }
		};

		// Update the screen to the texture
		D3DLOCKED_RECT lockedRect;
		this->screenTexture->LockRect(0, &lockedRect, nullptr, 0);
		std::memcpy(lockedRect.pBits, this->screenBuffer, sizeof this->screenBuffer);
		this->screenTexture->UnlockRect(0);

		this->device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		this->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
		this->device->SetTexture(0, this->screenTexture);
		this->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, qV, sizeof(D3DTLVERTEX));

		this->device->EndScene();
		this->device->Present(nullptr, nullptr, nullptr, nullptr);
	}
}

void GPU::renderScreen()
{
	if (this->mem.flags & FLAG_BACKGROUND_ON)
	{
		unsigned short linebase = 160 * this->mem.curline;
		unsigned short mapbase = ((this->mem.flags & FLAG_ALT_TILE_MAP) ? 0x1C00 : 0x1800) + ((((this->mem.curline + this->mem.yscrl) & 255) >> 3) << 5);
		unsigned char y = (this->mem.curline + this->mem.yscrl) & 7;
		unsigned char x = this->mem.xscrl & 7;
		unsigned char t = (this->mem.xscrl >> 3) & 31;
		unsigned char w = 160;

		if (!(this->mem.flags & FLAG_ALT_TILE_SET))
		{
			unsigned short tile = this->cpu->getMMU()->vram[mapbase + t];
			if (tile < 128) tile = 256 + tile;
			unsigned char* tilerow = this->tiles[tile][y];
			do
			{
				//if (tilerow[x]) __debugbreak();

				//GPU._scanrow[160 - x] = tilerow[x];
				this->screenBuffer[linebase] = this->getColorFromPalette(0, tilerow[x]);
				x++;
				if (x == 8)
				{
					t = (t + 1) & 31; x = 0;
					tile = this->cpu->getMMU()->vram[mapbase + t];
					if (tile < 128)
						tile = 256 + tile;
					tilerow = this->tiles[tile][y];
				}
				linebase++;
			} while (--w);
		}
		else
		{
			unsigned char* tilerow = this->tiles[this->cpu->getMMU()->vram[mapbase + t]][y];
			do
			{
				//GPU._scanrow[160 - x] = tilerow[x];
				this->screenBuffer[linebase] = this->getColorFromPalette(0, tilerow[x]);
				x++;
				if (x == 8) { t = (t + 1) & 31; x = 0; tilerow = this->tiles[this->cpu->getMMU()->vram[mapbase + t]][y]; }
				linebase++;
			} while (--w);
		}
	}
	if (this->mem.flags & FLAG_SPRITES_ON)
	{

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
					this->renderTexture();
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
					this->renderScreen();
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

	if (address < sizeof(this->mem))
	{
		return reinterpret_cast<unsigned char*>(&this->mem) + address;
	}

	return nullptr;
}

void GPU::updateTile(unsigned short addr)
{
	unsigned short saddr = addr;
	if (addr & 1) { saddr--; addr--; }
	unsigned short tile = (addr >> 4) & 511;
	unsigned short y = (addr >> 1) & 7;
	unsigned short sx;
	for (unsigned short x = 0; x < 8; x++)
	{
		sx = 1 << (7 - x);

		unsigned char var = (this->cpu->getMMU()->vram[saddr & 0x1FFF] & sx) ? 1 : 0;
		var |= (this->cpu->getMMU()->vram[saddr & 0x1FFF + 1] & sx) ? 2 : 0;
		var &= 3;

		this->tiles[tile][y][x] = var;
	}
}

DWORD GPU::getColorFromPalette(unsigned int palette, unsigned int index)
{
	if (palette > 3 || index > 4) return 0;

	GPU::GBCPixelQuad* quad = reinterpret_cast<GPU::GBCPixelQuad*>(&this->mem.palette[palette]);

	GPU::GBColor color;
	switch (index)
	{
	case 0: color = quad->_1; break;
	case 1: color = quad->_2; break;
	case 2: color = quad->_3; break;
	case 3: color = quad->_4; break;
	default: color = GBC_WHITE; break;
	}

	return GPU::GetGBColor(color);
}

DWORD GPU::GetGBColor(GPU::GBColor pixel)
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
