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
	0, 6, 0, 6, 0, 8, 4, /*8*/ 0,  0, 2, 0, 0, 0, 6, 4, /*8*/ 0, // 0xc_
	0, 6, 0, 0, 0, 8, 4, /*8*/ 0,  0, 8, 0, 0, 0, 0, 4, /*8*/ 0, // 0xd_
	6, 6, 4, 0, 0, 8, 4, /*8*/ 0,  8, 2, 8, 0, 0, 0, 4, /*8*/ 0, // 0xe_
	6, 6, 4, 2, 0, 8, 4, /*8*/ 0,  6, 4, 8, 2, 0, 0, 4, /*8*/ 0  // 0xf_
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

CPU::CPU(GameBoy* gameBoy) : ime(true), gb(gameBoy)
{
	ZeroObject(this->registers);
	ZeroObject(this->operations);
	ZeroObject(this->callbacks);

	this->setupOperations();
	this->setupCallbacks();

	int implOp = 0;
	int implCb = 0;

	int opSize = ARRAYSIZE(this->operations);
	int cbSize = ARRAYSIZE(this->callbacks);

	for(int i = 0; i < min(opSize, cbSize); ++i)
	{
		if (this->operations[i]) implOp++;
		if (this->callbacks[i]) implCb++;
	}

	printf("Operation coverage: %d/%d\n", implOp, opSize);
	printf("Callback coverage: %d/%d\n", implCb, cbSize);
}

CPU::~CPU()
{

}

