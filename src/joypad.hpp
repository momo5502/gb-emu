#pragma once

class GameBoy;

class Joypad
{
public:
	Joypad(GameBoy* gameBoy);
	~Joypad();

	unsigned char read();
	void write(unsigned char val);

private:
	enum KeyBits
	{
		// High column (0x20)
		KEY_RIGHT = 1,
		KEY_LEFT  = 2,
		KEY_UP    = 4,
		KEY_DOWN  = 8,

		// Low column (0x10)
		KEY_A = 1,
		KEY_B = 2,
		KEY_SELECT = 4,
		KEY_START = 8,
	};

	GameBoy* gb;

	bool up();
	bool down();
	bool left();
	bool right();

	bool a();
	bool b();

	bool start();
	bool select();

	bool isKeyPressed(int vk);

	unsigned char column;
};
