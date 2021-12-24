#include "std_include.hpp"

GameBoy::GameBoy() : joypad(this), cpu(this), mmu(this), gpu(this)
{

}

GameBoy::~GameBoy()
{

}

void GameBoy::run()
{
	while (this->frame());
	this->gpu.closeWindow();
}

void GameBoy::skipBIOS()
{
	this->cpu.skipBIOS();
}

bool GameBoy::frame()
{
	unsigned int endTick = this->cpu.registers.m + 17556;

	auto start = std::chrono::high_resolution_clock::now();

	while (this->cpu.registers.m < endTick)
	{
		if (!this->cpu.execute())
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

void GameBoy::loadRom(std::string data)
{
	this->mmu.loadRom(std::basic_string<unsigned char>(data.begin(), data.end()));

	if (data.size() >= sizeof(Rom))
	{
		Rom* rom = reinterpret_cast<Rom*>(data.data());

		std::string romName(rom->title, 16);
		while (!romName.empty() && !romName.back()) romName.pop_back();
		this->gpu.setTitle(romName);
	}
}
