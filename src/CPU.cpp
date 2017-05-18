#include "STDInclude.hpp"

CPU::CPU(std::shared_ptr<MMU> _mmu, std::shared_ptr<GPU> _gpu)
{
	ZeroMemory(&this->registers, sizeof this->registers);

	this->mmu = _mmu;
	this->gpu = _gpu;

	this->mmu->connectGPU(this->gpu);

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

	// INC BC
	this->operations[0x03] = { 1, [](CPU* cpu)
	{
		cpu->registers.bc++;
	} };

	// DEC B
	this->operations[0x05] = { 1, [](CPU* cpu)
	{
		cpu->registers.b--;
		cpu->registers.f = cpu->registers.b ? 0 : FLAG_ZERO;
	} };

	// LD B,n
	this->operations[0x06] = { 2, [](CPU* cpu)
	{
		cpu->registers.b = cpu->readProgramByte();
	} };

	// INC C
	this->operations[0x0C] = { 1, [](CPU* cpu)
	{
		cpu->registers.c++;
		cpu->registers.f = (cpu->registers.c ? 0 : FLAG_ZERO);
	} };

	// DEC C
	this->operations[0x0D] = { 1, [](CPU* cpu)
	{
		cpu->registers.c--;
		cpu->registers.f = cpu->registers.c ? 0 : FLAG_ZERO;
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

	// INC DE
	this->operations[0x13] = { 1, [](CPU* cpu)
	{
		cpu->registers.de++;
	} };

	// DEC D
	this->operations[0x15] = { 1, [](CPU* cpu)
	{
		cpu->registers.d--;
		cpu->registers.f = cpu->registers.d ? 0 : FLAG_ZERO;
	} };

	// LD D,n
	this->operations[0x16] = { 2, [](CPU* cpu)
	{
		cpu->registers.d = cpu->readProgramByte();
	} };

	// RL A
	this->operations[0x17] = { 1, [](CPU* cpu)
	{
		unsigned char ci = cpu->registers.f & FLAG_CARRY ? 1 : 0;
		unsigned char co = cpu->registers.a & FLAG_ZERO ? FLAG_CARRY : 0;

		cpu->registers.a = (cpu->registers.a << 1) + ci;
		cpu->registers.a &= 255;
		cpu->registers.f = (cpu->registers.f & 0xEF) + co;
	} };
	// LD A,(DE)
	this->operations[0x1A] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.de);
	} };

	// INC E
	this->operations[0x1C] = { 1, [](CPU* cpu)
	{
		cpu->registers.e++;
		cpu->registers.f = (cpu->registers.e ? 0 : FLAG_ZERO);
	} };

	// DEC E
	this->operations[0x1D] = { 1, [](CPU* cpu)
	{
		cpu->registers.e--;
		cpu->registers.f = cpu->registers.e ? 0 : FLAG_ZERO;
	} };

	// LD E,n
	this->operations[0x1E] = { 2, [](CPU* cpu)
	{
		cpu->registers.e = cpu->readProgramByte();
	} };

	// JR NZ,n
	this->operations[0x20] = { 2, [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();

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

	// INC HL
	this->operations[0x23] = { 1, [](CPU* cpu)
	{
		cpu->registers.hl++;
	} };

	// DEC H
	this->operations[0x25] = { 1, [](CPU* cpu)
	{
		cpu->registers.h--;
		cpu->registers.f = cpu->registers.h ? 0 : FLAG_ZERO;
	} };

	// LD H,n
	this->operations[0x26] = { 2, [](CPU* cpu)
	{
		cpu->registers.h = cpu->readProgramByte();
	} };

	// JR n
	this->operations[0x28] = { 2, [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();
		cpu->registers.pc += jumpLoc;
		cpu->registers.m++;
	} };

	// INC L
	this->operations[0x2C] = { 1, [](CPU* cpu)
	{
		cpu->registers.l++;
		cpu->registers.f = (cpu->registers.l ? 0 : FLAG_ZERO);
	} };

	// DEC L
	this->operations[0x2D] = { 1, [](CPU* cpu)
	{
		cpu->registers.l--;
		cpu->registers.f = cpu->registers.l ? 0 : FLAG_ZERO;
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

	// INC SP
	this->operations[0x33] = { 1, [](CPU* cpu)
	{
		cpu->registers.sp++;
	} };

	// INC A
	this->operations[0x3C] = { 1, [](CPU* cpu)
	{
		cpu->registers.a++;
		cpu->registers.f = (cpu->registers.a ? 0 : FLAG_ZERO);
	} };

	// DEC A
	this->operations[0x3D] = { 1, [](CPU* cpu)
	{
		cpu->registers.a--;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// LD A,n
	this->operations[0x3E] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->readProgramByte();
	} };

	// LD B,A
	this->operations[0x47] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.a;
	} };

	// LD C,B
	this->operations[0x48] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.b;
	} };

	// LD C,C
	this->operations[0x49] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.c;
	} };

	// LD C,D
	this->operations[0x4A] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.d;
	} };

	// LD C,E
	this->operations[0x4B] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.e;
	} };

	// LD C,H
	this->operations[0x4C] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.h;
	} };

	// LD C,L
	this->operations[0x4D] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.l;
	} };
	
	// LD C,A
	this->operations[0x4F] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.a;
	} };

	// LD D,A
	this->operations[0x57] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.a;
	} };

	// LD E,B
	this->operations[0x58] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.b;
	} };

	// LD E,C
	this->operations[0x59] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.c;
	} };

	// LD E,D
	this->operations[0x5A] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.d;
	} };

	// LD E,E
	this->operations[0x5B] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.e;
	} };

	// LD E,H
	this->operations[0x5C] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.h;
	} };

	// LD E,L
	this->operations[0x5D] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.l;
	} };

	// LD E,A
	this->operations[0x5F] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.a;
	} };

	// LD H,A
	this->operations[0x67] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.a;
	} };

	// LD L,B
	this->operations[0x68] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.b;
	} };

	// LD L,C
	this->operations[0x69] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.c;
	} };

	// LD L,D
	this->operations[0x6A] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.d;
	} };

	// LD L,E
	this->operations[0x6B] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.e;
	} };

	// LD L,H
	this->operations[0x6C] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.h;
	} };

	// LD L,L
	this->operations[0x6D] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.l;
	} };

	// LD L,A
	this->operations[0x6F] = { 1, [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.a;
	} };

	// LD(HL), A
	this->operations[0x77] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
	} };

	// LD A,E
	this->operations[0x7B] = { 1, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.e;
	} };

	// LD A,H
	this->operations[0x7C] = { 1, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.h;
	} };

	// LD A,L
	this->operations[0x7D] = { 1, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.l;
	} };

	// LD A,A
	this->operations[0x7F] = { 1, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.a;
	} };

	// XOR A
	this->operations[0xAF] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.a;
		cpu->registers.a &= 0xFF;

		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// POP BC
	this->operations[0xC1] = { 3, [](CPU* cpu)
	{
		cpu->registers.bc = cpu->stackPopWord();
	} };

	// PUSH BC
	this->operations[0xC5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.bc);
	} };

	// RET
	this->operations[0xC9] = { 3, [](CPU* cpu)
	{
		cpu->registers.pc = cpu->stackPopWord();
	} };

	// Ext ops (callbacks)
	this->operations[0xCB] = { 0, [](CPU* cpu)
	{
		cpu->executeCallback(cpu->readProgramByte());
	} };
	
	// CALL nn
	this->operations[0xCD] = { 5, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.pc + 2);
		cpu->registers.pc = cpu->readProgramWord();
	} };

	// POP DE
	this->operations[0xD1] = { 3, [](CPU* cpu)
	{
		cpu->registers.de = cpu->stackPopWord();
	} };

	// PUSH DE
	this->operations[0xD5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.de);
	} };

	// LDH (n),A
	this->operations[0xE0] = { 3, [](CPU* cpu)
	{
		cpu->mmu->writeByte(0xFF00 | cpu->readProgramByte(), cpu->registers.a);
	} };

	// POP HL
	this->operations[0xE1] = { 3, [](CPU* cpu)
	{
		cpu->registers.hl = cpu->stackPopWord();
	} };

	// LDH (C),A
	this->operations[0xE2] = { 2, [](CPU* cpu)
	{
		cpu->mmu->writeByte(0xFF00 | cpu->registers.c, cpu->registers.a);
	} };

	// PUSH HL
	this->operations[0xE5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.hl);
	} };

	// LD (nn),A
	this->operations[0xEA] = { 4, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->readProgramWord(), cpu->registers.a);
	} };

	// POP AF
	this->operations[0xF1] = { 3, [](CPU* cpu)
	{
		cpu->registers.af = cpu->stackPopWord();
	} };

	// PUSH AF
	this->operations[0xF5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.af);
	} };

	// CP n
	this->operations[0xFE] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->readProgramByte();
		char regA = cpu->registers.a;

		if(regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };
}

