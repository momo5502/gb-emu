#pragma once

struct gb_rom;
class cpu;

struct mbc
{
	int rom_bank = 0;
	int ram_bank = 0;
	int ram_on = 0;
	int mode = 0;
};

class mmu
{
public:
	mmu(game_boy* game_boy);
	~mmu();

	void load_rom(std::vector<uint8_t> data);

	unsigned char read_byte(unsigned short address);
	unsigned short read_word(unsigned short address);

	void write_byte(unsigned short address, unsigned char value);
	void write_word(unsigned short address, unsigned short value);

	unsigned char* get_memory_ptr(unsigned short address);

	void control_mbc(unsigned short address, unsigned char value);

	void mark_bios_pass();

	unsigned char vram[0x2000]{};
	unsigned char eram[0x8000]{};
	unsigned char wram[0x2000]{};
	unsigned char zram[0x7F]{};

	unsigned char i_f{};
	unsigned char i_e{};

	unsigned int rom_offset = 0x4000;
	unsigned int ram_offset = 0x0000;
	unsigned char cartridge_type = 0;

	mbc mbc[4]{};

	gb_rom* get_rom();

private:
	static unsigned char bios_[256];

	std::vector<uint8_t> rom_;

	unsigned char oam_[160];

	unsigned char zero_[2];

	bool passed_bios_;

	game_boy* gb_;
};
