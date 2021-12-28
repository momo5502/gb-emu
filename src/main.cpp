#include "std_include.hpp"
#include <conio.h>
#include <Windows.h>

#include "utils/utils.hpp"

#include "game_boy.hpp"
#include "joypad_win.hpp"
#include "display_win.hpp"

int main(const int argc, char* argv[])
{
#ifdef NDEBUG
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

	std::vector<uint8_t> data{};
	if (!utils::read_file(argv[1], data))
	{
		printf("Failed to open %s", argv[1]);
		return 1;
	}

	printf("Loaded game!\n");

	display_win display{};
	joypad_win joypad{&display};

	game_boy game_boy{&joypad, &display};

        bool was_loaded = false;

        if(data.size() > 4 && !memcmp(data.data(), "LUL\0", 4)) {
	  was_loaded = true;
	  std::string s(reinterpret_cast<char*>(data.data() + 4), data.size() - 4);
	  utils::binary_buffer b(std::move(s));
	  game_boy.serialize(b);
        }
	else {
	  game_boy.load_rom(data);
	  game_boy.skip_bios();
	}
	game_boy.run();

        utils::binary_buffer binary_buffer{};
        game_boy.serialize(binary_buffer);

        FILE* f{};
        fopen_s(&f, was_loaded ? argv[1] : (argv[1] + ".sav"s).data(), "wb");
        fwrite("LUL\0", 1, 4, f);
        fwrite(binary_buffer.get_buffer().data(), 1, binary_buffer.get_buffer().size(), f);
        fclose(f);

	printf("Terminated!\n");

#ifndef DEBUG_OPS
	_getch();
#endif
	return 0;
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, char*, int)
{
	return main(__argc, __argv);
}
