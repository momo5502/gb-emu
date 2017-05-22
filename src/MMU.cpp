#include "STDInclude.hpp"

unsigned char MMU::Bios[256] =
{
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

MMU::MMU() : iF(0), iE(0), passedBios(false), cpu(nullptr)
{
	ZeroObject(this->vram);
	ZeroObject(this->eram);
	ZeroObject(this->wram);
	ZeroObject(this->zram);

	ZeroObject(this->oam);
}

void MMU::connectCPU(CPU* _cpu)
{
	this->cpu = _cpu;
}

void MMU::loadRom(std::basic_string<unsigned char> data)
{
	this->rom = data;
}

unsigned char MMU::readByte(unsigned short address)
{
	if (auto mem = this->getMemoryPtr(address))
	{
		return *mem;
	}

	throw std::runtime_error("Nullptr dereferenced!");
}

unsigned short MMU::readWord(unsigned short address)
{
	unsigned short low = this->readByte(address);
	unsigned short high = this->readByte(address + 1);
	return low | (high << 8);
}

void MMU::writeByte(unsigned short address, unsigned char value)
{
	if (address < 0x8000) return;

	if(auto mem = this->getMemoryPtr(address))
	{
		*mem = value;

		if(mem >= this->vram && mem < &this->vram[sizeof this->vram])
		{
			this->cpu->getGPU()->updateTile(address);
		}
		else if (static_cast<void*>(mem) == &this->cpu->timer.div)
		{
			this->cpu->timer.div = 0;
		}
		else if (static_cast<void*>(mem) == &this->cpu->timer.tac)
		{
			this->cpu->timer.tac &= 7;
		}
	}
	else throw std::runtime_error("Nullptr dereferenced!");
}

void MMU::writeWord(unsigned short address, unsigned short value)
{
	this->writeByte(address, value & 0xFF);
	this->writeByte(address + 1, value >> 8);
}

#pragma optimize("", off)
void MMU::markBiosPass()
{
	this->passedBios = true;
}
#pragma optimize("", on)

unsigned char* MMU::getMemoryPtr(unsigned short address)
{
	switch(address & 0xF000)
	{
		// BIOS / ROM0
		case 0x0000:
		{
			if(!this->passedBios)
			{
				if(address < 0x100)
				{
					return &MMU::Bios[address];
				}
			}

			if (address >= this->rom.size()) throw std::runtime_error("Rom not loaded!");
			return &const_cast<unsigned char*>(this->rom.data())[address];
		}

		// ROM0
		case 0x1000:
		case 0x2000:
		case 0x3000:
		// ROM1
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
		{
			if (address >= this->rom.size()) throw std::runtime_error("Rom not loaded!");
			return &const_cast<unsigned char*>(this->rom.data())[address];
		}

		// VRAM
		case 0x8000:
		case 0x9000:
		{
			return &this->vram[address & 0x1FFF];
		}

		// ERAM
		case 0xA000:
		case 0xB000:
		{
			return &this->eram[address & 0x1FFF];
		}

		// WRAM
		case 0xC000:
		case 0xD000:
		case 0xE000: // Shadow
		{
			return &this->wram[address & 0x1FFF];
		}

		case 0xF000:
		{
			unsigned short lhAddr = address & 0x0F00;
			if(lhAddr == 0x0F00) // Zero page
			{
				if (address == 0xFFFF) return &this->iE;

				if (address >= 0xFF80)
				{
					return &this->zram[address & 0x7F];
				}
				else // TODO: Implement
				{
					if(address & 0x30)
					{
						this->zero[0] = 0;
						this->zero[1] = 0;
						return &this->zero[0];
					}
					else if((address & 0xF0) >= 0x40 && (address & 0xF0) <= 0x70)
					{
						return this->cpu->getGPU()->getMemoryPtr(address);
					}

					// Joypad
					if (address >= 0xFF00 && address <= 0xFF03)
					{
						this->zero[0] = 0;
						this->zero[1] = 0;
						return &this->zero[0];
					}

					// Timer
					if (address == 0xFF04) return reinterpret_cast<unsigned char*>(&this->cpu->timer.div);
					if (address == 0xFF05) return reinterpret_cast<unsigned char*>(&this->cpu->timer.tima);
					if (address == 0xFF06) return reinterpret_cast<unsigned char*>(&this->cpu->timer.tma);
					if (address == 0xFF07) return reinterpret_cast<unsigned char*>(&this->cpu->timer.tac);

					// Others
					if(address >= 0xFF08 && address <= 0xFF0E)
					{
						this->zero[0] = 0;
						this->zero[1] = 0;
						return &this->zero[0];
					}

					if (address == 0xFF0F)
					{
						return &this->iF;
					}

					throw std::runtime_error(Utils::VA("Not implemented (%X)!", address));
					/*
					this->zero[0] = 0;
					this->zero[1] = 0;
					return &this->zero[0];
					*/
				}
			}
			else if(lhAddr == 0x0E00) // OAM
			{
				if (address < 0xFEA0)
				{
					return &this->oam[address & 0xFF]; // GPU memory
				}
				else
				{
					this->zero[0] = 0;
					this->zero[1] = 0;
					return &this->zero[0];
				}
			}
			else
			{
				return &this->wram[address & 0x1FFF];
			}
		}

		default:
		{
			return nullptr;
		}
	}
}

MMU::~MMU()
{
	
}
