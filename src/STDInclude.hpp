#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#pragma warning(push)
#pragma warning(disable: 4244)
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#pragma warning(pop)

using namespace std::literals;

#include "Utils/Utils.hpp"

#include "Timer.hpp"
#include "Joypad.hpp"
#include "GPU.hpp"
#include "MMU.hpp"
#include "CPU.hpp"

#include "GameBoy.hpp"