void CPU::setupOperations()
{
	// NOP
	this->operations[0x00] = [](GameBoy*) { };

	// LD BC,nn
	this->operations[0x01] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.bc = gb->getCPU()->readProgramWord();
	};

	// LD (BC),A
	this->operations[0x02] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.bc, gb->getCPU()->registers.a);
	};

	// INC BC
	this->operations[0x03] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.bc++;
	};

	// INC B
	this->operations[0x04] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.b);
	};

	// DEC B
	this->operations[0x05] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.b);
	};

	// LD B,n
	this->operations[0x06] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->readProgramByte();
	};

	// RLC A
	this->operations[0x07] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f = (gb->getCPU()->registers.a & 0x80) ? FLAG_CARRY : 0;
		gb->getCPU()->registers.a <<= 1;
		gb->getCPU()->registers.a |= (gb->getCPU()->registers.f & FLAG_CARRY) ? 1 : 0;
	};

	// LD (nn),SP
	this->operations[0x08] = [](GameBoy* gb)
	{
		gb->getMMU()->writeWord(gb->getCPU()->readProgramWord(), gb->getCPU()->registers.sp);
	};

	// ADD HL,BC
	this->operations[0x09] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f &= ~FLAG_ZERO;

		unsigned short value = gb->getCPU()->registers.bc;
		int result = gb->getCPU()->registers.hl + value;

		if (result > 0xFFFF) gb->getCPU()->registers.f |= FLAG_CARRY;

		if (gb->getCPU()->registers.hl ^ value ^ (result & 0xFFFF) & 0x1000) gb->getCPU()->registers.f |= FLAG_HALF_CARRY;

		gb->getCPU()->registers.hl = static_cast<unsigned short>(result & 0xFFFF);
	};

	// LD A,(BC)
	this->operations[0x0A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getMMU()->readByte(gb->getCPU()->registers.bc);
	};

	// DEC BC
	this->operations[0x0B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.bc--;
	};

	// INC C
	this->operations[0x0C] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.c);
	};

	// DEC C
	this->operations[0x0D] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.c);
	};

	// LD C,n
	this->operations[0x0E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->readProgramByte();
	};

	// LD DE,nn
	this->operations[0x11] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.de = gb->getCPU()->readProgramWord();
	};

	// LD (DE),A
	this->operations[0x12] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.de, gb->getCPU()->registers.a);
	};

	// INC DE
	this->operations[0x13] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.de++;
	};

	// INC D
	this->operations[0x14] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.d);
	};

	// DEC D
	this->operations[0x15] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.d);
	};

	// LD D,n
	this->operations[0x16] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->readProgramByte();
	};

	// RL A
	this->operations[0x17] = [](GameBoy* gb)
	{
		unsigned char carry = (gb->getCPU()->registers.f & FLAG_CARRY) ? 1 : 0;
		gb->getCPU()->registers.f = (gb->getCPU()->registers.a & 0x80) ? FLAG_CARRY : 0;
		gb->getCPU()->registers.a <<= 1;
		gb->getCPU()->registers.a |= carry;
	};

	// JR n
	this->operations[0x18] = [](GameBoy* gb)
	{
		char jumpLoc = gb->getCPU()->readProgramByte();
		gb->getCPU()->registers.pc += jumpLoc;
		gb->getCPU()->registers.m++;
	};

	// ADD HL,DE
	this->operations[0x19] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f &= ~FLAG_ZERO;

		unsigned short value = gb->getCPU()->registers.de;
		int result = gb->getCPU()->registers.hl + value;

		if (result > 0xFFFF) gb->getCPU()->registers.f |= FLAG_CARRY;

		if (gb->getCPU()->registers.hl ^ value ^ (result & 0xFFFF) & 0x1000) gb->getCPU()->registers.f |= FLAG_HALF_CARRY;

		gb->getCPU()->registers.hl = static_cast<unsigned short>(result & 0xFFFF);
	};

	// LD A,(DE)
	this->operations[0x1A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getMMU()->readByte(gb->getCPU()->registers.de);
	};

	// DEC DE
	this->operations[0x1B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.de--;
	};

	// INC E
	this->operations[0x1C] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.e);
	};

	// DEC E
	this->operations[0x1D] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.e);
	};

	// LD E,n
	this->operations[0x1E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->readProgramByte();
	};

	// RR a
	this->operations[0x1F] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.a);
	};

	// JR NZ,n
	this->operations[0x20] = [](GameBoy* gb)
	{
		char jumpLoc = gb->getCPU()->readProgramByte();

		if (!(gb->getCPU()->registers.f & FLAG_ZERO))
		{
			gb->getCPU()->registers.pc += jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// LD HL,nn
	this->operations[0x21] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.hl = gb->getCPU()->readProgramWord();
	};

	// LDI (HL),A
	this->operations[0x22] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.a);
		gb->getCPU()->registers.hl++;
	};

	// INC HL
	this->operations[0x23] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.hl++;
	};

	// INC H
	this->operations[0x24] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.h);
	};

	// DEC H
	this->operations[0x25] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.h);
	};

	// LD H,n
	this->operations[0x26] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->readProgramByte();
	};

	// JR Z,n
	this->operations[0x28] = [](GameBoy* gb)
	{
		char jumpLoc = gb->getCPU()->readProgramByte();
		if (gb->getCPU()->registers.f & FLAG_ZERO)
		{
			gb->getCPU()->registers.pc += jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// ADD HL,HL
	this->operations[0x29] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f &= ~FLAG_ZERO;

		unsigned short value = gb->getCPU()->registers.hl;
		int result = gb->getCPU()->registers.hl + value;

		if (result > 0xFFFF) gb->getCPU()->registers.f |= FLAG_CARRY;

		if (gb->getCPU()->registers.hl ^ value ^ (result & 0xFFFF) & 0x1000) gb->getCPU()->registers.f |= FLAG_HALF_CARRY;

		gb->getCPU()->registers.hl = static_cast<unsigned short>(result & 0xFFFF);
	};

	// LDI A,(HL)
	this->operations[0x2A] = [](GameBoy* gb)
	{

		gb->getCPU()->registers.a = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->registers.hl++;
	};

	// DEC HL
	this->operations[0x2B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.hl--;
	};

	// INC L
	this->operations[0x2C] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.l);
	};

	// DEC L
	this->operations[0x2D] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.l);
	};

	// LD L,n
	this->operations[0x2E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->readProgramByte();
	};

	// CPL
	this->operations[0x2F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a ^= 0xFF;
		gb->getCPU()->registers.f |= FLAG_HALF_CARRY | FLAG_NEGATIVE;
	};

	// JR NC,n
	this->operations[0x30] = [](GameBoy* gb)
	{
		char jumpLoc = gb->getCPU()->readProgramByte();
		if (!(gb->getCPU()->registers.f & FLAG_CARRY))
		{
			gb->getCPU()->registers.pc += jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// LD SP,nn
	this->operations[0x31] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.sp = gb->getCPU()->readProgramWord();
	};

	// LDD (HL),A
	this->operations[0x32] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.a);
		gb->getCPU()->registers.hl--;
	};

	// INC SP
	this->operations[0x33] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.sp++;
	};

	// INC (HL)
	this->operations[0x34] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f &= FLAG_CARRY;

		unsigned char value = gb->getMMU()->readByte(gb->getCPU()->registers.hl) + 1;
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, value);

		if (!value) gb->getCPU()->registers.f |= FLAG_ZERO;
		if ((value & 0x0F) == 0x0F) gb->getCPU()->registers.f |= FLAG_HALF_CARRY;
	};

	// DEC (HL)
	this->operations[0x35] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f &= FLAG_CARRY;

		unsigned char value = gb->getMMU()->readByte(gb->getCPU()->registers.hl) - 1;
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, value);

		gb->getCPU()->registers.f |= FLAG_NEGATIVE;
		if(!value) gb->getCPU()->registers.f |= FLAG_ZERO;
		if((value & 0x0F) == 0x0F) gb->getCPU()->registers.f |= FLAG_HALF_CARRY;
	};

	// LD (HL),n
	this->operations[0x36] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->readProgramByte());
	};

	// JR C,n
	this->operations[0x38] = [](GameBoy* gb)
	{
		char jumpLoc = gb->getCPU()->readProgramByte();
		if (gb->getCPU()->registers.f & FLAG_CARRY)
		{
			gb->getCPU()->registers.pc += jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// ADD HL,SP
	this->operations[0x39] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.f &= ~FLAG_ZERO;

		unsigned short value = gb->getCPU()->registers.hl;
		int result = gb->getCPU()->registers.hl + value;

		if (result > 0xFFFF) gb->getCPU()->registers.f |= FLAG_CARRY;

		if (gb->getCPU()->registers.hl ^ value ^ (result & 0xFFFF) & 0x1000) gb->getCPU()->registers.f |= FLAG_HALF_CARRY;

		gb->getCPU()->registers.hl = static_cast<unsigned short>(result & 0xFFFF);
	};

	// DEC SP
	this->operations[0x3B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.sp--;
	};

	// INC A
	this->operations[0x3C] = [](GameBoy* gb)
	{
		gb->getCPU()->inc(&gb->getCPU()->registers.a);
	};

	// DEC A
	this->operations[0x3D] = [](GameBoy* gb)
	{
		gb->getCPU()->dec(&gb->getCPU()->registers.a);
	};

	// LD A,n
	this->operations[0x3E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->readProgramByte();
	};

	// LD B,B
	this->operations[0x40] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.b;
	};

	// LD B,C
	this->operations[0x41] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.c;
	};

	// LD B,D
	this->operations[0x42] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.d;
	};

	// LD B,E
	this->operations[0x43] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.e;
	};

	// LD B,H
	this->operations[0x44] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.h;
	};

	// LD B,L
	this->operations[0x45] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.l;
	};

	// LD B,(HL)
	this->operations[0x46] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD B,A
	this->operations[0x47] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b = gb->getCPU()->registers.a;
	};

	// LD C,B
	this->operations[0x48] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.b;
	};

	// LD C,C
	this->operations[0x49] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.c;
	};

	// LD C,D
	this->operations[0x4A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.d;
	};

	// LD C,E
	this->operations[0x4B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.e;
	};

	// LD C,H
	this->operations[0x4C] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.h;
	};

	// LD C,L
	this->operations[0x4D] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.l;
	};

	// LD C,(HL)
	this->operations[0x4E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};
	
	// LD C,A
	this->operations[0x4F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c = gb->getCPU()->registers.a;
	};

	// LD D,B
	this->operations[0x50] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.b;
	};

	// LD D,C
	this->operations[0x51] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.c;
	};

	// LD D,D
	this->operations[0x52] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.d;
	};

	// LD D,E
	this->operations[0x53] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.e;
	};

	// LD D,H
	this->operations[0x54] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.h;
	};

	// LD D,L
	this->operations[0x55] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.l;
	};

	// LD D,(HL)
	this->operations[0x56] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD D,A
	this->operations[0x57] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d = gb->getCPU()->registers.a;
	};

	// LD E,B
	this->operations[0x58] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.b;
	};

	// LD E,C
	this->operations[0x59] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.c;
	};

	// LD E,D
	this->operations[0x5A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.d;
	};

	// LD E,E
	this->operations[0x5B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.e;
	};

	// LD E,H
	this->operations[0x5C] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.h;
	};

	// LD E,L
	this->operations[0x5D] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.l;
	};

	// LD E,(HL)
	this->operations[0x5E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD E,A
	this->operations[0x5F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e = gb->getCPU()->registers.a;
	};

	// LD H,B
	this->operations[0x60] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.b;
	};

	// LD H,C
	this->operations[0x61] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.c;
	};

	// LD H,D
	this->operations[0x62] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.d;
	};

	// LD H,E
	this->operations[0x63] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.e;
	};

	// LD H,H
	this->operations[0x64] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.h;
	};

	// LD H,L
	this->operations[0x65] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.l;
	};

	// LD H,(HL)
	this->operations[0x66] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD H,A
	this->operations[0x67] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h = gb->getCPU()->registers.a;
	};

	// LD L,B
	this->operations[0x68] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.b;
	};

	// LD L,C
	this->operations[0x69] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.c;
	};

	// LD L,D
	this->operations[0x6A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.d;
	};

	// LD L,E
	this->operations[0x6B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.e;
	};

	// LD L,H
	this->operations[0x6C] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.h;
	};

	// LD L,L
	this->operations[0x6D] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.l;
	};

	// LD L,(HL)
	this->operations[0x6E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD L,A
	this->operations[0x6F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l = gb->getCPU()->registers.a;
	};

	// LD (HL),B
	this->operations[0x70] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.b);
	};

	// LD (HL),C
	this->operations[0x71] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.c);
	};

	// LD (HL),D
	this->operations[0x72] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.d);
	};

	// LD (HL),E
	this->operations[0x73] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.e);
	};

	// LD (HL),H
	this->operations[0x74] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.h);
	};

	// LD (HL),L
	this->operations[0x75] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.l);
	};

	// HALT
	this->operations[0x76] = [](GameBoy*)
	{
		throw std::runtime_error("Halt not implemented!");
	};

	// LD(HL), A
	this->operations[0x77] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getCPU()->registers.a);
	};

	// LD A,B
	this->operations[0x78] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.b;
	};

	// LD A,C
	this->operations[0x79] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.c;
	};

	// LD A,D
	this->operations[0x7A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.d;
	};

	// LD A,E
	this->operations[0x7B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.e;
	};

	// LD A,H
	this->operations[0x7C] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.h;
	};

	// LD A,L
	this->operations[0x7D] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.l;
	};

	// LD A,(HL)
	this->operations[0x7E] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD A,A
	this->operations[0x7F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->registers.a;
	};

	// ADD A,B
	this->operations[0x80] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.b);
	};

	// ADD A,C
	this->operations[0x81] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.c);
	};

	// ADD A,D
	this->operations[0x82] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.d);
	};

	// ADD A,E
	this->operations[0x83] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.e);
	};

	// ADD A,H
	this->operations[0x84] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.h);
	};

	// ADD A,L
	this->operations[0x85] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.l);
	};

	// ADD A,(HL)
	this->operations[0x86] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// ADD A,A
	this->operations[0x87] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->registers.a);
	};

	// ADC A,B
	this->operations[0x88] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.b);
	};

	// ADC A,C
	this->operations[0x89] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.c);
	};

	// ADC A,D
	this->operations[0x8A] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.d);
	};

	// ADC A,E
	this->operations[0x8B] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.e);
	};

	// ADC A,H
	this->operations[0x8C] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.h);
	};

	// ADC A,L
	this->operations[0x8D] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.l);
	};

	// ADC A,(HL)
	this->operations[0x8E] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// ADC A,A
	this->operations[0x8F] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->registers.a);
	};

	// SUB A,B
	this->operations[0x90] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.b);
	};

	// SUB A,C
	this->operations[0x91] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.c);
	};

	// SUB A,D
	this->operations[0x92] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.d);
	};

	// SUB A,E
	this->operations[0x93] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.e);
	};

	// SUB A,H
	this->operations[0x94] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.h);
	};

	// SUB A,L
	this->operations[0x95] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.l);
	};

	// SUB A,(HL)
	this->operations[0x96] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// SUB A,A
	this->operations[0x97] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->registers.a);
	};

	// AND B
	this->operations[0xA0] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.b);
	};

	// AND C
	this->operations[0xA1] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.c);
	};

	// AND D
	this->operations[0xA2] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.d);
	};

	// AND E
	this->operations[0xA3] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.e);
	};

	// AND H
	this->operations[0xA4] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.h);
	};

	// AND L
	this->operations[0xA5] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.h);
	};

	// AND A,(HL)
	this->operations[0xA6] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// AND A
	this->operations[0xA7] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->registers.a);
	};

	// XOR B
	this->operations[0xA8] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.b);
	};

	// XOR C
	this->operations[0xA9] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.c);
	};

	// XOR D
	this->operations[0xAA] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.d);
	};

	// XOR E
	this->operations[0xAB] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.e);
	};

	// XOR H
	this->operations[0xAC] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.h);
	};

	// XOR L
	this->operations[0xAD] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.l);
	};

	// XOR (HL)
	this->operations[0xAE] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// XOR A
	this->operations[0xAF] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->registers.a);
	};

	// OR B
	this->operations[0xB0] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.b);
	};

	// OR C
	this->operations[0xB1] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.c);
	};

	// OR D
	this->operations[0xB2] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.d);
	};

	// OR E
	this->operations[0xB3] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.e);
	};

	// OR H
	this->operations[0xB4] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.h);
	};

	// OR L
	this->operations[0xB5] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.l);
	};

	// OR (HL)
	this->operations[0xB6] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// OR A
	this->operations[0xB7] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->registers.a);
	};

	// CP B
	this->operations[0xB8] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.b);
	};

	// CP C
	this->operations[0xB9] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.c);
	};

	// CP D
	this->operations[0xBA] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.d);
	};

	// CP E
	this->operations[0xBB] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.e);
	};

	// CP H
	this->operations[0xBC] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.h);
	};

	// CP L
	this->operations[0xBD] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.l);
	};

	// CP (HL)
	this->operations[0xBE] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getMMU()->readByte(gb->getCPU()->registers.hl));
	};

	// CP A
	this->operations[0xBF] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->registers.a);
	};

	// RET NZ
	this->operations[0xC0] = [](GameBoy* gb)
	{
		if (!(gb->getCPU()->registers.f & FLAG_ZERO))
		{
			gb->getCPU()->registers.pc = gb->getCPU()->stackPopWord();
			gb->getCPU()->registers.m++;
		}
	};

	// POP BC
	this->operations[0xC1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.bc = gb->getCPU()->stackPopWord();
	};

	// JP NZ,nn
	this->operations[0xC2] = [](GameBoy* gb)
	{
		unsigned short jumpLoc = gb->getCPU()->readProgramWord();
		if (!(gb->getCPU()->registers.f & FLAG_ZERO))
		{
			gb->getCPU()->registers.pc = jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// JP nn
	this->operations[0xC3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.pc = gb->getCPU()->readProgramWord();
	};

	// CALL NZ,nn
	this->operations[0xC4] = [](GameBoy* gb)
	{
		unsigned short callLoc = gb->getCPU()->readProgramWord();
		if (!(gb->getCPU()->registers.f & FLAG_ZERO))
		{
			gb->getCPU()->stackPushWord(gb->getCPU()->registers.pc);
			gb->getCPU()->registers.pc = callLoc;
		}
	};

	// PUSH BC
	this->operations[0xC5] = [](GameBoy* gb)
	{
		gb->getCPU()->stackPushWord(gb->getCPU()->registers.bc);
	};

	// ADD A,n
	this->operations[0xC6] = [](GameBoy* gb)
	{
		gb->getCPU()->add(gb->getCPU()->readProgramByte());
	};

	// RST 0
	this->operations[0xC7] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x00);
	};

	// RET Z
	this->operations[0xC8] = [](GameBoy* gb)
	{
		if (gb->getCPU()->registers.f & FLAG_ZERO)
		{
			gb->getCPU()->registers.pc = gb->getCPU()->stackPopWord();
			gb->getCPU()->registers.m++;
		}
	};

	// RET
	this->operations[0xC9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.pc = gb->getCPU()->stackPopWord();
	};

	// JP Z,nn
	this->operations[0xCA] = [](GameBoy* gb)
	{
		unsigned short jumpLoc = gb->getCPU()->readProgramWord();
		if (gb->getCPU()->registers.f & FLAG_ZERO)
		{
			gb->getCPU()->registers.pc = jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// Ext ops (callbacks)
	this->operations[0xCB] = [](GameBoy* gb)
	{
		gb->getCPU()->executeCallback(gb->getCPU()->readProgramByte());
	};

	// CALL Z,nn
	this->operations[0xCC] = [](GameBoy* gb)
	{
		unsigned short callLoc = gb->getCPU()->readProgramWord();
		if (gb->getCPU()->registers.f & FLAG_ZERO)
		{
			gb->getCPU()->stackPushWord(gb->getCPU()->registers.pc);
			gb->getCPU()->registers.pc = callLoc;
		}
	};
	
	// CALL nn
	this->operations[0xCD] = [](GameBoy* gb)
	{
		gb->getCPU()->stackPushWord(gb->getCPU()->registers.pc + 2);
		gb->getCPU()->registers.pc = gb->getCPU()->readProgramWord();
	};

	// ADC A,n
	this->operations[0xCE] = [](GameBoy* gb)
	{
		gb->getCPU()->adc(gb->getCPU()->readProgramByte());
	};

	// RST 8
	this->operations[0xCF] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x08);
	};

	// RET NC
	this->operations[0xD0] = [](GameBoy* gb)
	{
		if (!(gb->getCPU()->registers.f & FLAG_CARRY))
		{
			gb->getCPU()->registers.pc = gb->getCPU()->stackPopWord();
			gb->getCPU()->registers.m++;
		}
	};

	// POP DE
	this->operations[0xD1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.de = gb->getCPU()->stackPopWord();
	};

	// JP NC,nn
	this->operations[0xD2] = [](GameBoy* gb)
	{
		unsigned short jumpLoc = gb->getCPU()->readProgramWord();
		if (!(gb->getCPU()->registers.f & FLAG_CARRY))
		{
			gb->getCPU()->registers.pc = jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// CALL NC,nn
	this->operations[0xD4] = [](GameBoy* gb)
	{
		unsigned short callLoc = gb->getCPU()->readProgramWord();
		if (!(gb->getCPU()->registers.f & FLAG_CARRY))
		{
			gb->getCPU()->stackPushWord(gb->getCPU()->registers.pc);
			gb->getCPU()->registers.pc = callLoc;
		}
	};

	// PUSH DE
	this->operations[0xD5] = [](GameBoy* gb)
	{
		gb->getCPU()->stackPushWord(gb->getCPU()->registers.de);
	};

	// SUB A,n
	this->operations[0xD6] = [](GameBoy* gb)
	{
		gb->getCPU()->sub(gb->getCPU()->readProgramByte());
	};

	// RST 10
	this->operations[0xD7] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x10);
	};

	// RET C
	this->operations[0xD8] = [](GameBoy* gb)
	{
		if (gb->getCPU()->registers.f & FLAG_CARRY)
		{
			gb->getCPU()->registers.pc = gb->getCPU()->stackPopWord();
			gb->getCPU()->registers.m++;
		}
	};

	// RETI
	this->operations[0xD9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getCPU()->savRegisters.a;
		gb->getCPU()->registers.b = gb->getCPU()->savRegisters.b;
		gb->getCPU()->registers.c = gb->getCPU()->savRegisters.c;
		gb->getCPU()->registers.d = gb->getCPU()->savRegisters.d;
		gb->getCPU()->registers.e = gb->getCPU()->savRegisters.e;
		gb->getCPU()->registers.f = gb->getCPU()->savRegisters.f;
		gb->getCPU()->registers.h = gb->getCPU()->savRegisters.h;
		gb->getCPU()->registers.l = gb->getCPU()->savRegisters.l;

		gb->getCPU()->ime = true;
		gb->getCPU()->registers.pc = gb->getCPU()->stackPopWord();
	};

	// JP C,nn
	this->operations[0xDA] = [](GameBoy* gb)
	{
		unsigned short jumpLoc = gb->getCPU()->readProgramWord();
		if (gb->getCPU()->registers.f & FLAG_CARRY)
		{
			gb->getCPU()->registers.pc = jumpLoc;
			gb->getCPU()->registers.m++;
		}
	};

	// CALL C,nn
	this->operations[0xDC] = [](GameBoy* gb)
	{
		unsigned short callLoc = gb->getCPU()->readProgramWord();
		if (gb->getCPU()->registers.f & FLAG_CARRY)
		{
			gb->getCPU()->stackPushWord(gb->getCPU()->registers.pc);
			gb->getCPU()->registers.pc = callLoc;
		}
	};

	// SBC A,n
	this->operations[0xDE] = [](GameBoy* gb)
	{
		gb->getCPU()->sbc(gb->getCPU()->readProgramByte());
	};

	// RST 18
	this->operations[0xDF] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x18);
	};

	// LDH (n),A
	this->operations[0xE0] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(0xFF00 | gb->getCPU()->readProgramByte(), gb->getCPU()->registers.a);
	};

	// POP HL
	this->operations[0xE1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.hl = gb->getCPU()->stackPopWord();
	};

	// LDH (C),A
	this->operations[0xE2] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(0xFF00 | gb->getCPU()->registers.c, gb->getCPU()->registers.a);
	};

	// PUSH HL
	this->operations[0xE5] = [](GameBoy* gb)
	{
		gb->getCPU()->stackPushWord(gb->getCPU()->registers.hl);
	};

	// AND n
	this->operations[0xE6] = [](GameBoy* gb)
	{
		gb->getCPU()->and(gb->getCPU()->readProgramByte());
	};

	// RST 20
	this->operations[0xE7] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x20);
	};

	// JP (HL)
	this->operations[0xE9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.pc = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
	};

	// LD (nn),A
	this->operations[0xEA] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->readProgramWord(), gb->getCPU()->registers.a);
	};

	// XOR n
	this->operations[0xEE] = [](GameBoy* gb)
	{
		gb->getCPU()->xor(gb->getCPU()->readProgramByte());
	};

	// RST 28
	this->operations[0xEF] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x28);
	};

	// LDH A,(n)
	this->operations[0xF0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getMMU()->readByte(0xFF00 | gb->getCPU()->readProgramByte());
	};

	// POP AF
	this->operations[0xF1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.af = gb->getCPU()->stackPopWord();
	};

	// LDH A,(C)
	this->operations[0xF2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getMMU()->readByte(0xFF00 | gb->getCPU()->registers.c);
	};

	// DI
	this->operations[0xF3] = [](GameBoy* gb)
	{
		gb->getCPU()->ime = false;
	};

	// PUSH AF
	this->operations[0xF5] = [](GameBoy* gb)
	{
		gb->getCPU()->stackPushWord(gb->getCPU()->registers.af);
	};

	// OR n
	this->operations[0xF6] = [](GameBoy* gb)
	{
		gb->getCPU()->or(gb->getCPU()->readProgramByte());
	};

	// RST 30
	this->operations[0xF7] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x30);
	};

	// LD A,(nn)
	this->operations[0xFA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a = gb->getMMU()->readByte(gb->getCPU()->readProgramWord());
	};

	// EI
	this->operations[0xFB] = [](GameBoy* gb)
	{
		gb->getCPU()->ime = true;
	};

	// CP n
	this->operations[0xFE] = [](GameBoy* gb)
	{
		gb->getCPU()->cp(gb->getCPU()->readProgramByte());
	};

	// RST 38
	this->operations[0xFF] = [](GameBoy* gb)
	{
		gb->getCPU()->executeRst(0x38);
	};
}

