#pragma once

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
	game_boy();
	~game_boy();

	mmu* get_mmu() { return &this->mmu_; }
	gpu* get_gpu() { return &this->gpu_; }
	cpu* get_cpu() { return &this->cpu_; }
	joypad* get_joypad() { return &this->joypad_; }

	void load_rom(std::vector<uint8_t> data);
	void run();

	void skip_bios();

private:
	joypad joypad_;
	cpu cpu_;
	mmu mmu_;
	gpu gpu_;

	bool frame();
};
