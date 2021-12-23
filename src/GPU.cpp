#include "STDInclude.hpp"

GPU::GPU(GameBoy* gameBoy) : window(nullptr), gb(gameBoy), mode(MODE_HBLANK), clock(0)
{
	ZeroObject(this->mem);
	ZeroObject(this->tiles);
	ZeroObject(this->screenBuffer);
	ZeroObject(this->objects);

	this->windowThread = std::thread([this]
	{
		this->windowRunner();
	});
	while(!this->working()) std::this_thread::sleep_for(1ms);
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
	wc.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(102));
	wc.hIconSm = wc.hIcon;
	wc.hbrBackground = HBRUSH(COLOR_WINDOW);
	wc.lpszClassName = L"GBAWindow";
	RegisterClassEx(&wc);

	const int scale = 3;

	int width = GB_WIDTH * scale;
	int height = GB_HEIGHT * scale;

	this->window = CreateWindowExA(NULL, "GBAWindow", "GB-EMU", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	SetWindowLongPtrA(this->window, GWLP_USERDATA, LONG_PTR(this));

	MSG msg;
	while (this->working())
	{
		if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			std::this_thread::sleep_for(1ms);
		}
	}
}

void GPU::renderTexture()
{
	RECT rect;
	GetClientRect(this->window, &rect);

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
	unsigned char scanrow[GB_WIDTH] = { 0 };

	if (this->mem.flags & FLAG_BACKGROUND_ON)
	{
		unsigned short linebase = GB_WIDTH * this->mem.curline;
		unsigned short mapbase = ((this->mem.flags & FLAG_ALT_TILE_MAP) ? 0x1C00 : 0x1800) + ((((this->mem.curline + this->mem.yscrl) & 255) >> 3) << 5);
		unsigned char y = (this->mem.curline + this->mem.yscrl) & 7;
		unsigned char x = this->mem.xscrl & 7;
		unsigned char t = (this->mem.xscrl >> 3) & 31;

		unsigned short tile = this->gb->getMMU()->vram[mapbase + t];
		for(int i = 0; i < GB_WIDTH; ++i)
		{
			if(this->mem.flags & FLAG_ALT_TILE_SET){}
			else tile = 0x100 + static_cast<char>(tile);

			scanrow[i] = this->tiles[tile][y][x];
			this->screenBuffer[linebase + i] = this->getColorFromPalette(0, scanrow[i]);

			x++;
			if (x == 8)
			{
				t = (t + 1) & 31;
				x = 0;
				tile = this->gb->getMMU()->vram[mapbase + t];
			}
		}
	}
	if (this->mem.flags & FLAG_SPRITES_ON)
	{
		for (int i = 0; i < 40; i++)
		{
			auto obj = this->objects[i];

			// Check if this sprite falls on this scanline
			if (obj.y <= this->mem.curline && (obj.y + 8) > this->mem.curline)
			{
				// Where to render on the canvas
				auto canvasoffs = (this->mem.curline * 160 + obj.x);

				// If the sprite is Y-flipped,
				// use the opposite side of the tile
				unsigned char* tilerow;
				if (obj.yFlip)
				{
					tilerow = this->tiles[obj.tile][7 - (this->mem.curline - obj.y)];
				}
				else
				{
					tilerow = this->tiles[obj.tile][this->mem.curline - obj.y];
				}

				for (int x = 0; x < 8; x++)
				{
					// If this pixel is still on-screen, AND
					// if it's not colour 0 (transparent), AND
					// if this sprite has priority OR shows under the bg
					// then render the pixel
					if ((obj.x + x) >= 0 && (obj.x + x) < 160 && tilerow[obj.xFlip ? (7 - x) : x] && (obj.priority || !scanrow[obj.x + x]))
					{
						// If the sprite is X-flipped,
						// write pixels in reverse order
						auto colour = this->getColorFromPalette(1 + (obj.palette != 0), tilerow[obj.xFlip ? (7 - x) : x]);

						this->screenBuffer[canvasoffs] = colour;
						canvasoffs++;
					}
				}
			}
		}
	}
}

void GPU::frame()
{
	int time = this->gb->getCPU()->registers.m;
	this->clock += time - this->lastTime;
	this->lastTime = time;

	switch (this->mode)
	{
		case MODE_HBLANK:
		{
			if(this->clock >= 51)
			{
				this->clock -= 51;
				this->mem.curline++;

				if(this->mem.curline == GB_HEIGHT)
				{
					this->mode = MODE_VBLANK;
					this->renderTexture();
					if(this->gb->getMMU()->iE & 1) this->gb->getMMU()->iF |= 1;
					if (this->mem.lcdStatus & (1 << 4) && this->gb->getMMU()->iE & 2) this->gb->getMMU()->iF |= 2;
				}
				else
				{
					this->mode = MODE_OAM;
					if (this->mem.lcdStatus & (1 << 5) && this->gb->getMMU()->iE & 2) this->gb->getMMU()->iF |= 2;
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

					if (this->mem.lcdStatus & (1 << 5) && this->gb->getMMU()->iE & 2) this->gb->getMMU()->iF |= 2;
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
				if (this->mem.lcdStatus & (1 << 3) && this->gb->getMMU()->iE & 2) this->gb->getMMU()->iF |= 2;

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
		auto pointer = reinterpret_cast<unsigned char*>(&this->mem) + address;

		if (pointer == &this->mem.lcdStatus)
		{
			this->mem.lcdStatus = static_cast<unsigned char>(this->mode | (this->mem.curline == this->mem.raster ? 0 : 0));
		}

		return pointer;
	}

	return nullptr;
}

void GPU::updateObject(unsigned short address, unsigned char value)
{
	int obj = (address - 0xFE00) >> 2;
	if (obj < 40)
	{
		switch (address & 3)
		{
			// Y-coordinate
		case 0: this->objects[obj].y = value - 16; break;

			// X-coordinate
		case 1: this->objects[obj].x = value - 8; break;

			// Data tile
		case 2: this->objects[obj].tile = value; break;

			// Options
		case 3:
			this->objects[obj].palette = (value & 0x10) ? 1 : 0;
			this->objects[obj].xFlip = (value & 0x20) ? 1 : 0;
			this->objects[obj].yFlip = (value & 0x40) ? 1 : 0;
			this->objects[obj].priority = (value & 0x80) ? 1 : 0;
			break;
		}
	}
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

		unsigned char var = (this->gb->getMMU()->vram[saddr & 0x1FFF] & sx) ? 1 : 0;
		var |= (this->gb->getMMU()->vram[saddr & 0x1FFF + 1] & sx) ? 2 : 0;
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
		case WM_SIZE:
		{
			this->renderTexture();
			break;
		}

		case WM_KILL_WINDOW:
		{
			DestroyWindow(this->window);
			return 0;
		}

		default: break;
	}

	return DefWindowProc(this->window, message, wParam, lParam);
}

LRESULT CALLBACK GPU::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GPU* gpu = reinterpret_cast<GPU*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (gpu) return gpu->windowProc(message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

GPU::~GPU()
{
	this->closeWindow();
	if(this->windowThread.joinable()) this->windowThread.join();
}

void GPU::closeWindow()
{
	if (this->working())
	{
		SendMessageA(this->window, WM_KILL_WINDOW, NULL, NULL);
		this->window = nullptr;
	}
}

void GPU::setTitle(std::string title)
{
	if(this->working()) SetWindowTextA(this->window, Utils::VA("GB-EMU - %s", title.data()));
}

bool GPU::isWindowActive()
{
	return this->window && GetForegroundWindow() == this->window;
}
