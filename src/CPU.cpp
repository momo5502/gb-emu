#include "STDInclude.hpp"

CPU::CPU()
{
	ZeroMemory(&this->registers, sizeof this->registers);
	this->setupOperations();
}

CPU::~CPU()
{
	
}

void CPU::setupOperations()
{
	// NOP
	this->operations[0x00] = { 2, 0, [](CPU*, const unsigned char*)
	{
		return true;
	} };

	// RST 38h
	this->operations[0xFF] = { 8, 0, [](CPU* cpu, const unsigned char*)
	{
		cpu->stackPush(cpu->registers.pc);
		cpu->registers.pc = 0x0038;
		return true;
	} };
}

void CPU::stackPush(unsigned short /*value*/)
{
	
}

void CPU::loadProgram(std::string _data)
{
	this->data.clear();
	this->data.append(_data.begin(), _data.end());
}

bool CPU::execute()
{
	if (this->data.size() <= this->registers.pc) return false;
	unsigned char instruction = this->data[this->registers.pc];

	auto operation = &this->operations[instruction];

	if(operation->callback)
	{
		auto dataPtr = &this->data[this->registers.pc];
		this->registers.pc += operation->params;

		operation->callback(this, dataPtr);

		this->ticks += operation->ticks;

		printf("Operation %X executed\n", instruction);
		return true;
	}
	else
	{
		printf("Unsupported instruction %X\n", instruction);
	}

	return false;
}
