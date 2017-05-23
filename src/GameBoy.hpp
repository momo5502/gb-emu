#pragma once

class GameBoy
{
public:
	GameBoy();
	~GameBoy();

	MMU* getMMU() { return &this->mmu; }
	GPU* getGPU() { return &this->gpu; }
	CPU* getCPU() { return &this->cpu; }
	Joypad* getJoypad() { return &this->joypad; }

	void loadRom(std::string data);
	void run();

private:

	Joypad joypad;
	CPU cpu;
	MMU mmu;
	GPU gpu;

	bool frame();
};
