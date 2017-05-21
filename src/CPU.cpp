#include "STDInclude.hpp"

CPU::CPU(std::unique_ptr<MMU> _mmu, std::unique_ptr<GPU> _gpu) : ime(true)
{
	ZeroObject(this->registers);
	ZeroObject(this->operations);
	ZeroObject(this->callbacks);

	this->mmu = std::move(_mmu);
	this->gpu = std::move(_gpu);

	this->mmu->connectCPU(this);
	this->gpu->connectCPU(this);

	this->verifyComponents();

	this->setupOperations();
	this->setupCallbacks();

	int implOp = 0;
	int implCb = 0;
	for(int i = 0; i <= 0x100; ++i)
	{
		if (this->operations[i].func) implOp++;
		if (this->callbacks[i].func) implCb++;
	}

	printf("Operation coverage: %d/256\n", implOp);
	printf("Callback coverage: %d/256\n", implCb);
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

	// INC B
	this->operations[0x04] = { 1, [](CPU* cpu)
	{
		cpu->registers.b++;
		cpu->registers.f = (cpu->registers.b ? 0 : FLAG_ZERO);
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

	// RLC A
	this->operations[0x07] = { 1, [](CPU* cpu)
	{
		unsigned char ci = cpu->registers.a & FLAG_ZERO ? 1 : 0;
		unsigned char co = cpu->registers.a & FLAG_ZERO ? FLAG_CARRY : 0;

		cpu->registers.a = (cpu->registers.a << 1) + ci;
		cpu->registers.a &= 255;
		cpu->registers.a = (cpu->registers.f & 0xEF) + co;
	} };
	// ADD HL,BC
	this->operations[0x09] = { 3, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		short value = cpu->registers.bc;
		unsigned int result = cpu->registers.hl + value;

		if (result & 0xFFFF0000) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned char>(result & 0xFFFF);

		if (!cpu->registers.hl) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// LD A,(BC)
	this->operations[0x0A] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.bc);
	} };

	// DEC BC
	this->operations[0x0B] = { 1, [](CPU* cpu)
	{
		cpu->registers.bc--;
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

	// INC D
	this->operations[0x14] = { 1, [](CPU* cpu)
	{
		cpu->registers.d++;
		cpu->registers.f = (cpu->registers.d ? 0 : FLAG_ZERO);
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

	// JR n
	this->operations[0x18] = { 2, [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();

		cpu->registers.pc += jumpLoc;
		cpu->registers.m++;
	} };

	// ADD HL,DE
	this->operations[0x19] = { 3, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		short value = cpu->registers.de;
		unsigned int result = cpu->registers.hl + value;

		if (result & 0xFFFF0000) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned char>(result & 0xFFFF);

		if (!cpu->registers.hl) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// LD A,(DE)
	this->operations[0x1A] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.de);
	} };

	// DEC DE
	this->operations[0x1B] = { 1, [](CPU* cpu)
	{
		cpu->registers.de--;
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
			cpu->registers.l++;
		}
	} };

	// INC HL
	this->operations[0x23] = { 1, [](CPU* cpu)
	{
		cpu->registers.hl++;
	} };

	// INC H
	this->operations[0x24] = { 1, [](CPU* cpu)
	{
		cpu->registers.h++;
		cpu->registers.f = (cpu->registers.h ? 0 : FLAG_ZERO);
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

	// ADD HL,HL
	this->operations[0x29] = { 3, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		short value = cpu->registers.hl;
		unsigned int result = cpu->registers.hl + value;

		if (result & 0xFFFF0000) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned char>(result & 0xFFFF);

		if (!cpu->registers.hl) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// LDI A,(HL)
	this->operations[0x2A] = { 2, [](CPU* cpu)
	{

		cpu->registers.a = cpu->mmu->readByte(cpu->registers.hl);
		cpu->registers.l = (cpu->registers.l + 1) & 0xFF;

		if (!cpu->registers.l)
		{
			cpu->registers.h++;
		}
	} };

	// DEC HL
	this->operations[0x2B] = { 1, [](CPU* cpu)
	{
		cpu->registers.hl--;
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

	// LD (HL),n
	this->operations[0x36] = { 3, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->readProgramByte());
	} };

	// ADD HL,SP
	this->operations[0x39] = { 3, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		short value = cpu->registers.sp;
		unsigned int result = cpu->registers.hl + value;

		if (result & 0xFFFF0000) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned char>(result & 0xFFFF);

		if (!cpu->registers.hl) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// DEC SP
	this->operations[0x3B] = { 1, [](CPU* cpu)
	{
		cpu->registers.sp--;
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

	// LD B,B
	this->operations[0x40] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.b;
	} };

	// LD B,C
	this->operations[0x41] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.c;
	} };

	// LD B,D
	this->operations[0x42] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.d;
	} };

	// LD B,E
	this->operations[0x43] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.e;
	} };

	// LD B,H
	this->operations[0x44] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.h;
	} };

	// LD B,L
	this->operations[0x45] = { 1, [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.l;
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

	// LD C,(HL)
	this->operations[0x4E] = { 2, [](CPU* cpu)
	{
		cpu->registers.c = cpu->mmu->readByte(cpu->registers.hl);
	} };
	
	// LD C,A
	this->operations[0x4F] = { 1, [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.a;
	} };

	// LD D,B
	this->operations[0x50] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.b;
	} };

	// LD D,C
	this->operations[0x51] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.c;
	} };

	// LD D,D
	this->operations[0x52] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.d;
	} };

	// LD D,E
	this->operations[0x53] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.e;
	} };

	// LD D,H
	this->operations[0x54] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.h;
	} };

	// LD D,L
	this->operations[0x55] = { 1, [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.l;
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

	// LD E,(HL)
	this->operations[0x5E] = { 2, [](CPU* cpu)
	{
		cpu->registers.e = cpu->mmu->readByte(cpu->registers.hl);
	} };

	// LD E,A
	this->operations[0x5F] = { 1, [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.a;
	} };

	// LD H,B
	this->operations[0x60] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.b;
	} };

	// LD H,C
	this->operations[0x61] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.c;
	} };

	// LD H,D
	this->operations[0x62] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.d;
	} };

	// LD H,E
	this->operations[0x63] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.e;
	} };

	// LD H,H
	this->operations[0x64] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.h;
	} };

	// LD H,L
	this->operations[0x65] = { 1, [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.l;
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

	// LD L,(HL)
	this->operations[0x6E] = { 2, [](CPU* cpu)
	{
		cpu->registers.l = cpu->mmu->readByte(cpu->registers.hl);
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

	// LD A,B
	this->operations[0x78] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.b;
	} };

	// LD A,C
	this->operations[0x79] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.c;
	} };

	// LD A,D
	this->operations[0x7A] = { 1, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.d;
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

	// LD A,(HL)
	this->operations[0x7E] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.hl);
	} };

	// LD A,A
	this->operations[0x7F] = { 1, [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.a;
	} };

	// ADD A,B
	this->operations[0x80] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.b;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,C
	this->operations[0x81] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.c;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,D
	this->operations[0x82] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.d;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,E
	this->operations[0x83] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.e;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,H
	this->operations[0x84] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.h;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,L
	this->operations[0x85] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.l;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,(HL)
	this->operations[0x86] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->mmu->readByte(cpu->registers.hl);
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// ADD A,B
	this->operations[0x87] = { 2, [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NIBBLE;

		char value = cpu->registers.b;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// SUB A,B
	this->operations[0x90] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.b;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// SUB A,C
	this->operations[0x91] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.c;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// SUB A,D
	this->operations[0x92] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.d;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// SUB A,E
	this->operations[0x93] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.e;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// SUB A,H
	this->operations[0x94] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.h;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// SUB A,L
	this->operations[0x95] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.l;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// SUB A,A
	this->operations[0x97] = { 1, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;

		char value = cpu->registers.a;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	} };

	// AND B
	this->operations[0xA0] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// AND C
	this->operations[0xA1] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.c;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// AND D
	this->operations[0xA2] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.d;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// AND E
	this->operations[0xA3] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.e;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// AND H
	this->operations[0xA4] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.h;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// AND L
	this->operations[0xA5] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.l;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// AND A
	this->operations[0xA7] = { 1, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.a;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR B
	this->operations[0xA8] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR C
	this->operations[0xA9] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.c;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR D
	this->operations[0xAA] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.d;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR E
	this->operations[0xAB] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.e;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR H
	this->operations[0xAC] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.h;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR L
	this->operations[0xAD] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.l;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// XOR A
	this->operations[0xAF] = { 1, [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.a;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR B
	this->operations[0xB0] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR C
	this->operations[0xB1] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR D
	this->operations[0xB2] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.d;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR E
	this->operations[0xB3] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.e;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR H
	this->operations[0xB4] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.h;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR L
	this->operations[0xB5] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.l;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// OR A
	this->operations[0xB7] = { 1, [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.a;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	} };

	// CP B
	this->operations[0xB8] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.b;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP C
	this->operations[0xB9] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.c;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP D
	this->operations[0xBA] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.d;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP E
	this->operations[0xBB] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.e;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP H
	this->operations[0xBC] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.h;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP L
	this->operations[0xBD] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.l;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP (HL)
	this->operations[0xBE] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->mmu->readByte(cpu->registers.hl);
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// CP A
	this->operations[0xBF] = { 2, [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NIBBLE;
		char value = cpu->registers.a;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	} };

	// RET NZ
	this->operations[0xC0] = { 2, [](CPU* cpu)
	{
		if (!(cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = cpu->stackPopWord();
			cpu->registers.m += 1;
		}
	} };

	// POP BC
	this->operations[0xC1] = { 3, [](CPU* cpu)
	{
		cpu->registers.bc = cpu->stackPopWord();
	} };

	// JP NZ,nn
	this->operations[0xC2] = { 3, [](CPU* cpu)
	{
		unsigned short jumpLoc = cpu->readProgramWord();
		if (!(cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = jumpLoc;
			cpu->registers.m += 1;
		}
	} };

	// JP nn
	this->operations[0xC3] = { 3, [](CPU* cpu)
	{
		cpu->registers.pc = cpu->readProgramWord();
	} };

	// PUSH BC
	this->operations[0xC5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.bc);
	} };

	// RET Z
	this->operations[0xC8] = { 2, [](CPU* cpu)
	{
		if ((cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = cpu->stackPopWord();
			cpu->registers.m += 1;
		}
	} };

	// RET
	this->operations[0xC9] = { 3, [](CPU* cpu)
	{
		cpu->registers.pc = cpu->stackPopWord();
	} };

	// JP Z,nn
	this->operations[0xCA] = { 3, [](CPU* cpu)
	{
		unsigned short jumpLoc = cpu->readProgramWord();
		if ((cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = jumpLoc;
			cpu->registers.m += 1;
		}
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

	// JP NC,nn
	this->operations[0xD2] = { 3, [](CPU* cpu)
	{
		unsigned short jumpLoc = cpu->readProgramWord();
		if (!(cpu->registers.f & FLAG_CARRY))
		{
			cpu->registers.pc = jumpLoc;
			cpu->registers.m += 1;
		}
	} };

	// PUSH DE
	this->operations[0xD5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.de);
	} };

	// RETI
	this->operations[0xD9] = { 3, [](CPU* cpu)
	{
		cpu->registers.a = cpu->savRegisters.a;
		cpu->registers.b = cpu->savRegisters.b;
		cpu->registers.c = cpu->savRegisters.c;
		cpu->registers.d = cpu->savRegisters.d;
		cpu->registers.e = cpu->savRegisters.e;
		cpu->registers.f = cpu->savRegisters.f;
		cpu->registers.h = cpu->savRegisters.h;
		cpu->registers.l = cpu->savRegisters.l;

		cpu->ime = true;
		cpu->registers.pc = cpu->stackPopWord();
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

	// AND n
	this->operations[0xE6] = { 2, [](CPU* cpu)
	{
		cpu->registers.a &= cpu->readProgramByte();

		cpu->registers.f &= ~(FLAG_CARRY | FLAG_NIBBLE);
		cpu->registers.f |= FLAG_HALF_CARRY;

		if(!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else  cpu->registers.f &= ~FLAG_ZERO;
	} };

	// LD (nn),A
	this->operations[0xEA] = { 4, [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->readProgramWord(), cpu->registers.a);
	} };

	// LDH A,(n)
	this->operations[0xF0] = { 3, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(0xFF00 | cpu->readProgramByte());
	} };

	// POP AF
	this->operations[0xF1] = { 3, [](CPU* cpu)
	{
		cpu->registers.af = cpu->stackPopWord();
	} };

	// LDH A,(C)
	this->operations[0xF2] = { 2, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(0xFF00 | cpu->registers.c);
	} };

	// DI
	this->operations[0xF3] = { 1, [](CPU* cpu)
	{
		cpu->ime = false;
	} };

	// PUSH AF
	this->operations[0xF5] = { 3, [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.af);
	} };

	// LD A,(nn)
	this->operations[0xFA] = { 4, [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->readProgramWord());
	} };

	// EI
	this->operations[0xFB] = { 1, [](CPU* cpu)
	{
		cpu->ime = true;
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

	// RST 38
	this->operations[0xFF] = { 0, [](CPU* cpu)
	{
		cpu->executeRst(0x38);
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
	
	this->callbacks[0x87] = { 2, [](CPU* cpu)
	{
		cpu->registers.a &= 0xFE;
	} };

	this->callbacks[0xCF] = { 2, [](CPU* cpu)
	{
		cpu->registers.a |= 1 << 1;
	} };
}

void CPU::executeRst(unsigned short num)
{
	std::memcpy(&this->savRegisters, &this->registers, sizeof this->registers);
	this->stackPushWord(this->registers.pc);
	this->registers.pc = num;
	this->registers.m += 3;
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

bool CPU::runFrame()
{
	unsigned int endTick = this->registers.m + 17556;

	auto start = std::chrono::high_resolution_clock::now();

	while(this->registers.m < endTick)
	{
		if (!this->execute()) return false;
		//std::this_thread::sleep_for(1ms);
	}

	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

	if(delta < (15ms))
	{
		std::this_thread::sleep_for((15ms) - delta);
	}

	return true;
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

#ifdef DEBUG
			printf("Operation %X (%X) executed\n", instruction, pc);
#endif

			this->timer.increment(this);

			if (this->ime && this->mmu->iE && this->mmu->iF)
			{
				this->ime = false;

				unsigned char ifired = this->mmu->iE & this->mmu->iF;
				if (ifired & 1)
				{
					this->mmu->iF &= 0xFE;
					this->executeRst(0x40);
				}
				else if (ifired & 2)
				{
					this->mmu->iF &= 0xFD;
					this->executeRst(0x48);
				}
				else if (ifired & 4)
				{
					this->mmu->iF &= 0xFB;
					this->executeRst(0x50);
				}
				else if (ifired & 8)
				{
					this->mmu->iF &= 0xF7;
					this->executeRst(0x58);
				}
				else if (ifired & 16)
				{
					this->mmu->iF &= 0xEF;
					this->executeRst(0x60);
				}
				else
				{
					this->ime = true;
				}
			}

			this->timer.increment(this);

			if (!this->gpu->working()) return false;
			this->gpu->frame();

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

#ifdef DEBUG
			printf("Callback %X (%X) executed\n", instruction, this->registers.pc);
#endif
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
