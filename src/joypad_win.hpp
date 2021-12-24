#pragma once
#include "joypad.hpp"

class display_win;

class joypad_win : public joypad
{
public:
	joypad_win(display_win* display);

	bool is_up_pressed() override;
	bool is_down_pressed() override;
	bool is_left_pressed() override;
	bool is_right_pressed() override;

	bool is_a_pressed() override;
	bool is_b_pressed() override;

	bool is_start_pressed() override;
	bool is_select_pressed() override;

private:
	display_win* display_{};

	bool is_key_pressed(int vk) const;
};
