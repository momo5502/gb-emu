#include "std_include.hpp"

game_boy::game_boy() : joypad_(this), cpu_(this), mmu_(this), gpu_(this)
{
}

game_boy::~game_boy() = default;

void game_boy::run()
{
	while (this->frame())
	{
	}
	this->gpu_.close_window();
}

void game_boy::skip_bios()
{
	this->cpu_.skip_bios();
}

bool game_boy::frame()
{
	const unsigned int end_tick = this->cpu_.registers.m + 17556;

	const auto start = std::chrono::high_resolution_clock::now();

	while (this->cpu_.registers.m < end_tick)
	{
		if (!this->cpu_.execute())
		{
			return false;
		}
	}

	const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now() - start);

	if (delta < (15ms))
	{
		std::this_thread::sleep_for((15ms) - delta);
	}

	return true;
}

void game_boy::load_rom(std::vector<uint8_t> data)
{
	if (data.size() < sizeof(gb_rom))
	{
		throw std::runtime_error("Invalid rom");
	}

	auto* rom = reinterpret_cast<const gb_rom*>(data.data());

	std::string rom_name(rom->title, 16);
	while (!rom_name.empty() && !rom_name.back()) rom_name.pop_back();
	this->gpu_.set_title(rom_name);

	this->mmu_.load_rom(std::move(data));
}
