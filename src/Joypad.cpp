#include "STDInclude.hpp"

Joypad::Joypad(GameBoy* gameBoy) : gb(gameBoy), column(0)
{
	
}

Joypad::~Joypad()
{

}

unsigned char Joypad::read()
{
	unsigned char input = 0;

	if(this->column == 0x10)
	{
		input |= (!this->a()      ? KEY_A      : 0);
		input |= (!this->b()      ? KEY_B      : 0);
		input |= (!this->select() ? KEY_SELECT : 0);
		input |= (!this->start()  ? KEY_START  : 0);
	}
	else if (this->column == 0x20)
	{
		input |= (!this->up()    ? KEY_UP    : 0);
		input |= (!this->down()  ? KEY_DOWN  : 0);
		input |= (!this->left()  ? KEY_LEFT  : 0);
		input |= (!this->right() ? KEY_RIGHT : 0);
	}
	else input = 0xF;

	if((input & 0xF) != 0xF && this->gb->getMMU()->iE & 0x10)
	{
		this->gb->getMMU()->iF |= 0x10;
	}

	input |= this->column;

	return input;
}

void Joypad::write(unsigned char val)
{
	this->column = val & 0x30;
}

bool Joypad::up()
{
	return Joypad::IsKeyPressed(VK_UP);
}

bool Joypad::down()
{
	return Joypad::IsKeyPressed(VK_DOWN);
}

bool Joypad::left()
{
	return Joypad::IsKeyPressed(VK_LEFT);
}

bool Joypad::right()
{
	return Joypad::IsKeyPressed(VK_RIGHT);
}

bool Joypad::a()
{
	return Joypad::IsKeyPressed('S');
}

bool Joypad::b()
{
	return Joypad::IsKeyPressed('A');
}

bool Joypad::start()
{
	return Joypad::IsKeyPressed(VK_RETURN);
}

bool Joypad::select()
{
	return Joypad::IsKeyPressed(VK_SPACE);
}

bool Joypad::IsKeyPressed(int vk)
{
	return (GetKeyState(vk) & 0x8000) != 0;
}
