#pragma once

enum Flags
{
	FLAG_ZERO = 0x80,
	FLAG_NIBBLE = 0x40,
	FLAG_HALF_CARRY = 0x20,
	FLAG_CARRY = 0x10,
};

#pragma warning(push)
#pragma warning(disable: 4201)
struct Registers
{
	union
	{
		struct
		{
			unsigned char f;
			unsigned char a;
		};

		unsigned short af;
	};

	union
	{
		struct
		{
			unsigned char c;
			unsigned char b;
		};

		unsigned short bc;
	};

	union
	{
		struct
		{
			unsigned char e;
			unsigned char d;
		};

		unsigned short de;
	};

	union
	{
		struct
		{
			unsigned char l;
			unsigned char h;
		};

		unsigned short hl;
	};

	unsigned short sp;
	unsigned short pc;

	unsigned int m;
};
#pragma warning(pop)

class CPU
{
public:
	struct Operation
	{
		unsigned char ticks;
		void(*func)(CPU*);
	};

	CPU(std::unique_ptr<MMU> _mmu, std::unique_ptr<GPU> _gpu);
	~CPU();

	void stackPushWord(unsigned short value);
	void stackPushByte(unsigned char value);

	unsigned short stackPopWord();
	unsigned char stackPopByte();

	unsigned char readProgramByte();
	unsigned short readProgramWord();

	void loadProgram(std::string data);

	bool execute();
	void executeCallback(unsigned char instruction);

	bool runFrame();

	MMU* getMMU() { return this->mmu.get(); }
	GPU* getGPU() { return this->gpu.get(); }

	Registers registers;

private:
	Operation operations[0x100];
	Operation callbacks[0x100];

	Registers savRegisters;

	bool ime;

	// Emulated memory management unit
	std::unique_ptr<MMU> mmu;

	// Emulated graphics processing unit
	std::unique_ptr<GPU> gpu;

	void setupOperations();
	void setupCallbacks();

	void verifyComponents();
};
