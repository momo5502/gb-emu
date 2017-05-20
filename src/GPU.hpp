#pragma once

class CPU;

class GPU
{
public:
	GPU();
	~GPU();

	void connectCPU(CPU* _cpu);

	void frame();
	bool working();

	unsigned char* getMemoryPtr(unsigned short address);
	void updateTile(unsigned short address);

private:
	struct Memory
	{
		unsigned short flags;
		unsigned char yscrl;
		unsigned char xscrl;
		unsigned char curline;
		unsigned char raster;
		unsigned char unk;
		unsigned char palette[3];

		unsigned char reg[0xFF];
	};

	enum GBColor : unsigned char
	{
		GBC_WHITE = 0,
		GBC_LIGHT_GRAY = 1,
		GBC_DARK_GRAY = 2,
		GBC_BLACK = 3,
	};

	enum Mode
	{
		MODE_HBLANK = 0,
		MODE_VBLANK = 1,
		MODE_OAM = 2,
		MODE_VRAM = 3
	};

	struct GBCPixelQuad
	{
		GBColor _1 : 2;
		GBColor _2 : 2;
		GBColor _3 : 2;
		GBColor _4 : 2;
	};

	enum Flags
	{
		FLAG_BACKGROUND_ON = (1 << 0),
		FLAG_SPRITES_ON = (1 << 1),
		FLAG_SPRITES_SIZE = (1 << 2),
		FLAG_ALT_TILE_MAP = (1 << 3),
		FLAG_ALT_TILE_SET = (1 << 4),
		FLAG_WINDOW_ON = (1 << 5),
		FLAG_ALT_WINDOW_TILE_MAP = (1 << 6),
		FLAG_DISPLAY_ON = (1 << 7)
	};

	struct D3DTLVERTEX
	{
		float x, y, z, rhw;
		FLOAT tu, tv;
	};

	HWND window;
	IDirect3D9Ex* d3d9;
	IDirect3DDevice9Ex* device;
	IDirect3DTexture9* screenTexture;

	CPU* cpu;
	Mode mode;
	Memory mem;
	unsigned int clock;

	DWORD screenBuffer[160 * 144];
	unsigned char tiles[512][8][8];

	DWORD getColorFromPalette(unsigned int palette, unsigned int index);

	static DWORD GetGBColor(GBColor pixel);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void renderScreen();
	void renderTexture();
};
