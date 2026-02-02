#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../lib/lib/cpu-core-debug.lib")
#pragma comment(lib, "../../lib/lib/cpu-render-debug.lib")
#pragma comment(lib, "../../lib/lib/cpu-engine-debug.lib")
#else
#pragma comment(lib, "../../lib/lib/cpu-core.lib")
#pragma comment(lib, "../../lib/lib/cpu-render.lib")
#pragma comment(lib, "../../lib/lib/cpu-engine.lib")
#endif

#include "../../lib/include/cpu-engine/cpu-engine.h"

#include "Packets.h"
#include "Utils.h"