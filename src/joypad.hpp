#pragma once

class game_boy;

class joypad
{
public:
	joypad(game_boy* game_boy);
	~joypad();

	unsigned char read() const;
	void write(unsigned char val);

private:
	enum key_bits
	{
		// High column (0x20)
		key_right = 1,
		key_left = 2,
		key_up = 4,
		key_down = 8,

		// Low column (0x10)
		key_a = 1,
		key_b = 2,
		key_select = 4,
		key_start = 8,
	};

	game_boy* gb_;

	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;

	bool a() const;
	bool b() const;

	bool start() const;
	bool select() const;

	bool is_key_pressed(int vk) const;

	unsigned char column_;
};
