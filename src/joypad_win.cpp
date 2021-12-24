#include "std_include.hpp"
#include "joypad_win.hpp"
#include "display_win.hpp"

joypad_win::joypad_win(display_win* display)
	: display_(display)
{
}

bool joypad_win::is_up_pressed()
{
	return this->is_key_pressed(VK_UP);
}

bool joypad_win::is_down_pressed()
{
	return this->is_key_pressed(VK_DOWN);
}

bool joypad_win::is_left_pressed()
{
	return this->is_key_pressed(VK_LEFT);
}

bool joypad_win::is_right_pressed()
{
	return this->is_key_pressed(VK_RIGHT);
}

bool joypad_win::is_a_pressed()
{
	return this->is_key_pressed('S');
}

bool joypad_win::is_b_pressed()
{
	return this->is_key_pressed('A');
}

bool joypad_win::is_start_pressed()
{
	return this->is_key_pressed(VK_RETURN);
}

bool joypad_win::is_select_pressed()
{
	return this->is_key_pressed(VK_SPACE);
}

bool joypad_win::is_key_pressed(const int vk) const
{
	return (GetKeyState(vk) & 0x8000) != 0 && (!this->display_ || this->display_->is_window_active());
}
