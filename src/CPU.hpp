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

	unsigned char m;
};
#pragma warning(pop)

class CPU
{
public:
	class Operation
	{
	public:
		unsigned char ticks;
		std::function<void(CPU*)> func;
	};

	CPU(std::shared_ptr<MMU> _mmu, std::shared_ptr<GPU> _gpu);
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

	Registers registers;

private:
	Operation operations[0x100];
	Operation callbacks[0x100];

	// Emulated memory management unit
	std::shared_ptr<MMU> mmu;

	// Emulated graphics processing unit
	std::shared_ptr<GPU> gpu;

	void setupOperations();
	void setupCallbacks();

	void verifyComponents();
};
