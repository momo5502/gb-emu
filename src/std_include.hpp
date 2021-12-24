#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

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

#include "utils/utils.hpp"

#include "timer.hpp"
#include "joypad.hpp"
#include "gpu.hpp"
#include "mmu.hpp"
#include "cpu.hpp"

#include "game_boy.hpp"

//#define DEBUG_OPS
