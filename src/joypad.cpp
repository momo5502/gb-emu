#include "std_include.hpp"

joypad::joypad(game_boy* game_boy) : gb_(game_boy), column_(0)
{
	
}

joypad::~joypad()
{

}

unsigned char joypad::read()
{
	unsigned char input = 0;

	if(this->column_ == 0x10)
	{
		input |= (!this->a()      ? key_a      : 0);
		input |= (!this->b()      ? key_b      : 0);
		input |= (!this->select() ? key_select : 0);
		input |= (!this->start()  ? key_start  : 0);
	}
	else if (this->column_ == 0x20)
	{
		input |= (!this->up()    ? key_up    : 0);
		input |= (!this->down()  ? key_down  : 0);
		input |= (!this->left()  ? key_left  : 0);
		input |= (!this->right() ? key_right : 0);
	}
	else input = 0xF;

	if((input & 0xF) != 0xF && this->gb_->get_mmu()->i_e & 0x10)
	{
		this->gb_->get_mmu()->i_f |= 0x10;
	}

	input |= this->column_;

	return input;
}

void joypad::write(unsigned char val)
{
	this->column_ = val & 0x30;
}

bool joypad::up()
{
	return this->is_key_pressed(VK_UP);
}

bool joypad::down()
{
	return this->is_key_pressed(VK_DOWN);
}

bool joypad::left()
{
	return this->is_key_pressed(VK_LEFT);
}

bool joypad::right()
{
	return this->is_key_pressed(VK_RIGHT);
}

bool joypad::a()
{
	return this->is_key_pressed('S');
}

bool joypad::b()
{
	return this->is_key_pressed('A');
}

bool joypad::start()
{
	return this->is_key_pressed(VK_RETURN);
}

bool joypad::select()
{
	return this->is_key_pressed(VK_SPACE);
}

bool joypad::is_key_pressed(int vk)
{
	return (GetKeyState(vk) & 0x8000) != 0 && this->gb_->get_gpu()->is_window_active();
}