void CPU::setupCallbacks()
{
	this->callbacks[0x00] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x01] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x02] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x03] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x04] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x05] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x06] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->rlc(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x07] = [](GameBoy* gb)
	{
		gb->getCPU()->rlc(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x08] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x09] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x0A] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x0B] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x0C] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x0D] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x0E] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->rrc(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x0F] = [](GameBoy* gb)
	{
		gb->getCPU()->rrc(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x10] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x11] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x12] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x13] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x14] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x15] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x16] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->rl(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x17] = [](GameBoy* gb)
	{
		gb->getCPU()->rl(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x18] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x19] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x1A] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x1B] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x1C] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x1D] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x1E] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->rr(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x1F] = [](GameBoy* gb)
	{
		gb->getCPU()->rr(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x20] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x21] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x22] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x23] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x24] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x25] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x26] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->sla(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x27] = [](GameBoy* gb)
	{
		gb->getCPU()->sla(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x28] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x29] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x2A] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x2B] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x2C] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x2D] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x2E] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->sra(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x2F] = [](GameBoy* gb)
	{
		gb->getCPU()->sra(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x30] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x31] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x32] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x33] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x34] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x35] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x36] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->swap(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x37] = [](GameBoy* gb)
	{
		gb->getCPU()->swap(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x38] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.b);
	};

	this->callbacks[0x39] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.c);
	};

	this->callbacks[0x3A] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.d);
	};

	this->callbacks[0x3B] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.e);
	};

	this->callbacks[0x3C] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.h);
	};

	this->callbacks[0x3D] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.l);
	};

	this->callbacks[0x3E] = [](GameBoy* gb)
	{
		unsigned char hlVal = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->getCPU()->srl(&hlVal);
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, hlVal);
	};

	this->callbacks[0x3F] = [](GameBoy* gb)
	{
		gb->getCPU()->srl(&gb->getCPU()->registers.a);
	};

	this->callbacks[0x40] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 0);
	};

	this->callbacks[0x41] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 0);
	};

	this->callbacks[0x42] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 0);
	};

	this->callbacks[0x43] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 0);
	};

	this->callbacks[0x44] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 0);
	};

	this->callbacks[0x45] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 0);
	};

	this->callbacks[0x46] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 0);
	};

	this->callbacks[0x47] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 0);
	};

	this->callbacks[0x48] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 1);
	};

	this->callbacks[0x49] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 1);
	};

	this->callbacks[0x4A] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 1);
	};

	this->callbacks[0x4B] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 1);
	};

	this->callbacks[0x4C] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 1);
	};

	this->callbacks[0x4D] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 1);
	};

	this->callbacks[0x4E] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 1);
	};

	this->callbacks[0x4F] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 1);
	};

	this->callbacks[0x50] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 2);
	};

	this->callbacks[0x51] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 2);
	};

	this->callbacks[0x52] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 2);
	};

	this->callbacks[0x53] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 2);
	};

	this->callbacks[0x54] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 2);
	};

	this->callbacks[0x55] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 2);
	};

	this->callbacks[0x56] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 2);
	};

	this->callbacks[0x57] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 2);
	};

	this->callbacks[0x58] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 3);
	};

	this->callbacks[0x59] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 3);
	};

	this->callbacks[0x5A] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 3);
	};

	this->callbacks[0x5B] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 3);
	};

	this->callbacks[0x5C] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 3);
	};

	this->callbacks[0x5D] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 3);
	};

	this->callbacks[0x5E] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 3);
	};

	this->callbacks[0x5F] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 3);
	};

	this->callbacks[0x60] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 4);
	};

	this->callbacks[0x61] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 4);
	};

	this->callbacks[0x62] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 4);
	};

	this->callbacks[0x63] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 4);
	};

	this->callbacks[0x64] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 4);
	};

	this->callbacks[0x65] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 4);
	};

	this->callbacks[0x66] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 4);
	};

	this->callbacks[0x67] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 4);
	};

	this->callbacks[0x68] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 5);
	};

	this->callbacks[0x69] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 5);
	};

	this->callbacks[0x6A] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 5);
	};

	this->callbacks[0x6B] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 5);
	};

	this->callbacks[0x6C] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 5);
	};

	this->callbacks[0x6D] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 5);
	};

	this->callbacks[0x6E] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 5);
	};

	this->callbacks[0x6F] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 5);
	};

	this->callbacks[0x70] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 6);
	};

	this->callbacks[0x71] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 6);
	};

	this->callbacks[0x72] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 6);
	};

	this->callbacks[0x73] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 6);
	};

	this->callbacks[0x74] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 6);
	};

	this->callbacks[0x75] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 6);
	};

	this->callbacks[0x76] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 6);
	};

	this->callbacks[0x77] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 6);
	};

	this->callbacks[0x78] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.b, 7);
	};

	this->callbacks[0x79] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.c, 7);
	};

	this->callbacks[0x7A] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.d, 7);
	};

	this->callbacks[0x7B] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.e, 7);
	};

	this->callbacks[0x7C] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.h, 7);
	};

	this->callbacks[0x7D] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.l, 7);
	};

	this->callbacks[0x7E] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getMMU()->readByte(gb->getCPU()->registers.hl), 7);
	};

	this->callbacks[0x7F] = [](GameBoy* gb)
	{
		gb->getCPU()->bit(gb->getCPU()->registers.a, 7);
	};

	this->callbacks[0x80] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 0);
	};

	this->callbacks[0x81] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 0);
	};

	this->callbacks[0x82] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 0);
	};

	this->callbacks[0x83] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 0);
	};

	this->callbacks[0x84] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 0);
	};

	this->callbacks[0x85] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 0);
	};

	this->callbacks[0x86] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 0));
	};

	this->callbacks[0x87] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 0);
	};

	this->callbacks[0x88] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 1);
	};

	this->callbacks[0x89] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 1);
	};

	this->callbacks[0x8A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 1);
	};

	this->callbacks[0x8B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 1);
	};

	this->callbacks[0x8C] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 1);
	};

	this->callbacks[0x8D] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 1);
	};

	this->callbacks[0x8E] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 1));
	};

	this->callbacks[0x8F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 1);
	};

	this->callbacks[0x90] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 2);
	};

	this->callbacks[0x91] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 2);
	};

	this->callbacks[0x92] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 2);
	};

	this->callbacks[0x93] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 2);
	};

	this->callbacks[0x94] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 2);
	};

	this->callbacks[0x95] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 2);
	};

	this->callbacks[0x96] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 2));
	};

	this->callbacks[0x97] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 2);
	};

	this->callbacks[0x98] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 3);
	};

	this->callbacks[0x99] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 3);
	};

	this->callbacks[0x9A] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 3);
	};

	this->callbacks[0x9B] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 3);
	};

	this->callbacks[0x9C] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 3);
	};

	this->callbacks[0x9D] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 3);
	};

	this->callbacks[0x9E] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 3));
	};

	this->callbacks[0x9F] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 3);
	};

	this->callbacks[0xA0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 4);
	};

	this->callbacks[0xA1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 4);
	};

	this->callbacks[0xA2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 4);
	};

	this->callbacks[0xA3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 4);
	};

	this->callbacks[0xA4] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 4);
	};

	this->callbacks[0xA5] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 4);
	};

	this->callbacks[0xA6] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 4));
	};

	this->callbacks[0xA7] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 4);
	};

	this->callbacks[0xA8] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 5);
	};

	this->callbacks[0xA9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 5);
	};

	this->callbacks[0xAA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 5);
	};

	this->callbacks[0xAB] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 5);
	};

	this->callbacks[0xAC] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 5);
	};

	this->callbacks[0xAD] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 5);
	};

	this->callbacks[0xAE] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 5));
	};

	this->callbacks[0xAF] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 5);
	};

	this->callbacks[0xB0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 6);
	};

	this->callbacks[0xB1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 6);
	};

	this->callbacks[0xB2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 6);
	};

	this->callbacks[0xB3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 6);
	};

	this->callbacks[0xB4] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 6);
	};

	this->callbacks[0xB5] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 6);
	};

	this->callbacks[0xB6] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 6));
	};

	this->callbacks[0xB7] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 6);
	};

	this->callbacks[0xB8] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b &= ~(1 << 7);
	};

	this->callbacks[0xB9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c &= ~(1 << 7);
	};

	this->callbacks[0xBA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d &= ~(1 << 7);
	};

	this->callbacks[0xBB] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e &= ~(1 << 7);
	};

	this->callbacks[0xBC] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h &= ~(1 << 7);
	};

	this->callbacks[0xBD] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l &= ~(1 << 7);
	};

	this->callbacks[0xBE] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) & ~(1 << 7));
	};

	this->callbacks[0xBF] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a &= ~(1 << 7);
	};

	this->callbacks[0xC0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 0;
	};

	this->callbacks[0xC1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 0;
	};

	this->callbacks[0xC2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 0;
	};

	this->callbacks[0xC3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 0;
	};

	this->callbacks[0xC4] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 0;
	};

	this->callbacks[0xC5] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 0;
	};

	this->callbacks[0xC6] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 0));
	};

	this->callbacks[0xC7] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 0;
	};

	this->callbacks[0xC8] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 1;
	};

	this->callbacks[0xC9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 1;
	};

	this->callbacks[0xCA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 1;
	};

	this->callbacks[0xCB] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 1;
	};

	this->callbacks[0xCC] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 1;
	};

	this->callbacks[0xCD] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 1;
	};

	this->callbacks[0xCE] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 1));
	};

	this->callbacks[0xCF] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 1;
	};

	this->callbacks[0xD0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 2;
	};

	this->callbacks[0xD1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 2;
	};

	this->callbacks[0xD2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 2;
	};

	this->callbacks[0xD3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 2;
	};

	this->callbacks[0xD4] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 2;
	};

	this->callbacks[0xD5] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 2;
	};

	this->callbacks[0xD6] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 2));
	};

	this->callbacks[0xD7] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 2;
	};

	this->callbacks[0xD8] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 3;
	};

	this->callbacks[0xD9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 3;
	};

	this->callbacks[0xDA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 3;
	};

	this->callbacks[0xDB] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 3;
	};

	this->callbacks[0xDC] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 3;
	};

	this->callbacks[0xDD] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 3;
	};

	this->callbacks[0xDE] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 3));
	};

	this->callbacks[0xDF] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 3;
	};

	this->callbacks[0xE0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 4;
	};

	this->callbacks[0xE1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 4;
	};

	this->callbacks[0xE2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 4;
	};

	this->callbacks[0xE3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 4;
	};

	this->callbacks[0xE4] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 4;
	};

	this->callbacks[0xE5] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 4;
	};

	this->callbacks[0xE6] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 4));
	};

	this->callbacks[0xE7] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 4;
	};

	this->callbacks[0xE8] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 5;
	};

	this->callbacks[0xE9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 5;
	};

	this->callbacks[0xEA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 5;
	};

	this->callbacks[0xEB] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 5;
	};

	this->callbacks[0xEC] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 5;
	};

	this->callbacks[0xED] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 5;
	};

	this->callbacks[0xEE] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 5));
	};

	this->callbacks[0xEF] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 5;
	};

	this->callbacks[0xF0] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 6;
	};

	this->callbacks[0xF1] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 6;
	};

	this->callbacks[0xF2] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 6;
	};

	this->callbacks[0xF3] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 6;
	};

	this->callbacks[0xF4] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 6;
	};

	this->callbacks[0xF5] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 6;
	};

	this->callbacks[0xF6] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 6));
	};

	this->callbacks[0xF7] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 6;
	};

	this->callbacks[0xF8] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.b |= 1 << 7;
	};

	this->callbacks[0xF9] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.c |= 1 << 7;
	};

	this->callbacks[0xFA] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.d |= 1 << 7;
	};

	this->callbacks[0xFB] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.e |= 1 << 7;
	};

	this->callbacks[0xFC] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.h |= 1 << 7;
	};

	this->callbacks[0xFD] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.l |= 1 << 7;
	};

	this->callbacks[0xFE] = [](GameBoy* gb)
	{
		gb->getMMU()->writeByte(gb->getCPU()->registers.hl, gb->getMMU()->readByte(gb->getCPU()->registers.hl) | (1 << 7));
	};

	this->callbacks[0xFF] = [](GameBoy* gb)
	{
		gb->getCPU()->registers.a |= 1 << 7;
	};
}

