#pragma once

class CPU;

class MMU
{
public:
	MMU();
	~MMU();

	void connectCPU(CPU* _cpu);
	void loadRom(std::basic_string<unsigned char> data);

	unsigned char readByte(unsigned short address);
	unsigned short readWord(unsigned short address);

	void writeByte(unsigned short address, unsigned char value);
	void writeWord(unsigned short address, unsigned short value);

	unsigned char* getMemoryPtr(unsigned short address);

	void markBiosPass();

	unsigned char vram[0x2000];
	unsigned char eram[0x8000];
	unsigned char wram[0x2000];
	unsigned char zram[0x7F];

private:
	static unsigned char Bios[256];

	std::basic_string<unsigned char> rom;

	unsigned char oam[160];

	unsigned char zero[2];

	bool passedBios;

	CPU* cpu;
};
