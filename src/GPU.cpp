#include "STDInclude.hpp"

GPU::GPU() : window(nullptr), cpu(nullptr), mode(MODE_HBLANK), clock(0)
{
	ZeroObject(this->mem);
	ZeroObject(this->tiles);
	ZeroObject(this->screenBuffer);

	this->windowThread = std::thread(std::bind(&GPU::windowRunner, this));
	while(!this->working()) std::this_thread::sleep_for(1ms);
}

void GPU::connectCPU(CPU* _cpu)
{
	this->cpu = _cpu;
}

void GPU::windowRunner()
{
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

	const int scale = 3;
	this->window = CreateWindowExA(NULL, "GBAWindow", "GB-EMU", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, GB_WIDTH * scale, GB_HEIGHT * scale, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	SetWindowLongPtrA(this->window, GWLP_USERDATA, LONG_PTR(this));

	while (this->working())
	{
		MSG msg;
		while (this->working() && PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		std::this_thread::sleep_for(1ms);
	}
}

void GPU::renderTexture()
{
	RECT rect;
	GetWindowRect(this->window, &rect);

	HDC hdc = GetDC(this->window);
	HDC src = CreateCompatibleDC(hdc);
	HBITMAP map = CreateBitmap(GB_WIDTH, GB_HEIGHT, 1, 8 * 4, this->screenBuffer);

	SelectObject(src, map);
	StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, src, 0, 0, GB_WIDTH, GB_HEIGHT, SRCCOPY);

	DeleteDC(src);
	DeleteObject(map);
	ReleaseDC(this->window, hdc);
}

void GPU::renderScreen()
{
	if (this->mem.flags & FLAG_BACKGROUND_ON)
	{
		unsigned short linebase = GB_WIDTH * this->mem.curline;
		unsigned short mapbase = ((this->mem.flags & FLAG_ALT_TILE_MAP) ? 0x1C00 : 0x1800) + ((((this->mem.curline + this->mem.yscrl) & 255) >> 3) << 5);
		unsigned char y = (this->mem.curline + this->mem.yscrl) & 7;
		unsigned char x = this->mem.xscrl & 7;
		unsigned char t = (this->mem.xscrl >> 3) & 31;

		unsigned short tile = this->cpu->getMMU()->vram[mapbase + t];
		for(int i = 0; i < GB_WIDTH; ++i)
		{
			if(this->mem.flags & FLAG_ALT_TILE_SET){}
			else tile = 0x100 + static_cast<char>(tile);

			this->screenBuffer[linebase + i] = this->getColorFromPalette(0, this->tiles[tile][y][x]);

			x++;
			if (x == 8)
			{
				t = (t + 1) & 31;
				x = 0;
				tile = this->cpu->getMMU()->vram[mapbase + t];
			}
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
				this->clock -= 51;
				this->mem.curline++;

				if(this->mem.curline == (GB_HEIGHT - 1))
				{
					this->mode = MODE_VBLANK;
					this->renderTexture();
					if(this->cpu->getMMU()->iE & 1) this->cpu->getMMU()->iF |= 1;
					if (this->mem.lcdStatus & (1 << 4) && this->cpu->getMMU()->iE & 2) this->cpu->getMMU()->iF |= 2;
				}
				else
				{
					this->mode = MODE_OAM;
					if (this->mem.lcdStatus & (1 << 5) && this->cpu->getMMU()->iE & 2) this->cpu->getMMU()->iF |= 2;
				}
			}
			break;
		}

		case MODE_VBLANK:
		{
			if(this->clock >= 114)
			{
				this->clock -= 114;
				this->mem.curline++;

				if(this->mem.curline > 153)
				{
					this->mode = MODE_OAM;
					this->mem.curline = 0;

					if (this->mem.lcdStatus & (1 << 5) && this->cpu->getMMU()->iE & 2) this->cpu->getMMU()->iF |= 2;
				}
			}
			break;
		}

		case MODE_OAM:
		{
			if(this->clock >= 20)
			{
				this->clock -= 20;
				this->mode = MODE_VRAM;
			}
			break;
		}

		case MODE_VRAM:
		{
			if(this->clock >= 43)
			{
				this->clock -= 43;
				this->mode = MODE_HBLANK;
				if (this->mem.lcdStatus & (1 << 3) && this->cpu->getMMU()->iE & 2) this->cpu->getMMU()->iF |= 2;

				if(this->mem.flags & FLAG_DISPLAY_ON)
				{
					this->renderScreen();
				}
			}
			break;
		}
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
	addr &= ~1;
	unsigned short saddr = addr;
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

COLORREF GPU::getColorFromPalette(unsigned int palette, unsigned int index)
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

COLORREF GPU::GetGBColor(GPU::GBColor pixel)
{
	switch(pixel)
	{
	case GBC_BLACK: return RGB(0, 0, 0);
	case GBC_DARK_GRAY: return RGB(192, 192, 192);
	case GBC_LIGHT_GRAY: return RGB(96, 96, 96);
	case GBC_WHITE: return RGB(255, 255, 255);
	}
	return 0;
}

COLORREF GPU::GetGBColor(unsigned char pixel)
{
	return GPU::GetGBColor(*reinterpret_cast<GPU::GBColor*>(&pixel));
}

LRESULT GPU::windowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			DestroyWindow(this->window);
			return 0;
		}

		default: return DefWindowProc(this->window, message, wParam, lParam);
	}
}

LRESULT CALLBACK GPU::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GPU* gpu = reinterpret_cast<GPU*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (gpu) gpu->windowProc(message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

GPU::~GPU()
{
	if (this->working()) DestroyWindow(this->window);
	if(this->windowThread.joinable()) this->windowThread.join();
}
