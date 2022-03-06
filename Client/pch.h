#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug\\Engine.lib")
#else
#pragma comment(lib, "Release\\Engine.lib")
#endif

#include "EnginePch.h"
#include <vector>
#include <memory>

using namespace std;