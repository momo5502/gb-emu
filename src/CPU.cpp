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
		cpu->stackPushWord(cpu->registers.pc);
		cpu->registers.pc = 0x0038;
		return true;
	} };
}

void CPU::stackPushWord(unsigned short value)
{
	this->registers.sp -= 2;
	this->mmu.writeWord(this->registers.sp, value);
}

void CPU::stackPushByte(unsigned char value)
{
	this->registers.sp -= 1;
	this->mmu.writeByte(this->registers.sp, value);
}

void CPU::loadProgram(std::string data)
{
	this->mmu.loadRom(std::basic_string<unsigned char>(data.begin(), data.end()));
}

bool CPU::execute()
{
	unsigned char instruction = this->mmu.readByte(this->registers.pc);

	auto operation = &this->operations[instruction];

	if(operation->callback)
	{
		auto dataPtr = this->mmu.getMemoryPtr(this->registers.pc);
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
