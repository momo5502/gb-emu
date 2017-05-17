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
};
#pragma warning(pop)

class CPU
{
public:
	class Operation
	{
	public:
		unsigned char ticks;
		unsigned char params;
		std::function<bool(CPU*, const unsigned char*)> callback;
	};

	CPU();
	~CPU();

	void stackPush(unsigned short value);

	void loadProgram(std::string data);
	bool execute();

	Registers registers;

private:
	Operation operations[256];
	std::basic_string<unsigned char> data;
	unsigned int ticks;

	void setupOperations();
};
