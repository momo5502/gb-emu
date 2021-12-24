#include "std_include.hpp"

gpu::gpu(game_boy* game_boy) : window_(nullptr), gb_(game_boy), mode_(mode_hblank), clock_(0)
{
	zero_object(this->mem_);
	zero_object(this->tiles_);
	zero_object(this->screen_buffer_);
	zero_object(this->objects_);

	this->window_thread_ = std::thread([this]
	{
		this->window_runner();
	});
	while (!this->working()) std::this_thread::sleep_for(1ms);
}

void gpu::window_runner()
{
	WNDCLASSEX wc;
	zero_object(wc);
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = gpu::window_proc;
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

	this->window_ = CreateWindowExA(NULL, "GBAWindow", "GB-EMU", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
	                                CW_USEDEFAULT, width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	SetWindowLongPtrA(this->window_, GWLP_USERDATA, LONG_PTR(this));

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

void gpu::render_texture()
{
	RECT rect;
	GetClientRect(this->window_, &rect);

	HDC hdc = GetDC(this->window_);
	HDC src = CreateCompatibleDC(hdc);
	HBITMAP map = CreateBitmap(GB_WIDTH, GB_HEIGHT, 1, 8 * 4, this->screen_buffer_);

	SelectObject(src, map);
	StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, src, 0, 0, GB_WIDTH, GB_HEIGHT, SRCCOPY);

	DeleteDC(src);
	DeleteObject(map);
	ReleaseDC(this->window_, hdc);
}

void gpu::render_screen()
{
	unsigned char scanrow[GB_WIDTH] = {0};

	if (this->mem_.flags & flag_background_on)
	{
		unsigned short linebase = GB_WIDTH * this->mem_.curline;
		unsigned short mapbase = ((this->mem_.flags & flag_alt_tile_map) ? 0x1C00 : 0x1800) + ((((this->mem_.curline +
			this->mem_.yscrl) & 255) >> 3) << 5);
		unsigned char y = (this->mem_.curline + this->mem_.yscrl) & 7;
		unsigned char x = this->mem_.xscrl & 7;
		unsigned char t = (this->mem_.xscrl >> 3) & 31;

		unsigned short tile = this->gb_->get_mmu()->vram[mapbase + t];
		for (int i = 0; i < GB_WIDTH; ++i)
		{
			if (this->mem_.flags & flag_alt_tile_set)
			{
			}
			else tile = 0x100 + static_cast<char>(tile);

			scanrow[i] = this->tiles_[tile][y][x];
			this->screen_buffer_[linebase + i] = this->get_color_from_palette(0, scanrow[i]);

			x++;
			if (x == 8)
			{
				t = (t + 1) & 31;
				x = 0;
				tile = this->gb_->get_mmu()->vram[mapbase + t];
			}
		}
	}
	if (this->mem_.flags & flag_sprites_on)
	{
		for (int i = 0; i < 40; i++)
		{
			auto obj = this->objects_[i];

			// Check if this sprite falls on this scanline
			if (obj.y <= this->mem_.curline && (obj.y + 8) > this->mem_.curline)
			{
				// Where to render on the canvas
				auto canvasoffs = (this->mem_.curline * 160 + obj.x);

				// If the sprite is Y-flipped,
				// use the opposite side of the tile
				unsigned char* tilerow;
				if (obj.y_flip)
				{
					tilerow = this->tiles_[obj.tile][7 - (this->mem_.curline - obj.y)];
				}
				else
				{
					tilerow = this->tiles_[obj.tile][this->mem_.curline - obj.y];
				}

				for (int x = 0; x < 8; x++)
				{
					// If this pixel is still on-screen, AND
					// if it's not colour 0 (transparent), AND
					// if this sprite has priority OR shows under the bg
					// then render the pixel
					if ((obj.x + x) >= 0 && (obj.x + x) < 160 && tilerow[obj.x_flip ? (7 - x) : x] && (obj.priority || !
						scanrow[obj.x + x]))
					{
						// If the sprite is X-flipped,
						// write pixels in reverse order
						auto colour = this->get_color_from_palette(1 + (obj.palette != 0),
						                                           tilerow[obj.x_flip ? (7 - x) : x]);

						this->screen_buffer_[canvasoffs] = colour;
						canvasoffs++;
					}
				}
			}
		}
	}
}

void gpu::frame()
{
	int time = this->gb_->get_cpu()->registers.m;
	this->clock_ += time - this->last_time_;
	this->last_time_ = time;

	switch (this->mode_)
	{
	case mode_hblank:
		{
			if (this->clock_ >= 51)
			{
				this->clock_ -= 51;
				this->mem_.curline++;

				if (this->mem_.curline == GB_HEIGHT)
				{
					this->mode_ = mode_vblank;
					this->render_texture();
					if (this->gb_->get_mmu()->i_e & 1) this->gb_->get_mmu()->i_f |= 1;
					if (this->mem_.lcd_status & (1 << 4) && this->gb_->get_mmu()->i_e & 2)
						this->gb_->get_mmu()->i_f |=
							2;
				}
				else
				{
					this->mode_ = mode_oam;
					if (this->mem_.lcd_status & (1 << 5) && this->gb_->get_mmu()->i_e & 2)
						this->gb_->get_mmu()->i_f |=
							2;
				}
			}
			break;
		}

	case mode_vblank:
		{
			if (this->clock_ >= 114)
			{
				this->clock_ -= 114;
				this->mem_.curline++;

				if (this->mem_.curline > 153)
				{
					this->mode_ = mode_oam;
					this->mem_.curline = 0;

					if (this->mem_.lcd_status & (1 << 5) && this->gb_->get_mmu()->i_e & 2)
						this->gb_->get_mmu()->i_f |=
							2;
				}
			}
			break;
		}

	case mode_oam:
		{
			if (this->clock_ >= 20)
			{
				this->clock_ -= 20;
				this->mode_ = mode_vram;
			}
			break;
		}

	case mode_vram:
		{
			if (this->clock_ >= 43)
			{
				this->clock_ -= 43;
				this->mode_ = mode_hblank;
				if (this->mem_.lcd_status & (1 << 3) && this->gb_->get_mmu()->i_e & 2) this->gb_->get_mmu()->i_f |= 2;

				if (this->mem_.flags & flag_display_on)
				{
					this->render_screen();
				}
			}
			break;
		}
	}
}

bool gpu::working()
{
	return (IsWindow(this->window_) != FALSE);
}

unsigned char* gpu::get_memory_ptr(unsigned short address)
{
	address -= 0xFF40;

	if (address < sizeof(this->mem_))
	{
		auto pointer = reinterpret_cast<unsigned char*>(&this->mem_) + address;

		if (pointer == &this->mem_.lcd_status)
		{
			this->mem_.lcd_status = static_cast<unsigned char>(this->mode_ | (this->mem_.curline == this->mem_.raster
				                                                                  ? 0
				                                                                  : 0));
		}

		return pointer;
	}

	return nullptr;
}

void gpu::update_object(unsigned short address, unsigned char value)
{
	int obj = (address - 0xFE00) >> 2;
	if (obj < 40)
	{
		switch (address & 3)
		{
			// Y-coordinate
		case 0: this->objects_[obj].y = value - 16;
			break;

			// X-coordinate
		case 1: this->objects_[obj].x = value - 8;
			break;

			// Data tile
		case 2: this->objects_[obj].tile = value;
			break;

			// Options
		case 3:
			this->objects_[obj].palette = (value & 0x10) ? 1 : 0;
			this->objects_[obj].x_flip = (value & 0x20) ? 1 : 0;
			this->objects_[obj].y_flip = (value & 0x40) ? 1 : 0;
			this->objects_[obj].priority = (value & 0x80) ? 1 : 0;
			break;
		}
	}
}

void gpu::update_tile(unsigned short addr)
{
	addr &= ~1;
	unsigned short saddr = addr;
	unsigned short tile = (addr >> 4) & 511;
	unsigned short y = (addr >> 1) & 7;
	unsigned short sx;
	for (unsigned short x = 0; x < 8; x++)
	{
		sx = 1 << (7 - x);

		unsigned char var = (this->gb_->get_mmu()->vram[saddr & 0x1FFF] & sx) ? 1 : 0;
		var |= (this->gb_->get_mmu()->vram[saddr & 0x1FFF + 1] & sx) ? 2 : 0;
		var &= 3;

		this->tiles_[tile][y][x] = var;
	}
}

COLORREF gpu::get_color_from_palette(unsigned int palette, unsigned int index)
{
	if (palette > 3 || index > 4) return 0;

	gpu::gbc_pixel_quad* quad = reinterpret_cast<gpu::gbc_pixel_quad*>(&this->mem_.palette[palette]);

	gpu::gb_color color;
	switch (index)
	{
	case 0: color = quad->_1;
		break;
	case 1: color = quad->_2;
		break;
	case 2: color = quad->_3;
		break;
	case 3: color = quad->_4;
		break;
	default: color = gbc_white;
		break;
	}

	return gpu::get_gb_color(color);
}

COLORREF gpu::get_gb_color(gpu::gb_color pixel)
{
	switch (pixel)
	{
	case gbc_black: return RGB(0, 0, 0);
	case gbc_dark_gray: return RGB(192, 192, 192);
	case gbc_light_gray: return RGB(96, 96, 96);
	case gbc_white: return RGB(255, 255, 255);
	}
	return 0;
}

COLORREF gpu::get_gb_color(unsigned char pixel)
{
	return gpu::get_gb_color(*reinterpret_cast<gpu::gb_color*>(&pixel));
}

LRESULT gpu::window_proc(UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
	case WM_SIZE:
		{
			this->render_texture();
			break;
		}

	case WM_KILL_WINDOW:
		{
			DestroyWindow(this->window_);
			return 0;
		}

	default: break;
	}

	return DefWindowProc(this->window_, message, w_param, l_param);
}

LRESULT CALLBACK gpu::window_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	gpu* gpu_ = reinterpret_cast<gpu*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA));
	if (gpu_) return gpu_->window_proc(message, w_param, l_param);
	return DefWindowProc(h_wnd, message, w_param, l_param);
}

gpu::~gpu()
{
	this->close_window();
	if (this->window_thread_.joinable()) this->window_thread_.join();
}

void gpu::close_window()
{
	if (this->working())
	{
		SendMessageA(this->window_, WM_KILL_WINDOW, NULL, NULL);
		this->window_ = nullptr;
	}
}

void gpu::set_title(std::string title)
{
	if (this->working()) SetWindowTextA(this->window_, utils::va("GB-EMU - %s", title.data()));
}

bool gpu::is_window_active() const
{
	return this->window_ && GetForegroundWindow() == this->window_;
}
