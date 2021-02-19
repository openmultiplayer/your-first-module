#pragma once

// Include the main header for declaring per-player data.
#include <open.mp/Server/PlayerData.hpp>

// All per-player data is derived from `openmp::PlayerData`, to inherit auto-allocation and casting.
class RealWeatherPlayerData : public openmp::PlayerData
{
public:
	// Just one boolean.  Could use a smaller array, or accessor functions.
	bool Enabled = false;
};

