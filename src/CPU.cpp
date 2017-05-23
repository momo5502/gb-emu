#include "STDInclude.hpp"

const unsigned char CPU::OperationTicks[0x100] =
{
	2, 6, 4, 4, 2, 2, 4, 4, 10, 4, 4, 4, 2, 2, 4, 4, // 0x0_
	2, 6, 4, 4, 2, 2, 4, 4,  4, 4, 4, 4, 2, 2, 4, 4, // 0x1_
	0, 6, 4, 4, 2, 2, 4, 2,  0, 4, 4, 4, 2, 2, 4, 2, // 0x2_
	4, 6, 4, 4, 6, 6, 6, 2,  0, 4, 4, 4, 2, 2, 4, 2, // 0x3_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x4_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x5_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x6_
	4, 4, 4, 4, 4, 4, 2, 4,  2, 2, 2, 2, 2, 2, 4, 2, // 0x7_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x8_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x9_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0xa_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0xb_
	0, 6, 0, 6, 0, 8, 4, 8,  0, 2, 0, 0, 0, 6, 4, /*8*/ 0, // 0xc_
	0, 6, 0, 0, 0, 8, 4, 8,  0, 8, 0, 0, 0, 0, 4, /*8*/ 0, // 0xd_
	6, 6, 4, 0, 0, 8, 4, 8,  8, 2, 8, 0, 0, 0, 4, /*8*/ 0, // 0xe_
	6, 6, 4, 2, 0, 8, 4, 8,  6, 4, 8, 2, 0, 0, 4, /*8*/ 0  // 0xf_
};

const unsigned char CPU::CallbackTicks[0x100] =
{
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x0_
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x1_
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x2_
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x3_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x4_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x5_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x6_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x7_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x8_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x9_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xa_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xb_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xc_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xd_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xe_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8  // 0xf_
};

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
	for(int i = 0; i <= min(ARRAYSIZE(this->operations), ARRAYSIZE(this->callbacks)); ++i)
	{
		if (this->operations[i]) implOp++;
		if (this->callbacks[i]) implCb++;
	}

	printf("Operation coverage: %d/%d\n", implOp, ARRAYSIZE(this->operations));
	printf("Callback coverage: %d/%d\n", implCb, ARRAYSIZE(this->callbacks));
}

CPU::~CPU()
{

}

