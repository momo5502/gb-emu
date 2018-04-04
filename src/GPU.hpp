#pragma once

class GameBoy;
#define GB_WIDTH (160)
#define GB_HEIGHT (144)

#define WM_KILL_WINDOW (WM_USER+0)

class GPU
{
public:
	GPU(GameBoy* gameBoy);
	~GPU();

	void frame();
	bool working();

	unsigned char* getMemoryPtr(unsigned short address);
	void updateTile(unsigned short address);
	void updateObject(unsigned short address, unsigned char value);
	void closeWindow();

	void setTitle(std::string title);

	bool isWindowActive();

private:
	struct Memory
	{
		unsigned char flags;
		unsigned char lcdStatus;
		unsigned char yscrl;
		unsigned char xscrl;
		unsigned char curline;
		unsigned char raster;
		unsigned char unk;
		unsigned char palette[3];

		unsigned char reg[0xFF];
	};

	struct Object
	{
		int y;
		int x;
		int tile;
		int palette;
		int xFlip;
		int yFlip;
		int priority;
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

	GameBoy* gb;

	Mode mode;
	Memory mem;
	unsigned int clock;
	unsigned int lastTime = 0;

	std::thread windowThread;

	COLORREF screenBuffer[GB_WIDTH * GB_HEIGHT];
	unsigned char tiles[512][8][8];
	Object objects[40];

	inline COLORREF getColorFromPalette(unsigned int palette, unsigned int index);

	static inline COLORREF GetGBColor(GBColor pixel);
	static inline COLORREF GetGBColor(unsigned char pixel);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void renderScreen();
	void renderTexture();

	void windowRunner();
};
