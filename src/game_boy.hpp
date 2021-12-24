#pragma once
#include "joypad.hpp"
#include "display.hpp"

#include "cpu.hpp"
#include "gpu.hpp"
#include "mmu.hpp"
#include "input.hpp"

struct gb_rom
{
	unsigned char padding[0x100];
	unsigned char entry_point[4];
	unsigned char logo[48];
	char title[16];
	unsigned short publisher;
	unsigned char sgb_flag;
	unsigned char cartridge_type;
	unsigned char rom_size;
	unsigned char ram_size;
	unsigned char destination;
	unsigned char old_publisher;
	unsigned char rom_version;
	unsigned char header_checksum;
	unsigned short global_checksum;
};

class game_boy
{
public:
	game_boy(joypad* joypad, display* display);
	~game_boy();

	mmu* get_mmu() { return &this->mmu_; }
	gpu* get_gpu() { return &this->gpu_; }
	cpu* get_cpu() { return &this->cpu_; }
	input* get_input() { return &this->input_; }
	joypad* get_joypad() const { return this->joypad_; }
	display* get_display() const { return this->display_; }

	void load_rom(std::vector<uint8_t> data);
	void run();

	void skip_bios();

	void turn_off();

private:
	joypad* joypad_;
	display* display_;
	input input_;
	cpu cpu_;
	mmu mmu_;
	gpu gpu_;
	std::atomic<bool> off_{false};

	bool frame();
};
