#include "STDInclude.hpp"

Timer::Timer() : div(0), tma(0), tima(0), tac(0), mainClock(0), subClock(0), divClock(0)
{

}

Timer::~Timer()
{

}

void Timer::increment(GameBoy* gb)
{
	this->subClock += gb->getCPU()->registers.m;

	if(this->subClock > 3)
	{
		this->mainClock++;
		this->subClock -= 4;

		this->divClock++;
		if(this->divClock == 16)
		{
			this->divClock = 0;
			this->div++;
			this->div &= 0xFF;
		}
	}

	if(this->tac & 4)
	{
		switch(this->tac & 3)
		{
		case 0:
			if (this->mainClock >= 64) this->step(gb);
			break;

		case 1:
			if (this->mainClock >= 1) this->step(gb);
			break;

		case 2:
			if (this->mainClock >= 4) this->step(gb);
			break;

		case 3:
			if (this->mainClock >= 16) this->step(gb);
			break;

		default:
			break;
		}
	}
}

void Timer::step(GameBoy* gb)
{
	this->tima++;
	this->mainClock = 0;

	if(this->tima > 0xFF)
	{
		this->tima = this->tma;
		if(gb->getMMU()->iE & 4) gb->getMMU()->iF |= 4;
	}
}
