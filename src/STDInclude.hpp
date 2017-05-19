#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#include <d3d9.h>
#include <D3dx9tex.h>
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#pragma warning(push)
#pragma warning(disable: 4244)
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#pragma warning(pop)

using namespace std::literals;

#include "Utils/Utils.hpp"

#include "GPU.hpp"
#include "MMU.hpp"
#include "CPU.hpp"