void CPU::setupCallbacks()
{
	this->callbacks[0x11] = { 2, [](CPU* cpu)
	{
		unsigned char ci = cpu->registers.f & FLAG_CARRY ? 1 : 0;
		unsigned char co = cpu->registers.c & FLAG_ZERO ? FLAG_CARRY : 0;

		cpu->registers.c = (cpu->registers.c << 1) + ci;
		cpu->registers.c &= 255;
		cpu->registers.f = (cpu->registers.c) ? 0 : FLAG_ZERO;
		cpu->registers.f = (cpu->registers.f & 0xEF) + co;
	} };

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
	unsigned short low = this->readProgramByte();
	unsigned short high = this->readProgramByte();
	return low | (high << 8);
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

unsigned short CPU::stackPopWord()
{
	unsigned short value = this->mmu->readWord(this->registers.sp);
	this->registers.sp += 2;
	return value;
}

unsigned char CPU::stackPopByte()
{
	unsigned char value = this->mmu->readByte(this->registers.sp);
	this->registers.sp += 1;
	return value;
}

void CPU::loadProgram(std::string data)
{
	this->mmu->loadRom(std::basic_string<unsigned char>(data.begin(), data.end()));
}

bool CPU::execute()
{
	unsigned short pc = this->registers.pc;
	unsigned char instruction = this->readProgramByte();

	auto operation = &this->operations[instruction];

	if(operation->func)
	{
		try
		{
			operation->func(this);
			this->registers.m += operation->ticks;

			printf("Operation %X (%X) executed\n", instruction, pc);

			if (!this->gpu->working()) return false;
			//this->gpu->frame();

			if(this->registers.pc == 0x100) this->mmu->markBiosPass();
			return true;
		}
		catch(std::exception e)
		{
			printf("Operation %X (%X) has thrown an exception: %s\n", instruction, pc, e.what());
		}
		catch(...)
		{
			printf("Operation %X (%X) has thrown an unknown exception\n", instruction, pc);
		}
	}
	else
	{
		printf("Unsupported instruction %X (%X)\n", instruction, pc);
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

			printf("Callback %X (%X) executed\n", instruction, this->registers.pc);
			return;
		}
		catch (std::exception e)
		{
			printf("Callback %X (%X) has thrown an exception: %s\n", instruction, this->registers.pc, e.what());
		}
		catch (...)
		{
			printf("Callback %X (%X) has thrown an exception\n", instruction, this->registers.pc);
		}
	}
	else
	{
		printf("Unsupported callback %X (%X)\n", instruction, this->registers.pc);
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
