#include "STDInclude.hpp"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Missing arguments!\n");
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
	gameBoy.run();

	printf("Terminated!\n");

	_getch();
	return 0;
}
