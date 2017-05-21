#pragma once

class CPU;

class Timer
{
public:
	Timer();
	~Timer();

	void increment(CPU* cpu);

	unsigned int div;
	unsigned int tma;
	unsigned int tima;
	unsigned int tac;

	unsigned int mainClock;
	unsigned int subClock;
	unsigned int divClock;

private:
	void step(CPU* cpu);
};
