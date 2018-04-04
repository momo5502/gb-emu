#pragma once

struct Rom
{
	unsigned char padding[0x100];
	unsigned char entryPoint[4];
	unsigned char logo[48];
	char title[16];
	unsigned short publisher;
	unsigned char sgbFlag;
	unsigned char cartridgeType;
	unsigned char romSize;
	unsigned char ramSize;
	unsigned char destination;
	unsigned char oldPublisher;
	unsigned char romVersion;
	unsigned char headerChecksum;
	unsigned short globalChecksum;
};

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

	void skipBIOS();

private:

	Joypad joypad;
	CPU cpu;
	MMU mmu;
	GPU gpu;

	bool frame();
};
