#pragma once

class GameBoy;

enum Flags
{
	FLAG_ZERO = 0x80,
	FLAG_NEGATIVE = 0x40,
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
	typedef void(*Operation)(GameBoy*);

	CPU(GameBoy* gameBoy);
	~CPU();

	void stackPushWord(unsigned short value);
	void stackPushByte(unsigned char value);

	unsigned short stackPopWord();
	unsigned char stackPopByte();

	unsigned char readProgramByte();
	unsigned short readProgramWord();

	bool execute();
	void executeCallback(unsigned char instruction);

	Registers registers;
	Timer timer;

private:
	Operation operations[0x100];
	static const unsigned char OperationTicks[0x100];

	Operation callbacks[0x100];
	static const unsigned char CallbackTicks[0x100];

	bool ime;
	GameBoy* gb;
	Registers savRegisters;

	void setupOperations();
	void setupCallbacks();

	void executeRst(unsigned short num);

	void inc(unsigned char* reg);
	void dec(unsigned char* reg);
	void add(unsigned char reg);
	void sub(unsigned char reg);
	void and(unsigned char reg);
	void xor(unsigned char reg);
	void or(unsigned char reg);
	void cp(unsigned char reg);
	void adc(unsigned char reg);
	void sbc(unsigned char reg);
};
