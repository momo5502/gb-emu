#pragma once

class game_boy;

enum flags
{
	flag_zero = 0x80,
	flag_negative = 0x40,
	flag_half_carry = 0x20,
	flag_carry = 0x10,
};

#pragma warning(push)
#pragma warning(disable: 4201)
struct cpu_registers
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

class cpu
{
public:
	typedef void (*operation)(game_boy*);

	cpu(game_boy* game_boy);
	~cpu();

	void stack_push_word(unsigned short value);
	void stack_push_byte(unsigned char value);

	unsigned short stack_pop_word();
	unsigned char stack_pop_byte();

	unsigned char read_program_byte();
	unsigned short read_program_word();

	bool execute();
	void execute_ext(unsigned char instruction);

	cpu_registers registers;
	timer timer;

	void skip_bios();

private:
	operation operations_[0x100];
	static const unsigned char operation_ticks[0x100];

	operation ext_operations_[0x100];
	static const unsigned char ext_operation_ticks[0x100];

	bool ime_;
	game_boy* gb_;
	cpu_registers sav_registers_;

	bool halted_ = false;

	void setup_operations();
	void setup_ext_operations();

	void execute_rst(unsigned short num);

	void add_hl(unsigned short value);

	void inc(unsigned char* reg);
	void dec(unsigned char* reg);
	void add(unsigned char reg);
	void sub(unsigned char reg);
	void _and(unsigned char reg);
	void _xor(unsigned char reg);
	void _or(unsigned char reg);
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
