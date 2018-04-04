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
	void executeExt(unsigned char instruction);

	Registers registers;
	Timer timer;

	void skipBIOS();

private:
	Operation operations[0x100];
	static const unsigned char OperationTicks[0x100];

	Operation extOperations[0x100];
	static const unsigned char ExtOperationTicks[0x100];

	bool ime;
	GameBoy* gb;
	Registers savRegisters;

	bool halted = false;

	void setupOperations();
	void setupExtOperations();

	void executeRst(unsigned short num);

	void addHL(unsigned short reg);

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

	void bit(unsigned char reg, unsigned char _bit);

	void rlc(unsigned char* reg);
	void rrc(unsigned char* reg);

	void rl(unsigned char* reg);
	void rr(unsigned char* reg);

	void sla(unsigned char* reg);
	void sra(unsigned char* reg);

	void swap(unsigned char* reg);
	void srl(unsigned char* reg);
};
