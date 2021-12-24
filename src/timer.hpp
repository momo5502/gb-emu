#pragma once

class game_boy;
class cpu;

class timer
{
public:
	timer();
	~timer();

	void increment(game_boy* gb);

	unsigned int div;
	unsigned int tma;
	unsigned int tima;
	unsigned int tac;

	unsigned int main_clock;
	unsigned int sub_clock;
	unsigned int div_clock;

private:
	void step(game_boy* gb);
};
