#pragma once
#include "display.hpp"

class game_boy;

class gpu
{
public:
	gpu(game_boy* game_boy);
	~gpu();

	void frame();

	unsigned char* get_memory_ptr(unsigned short address);
	void update_tile(unsigned short address);
	void update_object(unsigned short address, unsigned char value);
	void set_is_color_gb(bool value);

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

	game_boy* gb_;

	mode mode_;
	memory mem_{};
	unsigned int clock_;
	unsigned int last_time_ = 0;
	bool is_color_gb {false};

	color screen_buffer_[GB_WIDTH * GB_HEIGHT]{};
	unsigned char tiles_[512][8][8]{};
	object objects_[40]{};

	inline color get_color_from_palette(unsigned int palette, unsigned int index);

	static inline color get_gb_color(gb_color pixel);
	static inline color get_gb_color(unsigned char pixel);

	void render_screen();
	void render_texture() const;
};
