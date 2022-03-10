#pragma once

// #define _2D

#ifdef _2D
#ifdef _DEBUG
#pragma comment(lib, "Debug\\Engine2D.lib")
#else
#pragma comment(lib, "Release\\Engine2D.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "Debug\\Engine.lib")
#else
#pragma comment(lib, "Release\\Engine.lib")
#endif
#endif

#include "EnginePch.h"
#include <vector>
#include <memory>

using namespace std;