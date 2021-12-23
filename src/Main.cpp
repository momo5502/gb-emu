#include "STDInclude.hpp"

int _main(int argc, char* argv[])
{
#ifdef DEBUG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	FILE* fp;
	freopen_s(&fp, "conin$", "r", stdin);
	freopen_s(&fp, "conout$", "w", stdout);
	freopen_s(&fp, "conout$", "w", stderr);

#ifdef DEBUG_OPS
	freopen_s(&fp, "out.txt", "w", stdout);
	freopen_s(&fp, "out.txt", "w", stderr);
#endif
#endif

	if (argc < 2)
	{
		printf("Missing arguments!\n");
		_getch();
		return 1;
	}

	std::string data;
	if (!Utils::ReadFile(argv[1], data))
	{
		printf("Failed to open %s", argv[1]);
		return 1;
	}

	printf("Loaded game!\n");

	GameBoy gameBoy;
	gameBoy.loadRom(data);
	gameBoy.skipBIOS();
	gameBoy.run();

	printf("Terminated!\n");

	_getch();
	return 0;
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    return _main(__argc, __argv);
}