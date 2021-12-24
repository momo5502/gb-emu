#include "std_include.hpp"

game_boy::game_boy() : joypad_(this), cpu_(this), mmu_(this), gpu_(this)
{

}

game_boy::~game_boy()
{

}

void game_boy::run()
{
	while (this->frame());
	this->gpu_.close_window();
}

void game_boy::skip_bios()
{
	this->cpu_.skip_bios();
}

bool game_boy::frame()
{
	unsigned int endTick = this->cpu_.registers.m + 17556;

	auto start = std::chrono::high_resolution_clock::now();

	while (this->cpu_.registers.m < endTick)
	{
		if (!this->cpu_.execute())
		{
			return false;
		}
	}

	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

	if (delta < (15ms))
	{
		std::this_thread::sleep_for((15ms) - delta);
	}

	return true;
}

void game_boy::load_rom(std::string data)
{
	this->mmu_.load_rom(std::basic_string<unsigned char>(data.begin(), data.end()));

	if (data.size() >= sizeof(gb_rom))
	{
		gb_rom* rom = reinterpret_cast<gb_rom*>(data.data());

		std::string romName(rom->title, 16);
		while (!romName.empty() && !romName.back()) romName.pop_back();
		this->gpu_.set_title(romName);
	}
}