void CPU::setupOperations()
{
	// NOP
	this->operations[0x00] = [](CPU*)
	{
		__nop();
	};

	// LD BC,nn
	this->operations[0x01] = [](CPU* cpu)
	{
		cpu->registers.bc = cpu->readProgramWord();
	};

	// LD (BC),A
	this->operations[0x02] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.bc, cpu->registers.a);
	};

	// INC BC
	this->operations[0x03] = [](CPU* cpu)
	{
		cpu->registers.bc++;
	};

	// INC B
	this->operations[0x04] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.b);
	};

	// DEC B
	this->operations[0x05] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.b);
	};

	// LD B,n
	this->operations[0x06] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->readProgramByte();
	};

	// RLC A
	this->operations[0x07] = [](CPU* cpu)
	{
		unsigned char ci = cpu->registers.a & FLAG_ZERO ? 1 : 0;
		unsigned char co = cpu->registers.a & FLAG_ZERO ? FLAG_CARRY : 0;

		cpu->registers.a = (cpu->registers.a << 1) + ci;
		cpu->registers.f = (cpu->registers.f & 0xEF) + co;
	};
	// ADD HL,BC
	this->operations[0x09] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		short value = cpu->registers.bc;
		unsigned int result = cpu->registers.hl + value;

		if (result > 0xFFFF) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned short>(result & 0xFFFF);

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// LD A,(BC)
	this->operations[0x0A] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.bc);
	};

	// DEC BC
	this->operations[0x0B] = [](CPU* cpu)
	{
		cpu->registers.bc--;
	};

	// INC C
	this->operations[0x0C] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.c);
	};

	// DEC C
	this->operations[0x0D] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.c);
	};

	// LD C,n
	this->operations[0x0E] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->readProgramByte();
	};

	// LD DE,nn
	this->operations[0x11] = [](CPU* cpu)
	{
		cpu->registers.de = cpu->readProgramWord();
	};

	// LD (DE),A
	this->operations[0x12] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.de, cpu->registers.a);
	};

	// INC DE
	this->operations[0x13] = [](CPU* cpu)
	{
		cpu->registers.de++;
	};

	// INC D
	this->operations[0x14] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.d);
	};

	// DEC D
	this->operations[0x15] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.d);
	};

	// LD D,n
	this->operations[0x16] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->readProgramByte();
	};

	// RL A
	this->operations[0x17] = [](CPU* cpu)
	{
		unsigned char ci = cpu->registers.f & FLAG_CARRY ? 1 : 0;
		unsigned char co = cpu->registers.a & FLAG_ZERO ? FLAG_CARRY : 0;

		cpu->registers.a = (cpu->registers.a << 1) + ci;
		cpu->registers.f = (cpu->registers.f & 0xEF) + co;
	};

	// JR n
	this->operations[0x18] = [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();

		cpu->registers.pc += jumpLoc;
		cpu->registers.m++;
	};

	// ADD HL,DE
	this->operations[0x19] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		short value = cpu->registers.de;
		unsigned int result = cpu->registers.hl + value;

		if (result > 0xFFFF) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned short>(result & 0xFFFF);

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// LD A,(DE)
	this->operations[0x1A] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.de);
	};

	// DEC DE
	this->operations[0x1B] = [](CPU* cpu)
	{
		cpu->registers.de--;
	};

	// INC E
	this->operations[0x1C] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.e);
	};

	// DEC E
	this->operations[0x1D] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.e);
	};

	// LD E,n
	this->operations[0x1E] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->readProgramByte();
	};

	// JR NZ,n
	this->operations[0x20] = [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();

		if (!(cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc += jumpLoc;
			cpu->registers.m++;
		}
	};

	// LD HL,nn
	this->operations[0x21] = [](CPU* cpu)
	{
		cpu->registers.hl = cpu->readProgramWord();
	};

	// LDI (HL),A
	this->operations[0x22] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
		cpu->registers.l = (cpu->registers.l + 1) & 0xFF;

		if (!cpu->registers.l)
		{
			cpu->registers.l++;
		}
	};

	// INC HL
	this->operations[0x23] = [](CPU* cpu)
	{
		cpu->registers.hl++;
	};

	// INC H
	this->operations[0x24] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.h);
	};

	// DEC H
	this->operations[0x25] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.h);
	};

	// LD H,n
	this->operations[0x26] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->readProgramByte();
	};

	// JR n
	this->operations[0x28] = [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();
		cpu->registers.pc += jumpLoc;
		cpu->registers.m++;
	};

	// ADD HL,HL
	this->operations[0x29] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		short value = cpu->registers.hl;
		unsigned int result = cpu->registers.hl + value;

		if (result > 0xFFFF) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned short>(result & 0xFFFF);

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// LDI A,(HL)
	this->operations[0x2A] = [](CPU* cpu)
	{

		cpu->registers.a = cpu->mmu->readByte(cpu->registers.hl);
		cpu->registers.hl++;
	};

	// DEC HL
	this->operations[0x2B] = [](CPU* cpu)
	{
		cpu->registers.hl--;
	};

	// INC L
	this->operations[0x2C] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.l);
	};

	// DEC L
	this->operations[0x2D] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.l);
	};

	// LD L,n
	this->operations[0x2E] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->readProgramByte();
	};

	// JR NC,n
	this->operations[0x30] = [](CPU* cpu)
	{
		char jumpLoc = cpu->readProgramByte();

		if (!(cpu->registers.f & FLAG_CARRY))
		{
			cpu->registers.pc += jumpLoc;
			cpu->registers.m++;
		}
	};

	// LD SP,nn
	this->operations[0x31] = [](CPU* cpu)
	{
		cpu->registers.sp = cpu->readProgramWord();
	};

	// LDD (HL),A
	this->operations[0x32] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
		cpu->registers.hl--;
	};

	// INC SP
	this->operations[0x33] = [](CPU* cpu)
	{
		cpu->registers.sp++;
	};

	// INC A
	this->operations[0x3C] = [](CPU* cpu)
	{
		cpu->inc(&cpu->registers.a);
	};

	// LD (HL),n
	this->operations[0x36] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->readProgramByte());
	};

	// ADD HL,SP
	this->operations[0x39] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		short value = cpu->registers.sp;
		unsigned int result = cpu->registers.hl + value;

		if (result > 0xFFFF) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.hl = static_cast<unsigned short>(result & 0xFFFF);

		if (((cpu->registers.hl & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// DEC SP
	this->operations[0x3B] = [](CPU* cpu)
	{
		cpu->registers.sp--;
	};

	// DEC A
	this->operations[0x3D] = [](CPU* cpu)
	{
		cpu->dec(&cpu->registers.a);
	};

	// LD A,n
	this->operations[0x3E] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->readProgramByte();
	};

	// LD B,B
	this->operations[0x40] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.b;
	};

	// LD B,C
	this->operations[0x41] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.c;
	};

	// LD B,D
	this->operations[0x42] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.d;
	};

	// LD B,E
	this->operations[0x43] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.e;
	};

	// LD B,H
	this->operations[0x44] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.h;
	};

	// LD B,L
	this->operations[0x45] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.l;
	};

	// LD B,A
	this->operations[0x47] = [](CPU* cpu)
	{
		cpu->registers.b = cpu->registers.a;
	};

	// LD C,B
	this->operations[0x48] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.b;
	};

	// LD C,C
	this->operations[0x49] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.c;
	};

	// LD C,D
	this->operations[0x4A] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.d;
	};

	// LD C,E
	this->operations[0x4B] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.e;
	};

	// LD C,H
	this->operations[0x4C] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.h;
	};

	// LD C,L
	this->operations[0x4D] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.l;
	};

	// LD C,(HL)
	this->operations[0x4E] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->mmu->readByte(cpu->registers.hl);
	};
	
	// LD C,A
	this->operations[0x4F] = [](CPU* cpu)
	{
		cpu->registers.c = cpu->registers.a;
	};

	// LD D,B
	this->operations[0x50] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.b;
	};

	// LD D,C
	this->operations[0x51] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.c;
	};

	// LD D,D
	this->operations[0x52] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.d;
	};

	// LD D,E
	this->operations[0x53] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.e;
	};

	// LD D,H
	this->operations[0x54] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.h;
	};

	// LD D,L
	this->operations[0x55] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.l;
	};

	// LD D,(HL)
	this->operations[0x56] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->mmu->readByte(cpu->registers.hl);
	};

	// LD D,A
	this->operations[0x57] = [](CPU* cpu)
	{
		cpu->registers.d = cpu->registers.a;
	};

	// LD E,B
	this->operations[0x58] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.b;
	};

	// LD E,C
	this->operations[0x59] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.c;
	};

	// LD E,D
	this->operations[0x5A] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.d;
	};

	// LD E,E
	this->operations[0x5B] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.e;
	};

	// LD E,H
	this->operations[0x5C] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.h;
	};

	// LD E,L
	this->operations[0x5D] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.l;
	};

	// LD E,(HL)
	this->operations[0x5E] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->mmu->readByte(cpu->registers.hl);
	};

	// LD E,A
	this->operations[0x5F] = [](CPU* cpu)
	{
		cpu->registers.e = cpu->registers.a;
	};

	// LD H,B
	this->operations[0x60] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.b;
	};

	// LD H,C
	this->operations[0x61] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.c;
	};

	// LD H,D
	this->operations[0x62] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.d;
	};

	// LD H,E
	this->operations[0x63] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.e;
	};

	// LD H,H
	this->operations[0x64] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.h;
	};

	// LD H,L
	this->operations[0x65] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.l;
	};

	// LD H,A
	this->operations[0x67] = [](CPU* cpu)
	{
		cpu->registers.h = cpu->registers.a;
	};

	// LD L,B
	this->operations[0x68] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.b;
	};

	// LD L,C
	this->operations[0x69] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.c;
	};

	// LD L,D
	this->operations[0x6A] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.d;
	};

	// LD L,E
	this->operations[0x6B] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.e;
	};

	// LD L,H
	this->operations[0x6C] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.h;
	};

	// LD L,L
	this->operations[0x6D] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.l;
	};

	// LD L,(HL)
	this->operations[0x6E] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->mmu->readByte(cpu->registers.hl);
	};

	// LD L,A
	this->operations[0x6F] = [](CPU* cpu)
	{
		cpu->registers.l = cpu->registers.a;
	};

	// LD(HL), A
	this->operations[0x77] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->registers.hl, cpu->registers.a);
	};

	// LD A,B
	this->operations[0x78] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.b;
	};

	// LD A,C
	this->operations[0x79] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.c;
	};

	// LD A,D
	this->operations[0x7A] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.d;
	};

	// LD A,E
	this->operations[0x7B] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.e;
	};

	// LD A,H
	this->operations[0x7C] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.h;
	};

	// LD A,L
	this->operations[0x7D] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.l;
	};

	// LD A,(HL)
	this->operations[0x7E] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->registers.hl);
	};

	// LD A,A
	this->operations[0x7F] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->registers.a;
	};

	// ADD A,B
	this->operations[0x80] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.b;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,C
	this->operations[0x81] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.c;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,D
	this->operations[0x82] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.d;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,E
	this->operations[0x83] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.e;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,H
	this->operations[0x84] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.h;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,L
	this->operations[0x85] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.l;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,(HL)
	this->operations[0x86] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->mmu->readByte(cpu->registers.hl);
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// ADD A,B
	this->operations[0x87] = [](CPU* cpu)
	{
		cpu->registers.f &= ~FLAG_NEGATIVE;

		char value = cpu->registers.b;
		unsigned int result = cpu->registers.a + value;

		if (result & 0xFF00) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		cpu->registers.a = static_cast<unsigned char>(result & 0xFF);

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (((cpu->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// SUB A,B
	this->operations[0x90] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.b;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// SUB A,C
	this->operations[0x91] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.c;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// SUB A,D
	this->operations[0x92] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.d;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// SUB A,E
	this->operations[0x93] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.e;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// SUB A,H
	this->operations[0x94] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.h;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// SUB A,L
	this->operations[0x95] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.l;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// SUB A,A
	this->operations[0x97] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->registers.a;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// AND B
	this->operations[0xA0] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// AND C
	this->operations[0xA1] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.c;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// AND D
	this->operations[0xA2] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.d;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// AND E
	this->operations[0xA3] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.e;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// AND H
	this->operations[0xA4] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.h;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// AND L
	this->operations[0xA5] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.l;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// AND A
	this->operations[0xA7] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->registers.a;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR B
	this->operations[0xA8] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR C
	this->operations[0xA9] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.c;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR D
	this->operations[0xAA] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.d;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR E
	this->operations[0xAB] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.e;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR H
	this->operations[0xAC] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.h;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR L
	this->operations[0xAD] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.l;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// XOR A
	this->operations[0xAF] = [](CPU* cpu)
	{
		cpu->registers.a ^= cpu->registers.a;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR B
	this->operations[0xB0] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR C
	this->operations[0xB1] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.b;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR D
	this->operations[0xB2] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.d;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR E
	this->operations[0xB3] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.e;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR H
	this->operations[0xB4] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.h;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR L
	this->operations[0xB5] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.l;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// OR A
	this->operations[0xB7] = [](CPU* cpu)
	{
		cpu->registers.a |= cpu->registers.a;
		cpu->registers.f = cpu->registers.a ? 0 : FLAG_ZERO;
	};

	// CP B
	this->operations[0xB8] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.b;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP C
	this->operations[0xB9] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.c;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP D
	this->operations[0xBA] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.d;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP E
	this->operations[0xBB] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.e;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP H
	this->operations[0xBC] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.h;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP L
	this->operations[0xBD] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.l;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP (HL)
	this->operations[0xBE] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->mmu->readByte(cpu->registers.hl);
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// CP A
	this->operations[0xBF] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->registers.a;
		char regA = cpu->registers.a;

		if (regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// RET NZ
	this->operations[0xC0] = [](CPU* cpu)
	{
		if (!(cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = cpu->stackPopWord();
			cpu->registers.m += 1;
		}
	};

	// POP BC
	this->operations[0xC1] = [](CPU* cpu)
	{
		cpu->registers.bc = cpu->stackPopWord();
	};

	// JP NZ,nn
	this->operations[0xC2] = [](CPU* cpu)
	{
		unsigned short jumpLoc = cpu->readProgramWord();
		if (!(cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = jumpLoc;
			cpu->registers.m += 1;
		}
	};

	// JP nn
	this->operations[0xC3] = [](CPU* cpu)
	{
		cpu->registers.pc = cpu->readProgramWord();
	};

	// PUSH BC
	this->operations[0xC5] = [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.bc);
	};

	// RET Z
	this->operations[0xC8] = [](CPU* cpu)
	{
		if ((cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = cpu->stackPopWord();
			cpu->registers.m += 1;
		}
	};

	// RET
	this->operations[0xC9] = [](CPU* cpu)
	{
		cpu->registers.pc = cpu->stackPopWord();
	};

	// JP Z,nn
	this->operations[0xCA] = [](CPU* cpu)
	{
		unsigned short jumpLoc = cpu->readProgramWord();
		if ((cpu->registers.f & FLAG_ZERO))
		{
			cpu->registers.pc = jumpLoc;
			cpu->registers.m += 1;
		}
	};

	// Ext ops (callbacks)
	this->operations[0xCB] = [](CPU* cpu)
	{
		cpu->executeCallback(cpu->readProgramByte());
	};
	
	// CALL nn
	this->operations[0xCD] = [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.pc + 2);
		cpu->registers.pc = cpu->readProgramWord();
	};

	// POP DE
	this->operations[0xD1] = [](CPU* cpu)
	{
		cpu->registers.de = cpu->stackPopWord();
	};

	// JP NC,nn
	this->operations[0xD2] = [](CPU* cpu)
	{
		unsigned short jumpLoc = cpu->readProgramWord();
		if (!(cpu->registers.f & FLAG_CARRY))
		{
			cpu->registers.pc = jumpLoc;
			cpu->registers.m += 1;
		}
	};

	// PUSH DE
	this->operations[0xD5] = [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.de);
	};

	// SUB A,n
	this->operations[0xD6] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;

		char value = cpu->readProgramByte();

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((value & 0x0f) > (cpu->registers.a & 0x0f)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;

		if (!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;
	};

	// RETI
	this->operations[0xD9] = [](CPU* cpu)
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
	};

	// SBC A,n
	this->operations[0xDE] = [](CPU* cpu)
	{
		char value = cpu->readProgramByte();
		value += (cpu->registers.f & FLAG_CARRY) ? 1 : 0;

		cpu->registers.f |= FLAG_NEGATIVE;

		if (value > cpu->registers.a) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if (value == cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if ((value & 0x0F) > (cpu->registers.a & 0x0F))  cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;

		cpu->registers.a -= value;
	};
	// LDH (n),A
	this->operations[0xE0] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(0xFF00 | cpu->readProgramByte(), cpu->registers.a);
	};

	// POP HL
	this->operations[0xE1] = [](CPU* cpu)
	{
		cpu->registers.hl = cpu->stackPopWord();
	};

	// LDH (C),A
	this->operations[0xE2] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(0xFF00 | cpu->registers.c, cpu->registers.a);
	};

	// PUSH HL
	this->operations[0xE5] = [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.hl);
	};

	// AND n
	this->operations[0xE6] = [](CPU* cpu)
	{
		cpu->registers.a &= cpu->readProgramByte();

		cpu->registers.f &= ~(FLAG_CARRY | FLAG_NEGATIVE);
		cpu->registers.f |= FLAG_HALF_CARRY;

		if(!cpu->registers.a) cpu->registers.f |= FLAG_ZERO;
		else  cpu->registers.f &= ~FLAG_ZERO;
	};

	// JP (HL)
	this->operations[0xE9] = [](CPU* cpu)
	{
		cpu->registers.pc = cpu->mmu->readByte(cpu->registers.hl);
	};

	// LD (nn),A
	this->operations[0xEA] = [](CPU* cpu)
	{
		cpu->mmu->writeByte(cpu->readProgramWord(), cpu->registers.a);
	};

	// RST 28
	this->operations[0xEF] = [](CPU* cpu)
	{
		cpu->executeRst(0x28);
	};

	// LDH A,(n)
	this->operations[0xF0] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(0xFF00 | cpu->readProgramByte());
	};

	// POP AF
	this->operations[0xF1] = [](CPU* cpu)
	{
		cpu->registers.af = cpu->stackPopWord();
	};

	// LDH A,(C)
	this->operations[0xF2] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(0xFF00 | cpu->registers.c);
	};

	// DI
	this->operations[0xF3] = [](CPU* cpu)
	{
		cpu->ime = false;
	};

	// PUSH AF
	this->operations[0xF5] = [](CPU* cpu)
	{
		cpu->stackPushWord(cpu->registers.af);
	};

	// LD A,(nn)
	this->operations[0xFA] = [](CPU* cpu)
	{
		cpu->registers.a = cpu->mmu->readByte(cpu->readProgramWord());
	};

	// EI
	this->operations[0xFB] = [](CPU* cpu)
	{
		cpu->ime = true;
	};

	// CP n
	this->operations[0xFE] = [](CPU* cpu)
	{
		cpu->registers.f |= FLAG_NEGATIVE;
		char value = cpu->readProgramByte();
		char regA = cpu->registers.a;

		if(regA == value) cpu->registers.f |= FLAG_ZERO;
		else cpu->registers.f &= ~FLAG_ZERO;

		if (regA < value) cpu->registers.f |= FLAG_CARRY;
		else cpu->registers.f &= ~FLAG_CARRY;

		if ((regA & 0x0F) < (value & 0x0F)) cpu->registers.f |= FLAG_HALF_CARRY;
		else cpu->registers.f &= ~FLAG_HALF_CARRY;
	};

	// RST 38
	this->operations[0xFF] = [](CPU* cpu)
	{
		cpu->executeRst(0x38);
	};
}

void CPU::setupCallbacks()
{
	this->callbacks[0x11] = [](CPU* cpu)
	{
		unsigned char ci = cpu->registers.f & FLAG_CARRY ? 1 : 0;
		unsigned char co = cpu->registers.c & FLAG_ZERO ? FLAG_CARRY : 0;

		cpu->registers.c = (cpu->registers.c << 1) + ci;
		cpu->registers.c &= 255;
		cpu->registers.f = (cpu->registers.c) ? 0 : FLAG_ZERO;
		cpu->registers.f = (cpu->registers.f & 0xEF) + co;
	};

	this->callbacks[0x7C] = [](CPU* cpu)
	{
		cpu->registers.f &= FLAG_HALF_CARRY - 1;
		cpu->registers.f |= FLAG_HALF_CARRY;
		cpu->registers.f = (cpu->registers.c & FLAG_ZERO) ? 0 : FLAG_ZERO;
	};
	
	this->callbacks[0x87] = [](CPU* cpu)
	{
		cpu->registers.a &= 0xFE;
	};

	this->callbacks[0xCF] = [](CPU* cpu)
	{
		cpu->registers.a |= 1 << 1;
	};
}

void CPU::inc(unsigned char* reg)
{
/*
	this->registers.f &= ~FLAG_NEGATIVE;

	if ((*reg & 0x0F) == 0x0F) this->registers.f |= FLAG_HALF_CARRY;
	else this->registers.f &= ~FLAG_HALF_CARRY;

	(*reg)++;
	if (!*reg) this->registers.f |= FLAG_ZERO;
*/

	(*reg)++;
	this->registers.f = ((*reg) ? 0 : FLAG_ZERO);
}

void CPU::dec(unsigned char* reg)
{
/*
	this->registers.f &= ~FLAG_NEGATIVE;

	if ((*reg & 0x0F) == 0x0F) this->registers.f |= FLAG_HALF_CARRY;
	else this->registers.f &= ~FLAG_HALF_CARRY;

	(*reg)--;
	if (!*reg) this->registers.f |= FLAG_ZERO;
*/

	(*reg)--;
	this->registers.f = ((*reg) ? 0 : FLAG_ZERO);
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

	auto operation = this->operations[instruction];

	if(operation)
	{
		try
		{
			operation(this);
			this->registers.m += (CPU::OperationTicks[instruction] / 2);

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
	auto callback = this->callbacks[instruction];

	if (callback)
	{
		try
		{
			callback(this);
			this->registers.m += (CPU::CallbackTicks[instruction] / 4);

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
