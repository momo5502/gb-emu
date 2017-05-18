#pragma once

class GPU
{
public:
	GPU();
	~GPU();

	void frame();
	bool working();

	unsigned char* getMemoryPtr(unsigned short address);

private:
	struct Memory
	{
		unsigned char na1;
		unsigned char na2;
		unsigned char yscrl;
		unsigned char xscrl;
		unsigned char curline;
		unsigned char raster;

		unsigned char reg[0xFF];
	};

	HWND window;
	IDirect3D9Ex* d3d9;
	IDirect3DDevice9Ex* device;

	Memory mem;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
