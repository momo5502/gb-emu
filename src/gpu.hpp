#pragma once

#include <Windows.h>

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

#define WM_KILL_WINDOW (WM_USER+0)

class game_boy;

class gpu
{
public:
	gpu(game_boy* game_boy);
	~gpu();

	void frame();
	bool working();

	unsigned char* get_memory_ptr(unsigned short address);
	void update_tile(unsigned short address);
	void update_object(unsigned short address, unsigned char value);
	void close_window();

	void set_title(std::string title);

	bool is_window_active() const;

private:
	struct memory
	{
		unsigned char flags;
		unsigned char lcd_status;
		unsigned char yscrl;
		unsigned char xscrl;
		unsigned char curline;
		unsigned char raster;
		unsigned char unk;
		unsigned char palette[3];

		unsigned char reg[0xFF];
	};

	struct object
	{
		int y;
		int x;
		int tile;
		int palette;
		int x_flip;
		int y_flip;
		int priority;
	};

	enum gb_color : unsigned char
	{
		gbc_white = 0,
		gbc_light_gray = 1,
		gbc_dark_gray = 2,
		gbc_black = 3,
	};

	enum mode
	{
		mode_hblank = 0,
		mode_vblank = 1,
		mode_oam = 2,
		mode_vram = 3
	};

	struct gbc_pixel_quad
	{
		gb_color _1 : 2;
		gb_color _2 : 2;
		gb_color _3 : 2;
		gb_color _4 : 2;
	};

	enum flags
	{
		flag_background_on = (1 << 0),
		flag_sprites_on = (1 << 1),
		flag_sprites_size = (1 << 2),
		flag_alt_tile_map = (1 << 3),
		flag_alt_tile_set = (1 << 4),
		flag_window_on = (1 << 5),
		flag_alt_window_tile_map = (1 << 6),
		flag_display_on = (1 << 7)
	};

	HWND window_;

	game_boy* gb_;

	mode mode_;
	memory mem_{};
	unsigned int clock_;
	unsigned int last_time_ = 0;

	std::thread window_thread_;

	COLORREF screen_buffer_[GB_WIDTH * GB_HEIGHT]{};
	unsigned char tiles_[512][8][8]{};
	object objects_[40]{};

	inline COLORREF get_color_from_palette(unsigned int palette, unsigned int index);

	static inline COLORREF get_gb_color(gb_color pixel);
	static inline COLORREF get_gb_color(unsigned char pixel);

	static LRESULT CALLBACK window_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
	LRESULT window_proc(UINT message, WPARAM w_param, LPARAM l_param);

	void render_screen();
	void render_texture();

	void window_runner();
};
