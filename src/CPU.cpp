#include "STDInclude.hpp"

CPU::CPU(std::shared_ptr<MMU> _mmu, std::shared_ptr<GPU> _gpu)
{
	ZeroMemory(&this->registers, sizeof this->registers);

	this->mmu = _mmu;
	this->gpu = _gpu;
	this->verifyComponents();

	this->setupOperations();
	this->setupCallbacks();
}

CPU::~CPU()
{
	
}

void CPU::setupOperations()
{
	// NOP
	this->operations[0x00] = { 1, [](CPU*)
	{
		__nop();
	} };

	// LD BC,nn
	this->operations[0x01] = { 3, [](CPU* cpu)
	{
		cpu->registers.bc = cpu->readProgramWord();
	} };

	// LD (BC),A
	this->operations[0x02] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.bc, cpu->registers.a);
	} };

	// INC C
	this->operations[0x0C] = { 1, [](CPU* cpu)
	{
		cpu->registers.c++;
		cpu->registers.c &= 0xFF;

		cpu->registers.f = (cpu->registers.c ? 0 : FLAG_ZERO);
	} };

	// LD C,n
	this->operations[0x0E] = { 2, [](CPU* cpu)
	{
		cpu->registers.c = cpu->readProgramByte();
	} };

	// LD DE,nn
	this->operations[0x11] = { 3, [](CPU* cpu)
	{
		cpu->registers.de = cpu->readProgramWord();
	} };

	// LD (DE),A
	this->operations[0x12] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.de, cpu->registers.a);
	} };

	// LD E,n
	this->operations[0x1E] = { 2, [](CPU* cpu)
	{
		cpu->registers.e = cpu->readProgramByte();
	} };

	// JR NZ,n
	this->operations[0x20] = { 2, [](CPU* cpu)
	{
		unsigned char jumpLoc = cpu->readProgramByte();
		if (jumpLoc > 0x7F) jumpLoc = -((~jumpLoc + 1) & 0xFF);

		if (!(cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc += jumpLoc;
			cpu->registers.m++;
		}
	} };

	// LD HL,nn
	this->operations[0x21] = { 3, [](CPU* cpu)
	{
		cpu->registers.hl = cpu->readProgramWord();
	} };

	// LDI (HL),A
	this->operations[0x22] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
		cpu->registers.l = (cpu->registers.l + 1) & 0xFF;

		if (!cpu->registers.l)
		{
			cpu->registers.l = (cpu->registers.l + 1) & 0xFF;
		}
	} };

	// LD L,n
	this->operations[0x2E] = { 2, [](CPU* cpu)
	{
		cpu->registers.l = cpu->readProgramByte();
	} };

	// LD SP,nn
	this->operations[0x31] = { 3, [](CPU* cpu)
	{
		cpu->registers.sp = cpu->readProgramWord();
	} };

	// LDD (HL),A
	this->operations[0x32] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
		cpu->registers.l = (cpu->registers.l + 1) & 0xFF;

		if (!cpu->registers.l)
		{
			cpu->registers.h = (cpu->registers.h + 1) & 0xFF;
		}
	} };

	// LD A,n
	this->operations[0x3E] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->readProgramByte();
	} };

	// LD(HL), A
	this->operations[0x77] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
	} };

	// XOR A
	this->operations[0xAF] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.a;
		cpu->registers.a &= 0xFF;

		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// LDH (n),A
	this->operations[0xE0] = { 3, [](CPU* cpu)
	{
		cpu->mmu->writeByte(0xFF00 | cpu->readProgramByte(), cpu->registers.a);
	} };

	// LDH (C),A
	this->operations[0xE2] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(0xFF00 | cpu->registers.c, cpu->registers.a);
	} };

	// Ext ops (callbacks)
	this->operations[0xCB] = { 0, [](CPU* cpu)
	{
		cpu->executeCallback(cpu->readProgramByte());
	} };
}

void CPU::setupCallbacks()
{
	this->callbacks[0x7C] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= FLAG_HALF_CARRY - 1;
		cpu->registers.f |= FLAG_HALF_CARRY;
		cpu->registers.f = (cpu->registers.c & FLAG_ZERO) ? 0 : FLAG_ZERO;
	} };
}

unsigned char CPU::readProgramByte()
{
	unsigned short addr = this->registers.pc++;
	this->registers.pc &= 0xFFFF;
	return this->mmu->readByte(addr);
}

unsigned short CPU::readProgramWord()
{
	return this->readProgramByte() | (this->readProgramByte() << 8);
}

void CPU::stackPushWord(unsigned short value)
{
	this->registers.sp -= 2;
	this->mmu->writeWord(this->registers.sp, value);
}

void CPU::stackPushByte(unsigned char value)
{
	this->registers.sp -= 1;
	this->mmu->writeByte(this->registers.sp, value);
}

void CPU::loadProgram(std::string data)
{
	this->mmu->loadRom(std::basic_string<unsigned char>(data.begin(), data.end()));
}

bool CPU::execute()
{
	unsigned char instruction = this->readProgramByte();

	auto operation = &this->operations[instruction];

	if(operation->func)
	{
		try
		{
			operation->func(this);
			this->registers.m += operation->ticks;

			printf("Operation %X executed\n", instruction);

			this->gpu->frame();
			return true;
		}
		catch(std::exception e)
		{
			printf("Operation %X has thrown an exception: %s\n", instruction, e.what());
		}
		catch(...)
		{
			printf("Operation %X has thrown an unknown exception\n", instruction);
		}
	}
	else
	{
		printf("Unsupported instruction %X\n", instruction);
	}

	return false;
}

void CPU::executeCallback(unsigned char instruction)
{
	auto callback = &this->callbacks[instruction];

	if (callback->func)
	{
		try
		{
			callback->func(this);
			this->registers.m += callback->ticks;

			printf("Callback %X executed\n", instruction);
			return;
		}
		catch (std::exception e)
		{
			printf("Callback %X has thrown an exception: %s\n", instruction, e.what());
		}
		catch (...)
		{
			printf("Callback %X has thrown an exception\n", instruction);
		}
	}
	else
	{
		printf("Unsupported callback %X\n", instruction);
	}

	throw std::exception();
}

void CPU::verifyComponents()
{
	if (!this->mmu || !this->gpu)
	{
		throw std::runtime_error("Emulated hardware components not connected!");
	}
}
