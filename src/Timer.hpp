#pragma once

class GameBoy;
class CPU;

class Timer
{
public:
	Timer();
	~Timer();

	void increment(GameBoy* gb);

	unsigned int div;
	unsigned int tma;
	unsigned int tima;
	unsigned int tac;

	unsigned int mainClock;
	unsigned int subClock;
	unsigned int divClock;

private:
	void step(GameBoy* gb);
};
