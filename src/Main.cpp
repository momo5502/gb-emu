#include "STDInclude.hpp"

int main(int argc, char* argv[])
{
	if(argc >= 2)
	{
		std::string data;
		if(Utils::ReadFile(argv[1], data))
		{
			printf("Loaded game!\n");
			_getch();
		}
		else
		{
			printf("Failed to open %s", argv[1]);
		}
	}
	_getch();
	return 0;
}