void CPU::inc(unsigned char* reg)
{
	(*reg)++;
	this->registers.f = (this->registers.f & FLAG_CARRY);
	if ((*reg & 0x0F) == 0x0F) this->registers.f |= FLAG_HALF_CARRY;
	if (!*reg) this->registers.f |= FLAG_ZERO;
}

void CPU::dec(unsigned char* reg)
{
	(*reg)--;
	this->registers.f = (this->registers.f & FLAG_CARRY) | FLAG_NEGATIVE;
	if ((*reg & 0x0F) == 0x00) this->registers.f |= FLAG_HALF_CARRY;
	if (!*reg) this->registers.f |= FLAG_ZERO;
}

void CPU::add(unsigned char reg)
{
	this->registers.f &= ~FLAG_NEGATIVE;

	char value = reg;
	unsigned int result = this->registers.a + value;

	if (result > 0xFF) this->registers.f |= FLAG_CARRY;
	else this->registers.f &= ~FLAG_CARRY;

	this->registers.a = static_cast<unsigned char>(result & 0xFF);

	if (!this->registers.a) this->registers.f |= FLAG_ZERO;
	else this->registers.f &= ~FLAG_ZERO;

	if (((this->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) this->registers.f |= FLAG_HALF_CARRY;
	else this->registers.f &= ~FLAG_HALF_CARRY;
}

void CPU::adc(unsigned char reg)
{
	this->registers.f |= FLAG_NEGATIVE;
	reg += (this->registers.f & FLAG_CARRY) ? 1 : 0;

	int value = reg;
	int result = this->registers.a + value;

	if (result > 0xFF) this->registers.f |= FLAG_CARRY;
	else this->registers.f &= ~FLAG_CARRY;

	if (this->registers.a == value) this->registers.f |= FLAG_ZERO;
	else this->registers.f &= ~FLAG_ZERO;

	if (((this->registers.a & 0x0F) + (value & 0x0F)) > 0x0F) this->registers.f |= FLAG_HALF_CARRY;
	else this->registers.f &= ~FLAG_HALF_CARRY;

	this->registers.a = static_cast<unsigned char>(result & 0xFF);
}

void CPU::sbc(unsigned char reg)
{
	int carry = (this->registers.f & FLAG_CARRY) ? 1 : 0;
	int result = this->registers.a - reg - carry;

	this->registers.f = FLAG_NEGATIVE;
	if(!(result & 0xFF)) this->registers.f |= FLAG_ZERO;

	if (result < 0) this->registers.f |= FLAG_CARRY;
	if (((this->registers.a & 0x0F) - (reg & 0x0F) - carry) < 0) this->registers.f |= FLAG_HALF_CARRY;

	this->registers.a = static_cast<unsigned char>(result);
}

void CPU::sub(unsigned char reg)
{
	this->registers.f |= FLAG_NEGATIVE;

	char value = reg;

	if (value > this->registers.a) this->registers.f |= FLAG_CARRY;
	else this->registers.f &= ~FLAG_CARRY;

	if ((value & 0x0F) > (this->registers.a & 0x0F)) this->registers.f |= FLAG_HALF_CARRY;
	else this->registers.f &= ~FLAG_HALF_CARRY;

	this->registers.a -= value;

	if (!this->registers.a) this->registers.f |= FLAG_ZERO;
	else this->registers.f &= ~FLAG_ZERO;
}

void CPU::and(unsigned char reg)
{
	this->registers.f = FLAG_HALF_CARRY;

	this->registers.a &= reg;
	if (!this->registers.a) this->registers.f |= FLAG_ZERO;
}

void CPU::xor(unsigned char reg)
{
	this->registers.a ^= reg;
	this->registers.f = this->registers.a ? 0 : FLAG_ZERO;
}

void CPU::or(unsigned char reg)
{
	this->registers.a |= reg;
	this->registers.f = this->registers.a ? 0 : FLAG_ZERO;
}

void CPU::cp(unsigned char reg)
{
	this->registers.f |= FLAG_NEGATIVE;
	char value = reg;
	char regA = this->registers.a;

	if (regA == value) this->registers.f |= FLAG_ZERO;
	else this->registers.f &= ~FLAG_ZERO;

	if (regA < value) this->registers.f |= FLAG_CARRY;
	else this->registers.f &= ~FLAG_CARRY;

	if ((regA & 0x0F) < (value & 0x0F)) this->registers.f |= FLAG_HALF_CARRY;
	else this->registers.f &= ~FLAG_HALF_CARRY;
}

void CPU::bit(unsigned char reg, unsigned char _bit)
{
	gb->getCPU()->registers.f &= ~(FLAG_ZERO | FLAG_NEGATIVE);
	gb->getCPU()->registers.f |= FLAG_HALF_CARRY;
	if (reg & (1 << _bit)) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::rlc(unsigned char* reg)
{
	bool carry = (*reg & 0x80) == 0x80;
	*reg <<= 1;
	*reg |= carry ? 0x01 : 0;

	gb->getCPU()->registers.f = carry ? FLAG_CARRY : 0;
	if (!*reg && reg != &this->registers.a) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::rrc(unsigned char* reg)
{
	bool carry = *reg & 0x01;
	*reg >>= 1;
	*reg |= carry ? 0x80 : 0;

	gb->getCPU()->registers.f = carry ? FLAG_CARRY : 0;
	if (!*reg && reg != &this->registers.a) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::rl(unsigned char* reg)
{
	unsigned char carry = (this->registers.f & FLAG_CARRY) == FLAG_CARRY;
	gb->getCPU()->registers.f = (*reg & 0x80) ? FLAG_CARRY : 0;

	*reg <<= 1;
	*reg |= carry;

	if (!*reg && reg != &this->registers.a) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::rr(unsigned char* reg)
{
	unsigned char carry = this->registers.f & FLAG_CARRY;
	gb->getCPU()->registers.f = (*reg & 0x01) ? FLAG_CARRY : 0;

	*reg >>= 1;
	*reg |= carry;

	if (!*reg && reg != &this->registers.a) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::sla(unsigned char* reg)
{
	gb->getCPU()->registers.f = (*reg & 0x80) ? FLAG_CARRY : 0;
	*reg <<= 1;
	if (!*reg) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::sra(unsigned char* reg)
{
	unsigned char carry = (*reg & 0x80);
	gb->getCPU()->registers.f = (*reg & 0x01) ? FLAG_CARRY : 0;
	*reg >>= 1;
	*reg |= carry;
	if (!*reg) gb->getCPU()->registers.f |= FLAG_ZERO;
}

void CPU::swap(unsigned char* reg)
{
	*reg = ((*reg & 0x0F) << 4) | ((*reg >> 4) & 0x0F);
	gb->getCPU()->registers.f = !*reg ? FLAG_ZERO : 0;
}

void CPU::srl(unsigned char* reg)
{
	gb->getCPU()->registers.f = (*reg & 0x01) ? FLAG_CARRY : 0;
	*reg >>= 1;
	if (!*reg) gb->getCPU()->registers.f |= FLAG_ZERO;
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
	return this->gb->getMMU()->readByte(addr);
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
	this->gb->getMMU()->writeWord(this->registers.sp, value);
}

void CPU::stackPushByte(unsigned char value)
{
	this->registers.sp -= 1;
	this->gb->getMMU()->writeByte(this->registers.sp, value);
}

unsigned short CPU::stackPopWord()
{
	unsigned short value = this->gb->getMMU()->readWord(this->registers.sp);
	this->registers.sp += 2;
	return value;
}

unsigned char CPU::stackPopByte()
{
	unsigned char value = this->gb->getMMU()->readByte(this->registers.sp);
	this->registers.sp += 1;
	return value;
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
			operation(this->gb);
			this->registers.m += (CPU::OperationTicks[instruction] / 2);

			this->timer.increment(this->gb);

			if (this->ime && this->gb->getMMU()->iE && this->gb->getMMU()->iF)
			{
				this->ime = false;

				unsigned char ifired = this->gb->getMMU()->iE & this->gb->getMMU()->iF;
				if (ifired & 1)
				{
					this->gb->getMMU()->iF &= 0xFE;
					this->executeRst(0x40);
				}
				else if (ifired & 2)
				{
					this->gb->getMMU()->iF &= 0xFD;
					this->executeRst(0x48);
				}
				else if (ifired & 4)
				{
					this->gb->getMMU()->iF &= 0xFB;
					this->executeRst(0x50);
				}
				else if (ifired & 8)
				{
					this->gb->getMMU()->iF &= 0xF7;
					this->executeRst(0x58);
				}
				else if (ifired & 16)
				{
					this->gb->getMMU()->iF &= 0xEF;
					this->executeRst(0x60);
				}
				else
				{
					this->ime = true;
				}
			}

			this->timer.increment(this->gb);

			if (!this->gb->getGPU()->working()) return false;
			this->gb->getGPU()->frame();

			if(this->registers.pc == 0x100) this->gb->getMMU()->markBiosPass();
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
			callback(this->gb);
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
