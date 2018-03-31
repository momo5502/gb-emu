#pragma once

struct Rom;
class CPU;

class MBC
{
public:
	int romBank = 0;
	int ramBank = 0;
	int ramOn = 0;
	int mode = 0;
};

class MMU
{
public:
	MMU(GameBoy* gameBoy);
	~MMU();

	void loadRom(std::basic_string<unsigned char> data);

	unsigned char readByte(unsigned short address);
	unsigned short readWord(unsigned short address);

	void writeByte(unsigned short address, unsigned char value);
	void writeWord(unsigned short address, unsigned short value);

	unsigned char* getMemoryPtr(unsigned short address);

	void controlMBC(unsigned short address, unsigned char value);

	void markBiosPass();

	unsigned char vram[0x2000];
	unsigned char eram[0x8000];
	unsigned char wram[0x2000];
	unsigned char zram[0x7F];

	unsigned char iF;
	unsigned char iE;

	unsigned int romOffset = 0x4000;
	unsigned int ramOffset = 0x0000;
	unsigned char cartridgeType = 0;

	MBC mbc[4];

	Rom* getRom();

private:
	static unsigned char Bios[256];

	std::basic_string<unsigned char> rom;

	unsigned char oam[160];

	unsigned char zero[2];

	bool passedBios;

	GameBoy* gb;
};
