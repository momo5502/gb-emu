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

private:
	struct Memory
	{
		unsigned short flags;
		unsigned char yscrl;
		unsigned char xscrl;
		unsigned char curline;
		unsigned char raster;

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
		HBLANK = 0,
		VBLANK = 1,
		OAM = 2,
		VRAM = 3
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
		BACKGROUND_ON = (1 << 0),
		SPRITES_ON = (1 << 1),
		SPRITES_SIZE = (1 << 2),
		ALT_TILE_MAP = (1 << 3),
		ALT_TILE_SET = (1 << 4),
		WINDOW_ON = (1 << 5),
		ALT_WINDOW_TILE_MAP = (1 << 6),
		DISPLAY_ON = (1 << 7)
	};

	struct D3DTLVERTEX
	{
		float x, y, z, rhw;
		DWORD color;
	};

	HWND window;
	IDirect3D9Ex* d3d9;
	IDirect3DDevice9Ex* device;
	IDirect3DTexture9* screenbuffer;

	CPU* cpu;
	Mode mode;
	Memory mem;
	unsigned short clock;

	static DWORD GetGBAColor(GBColor pixel);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void renderToTexture();	
};
