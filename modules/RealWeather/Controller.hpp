#pragma once

// All modules derive from `BaseModule`, included transitively here.
#include <open.mp/Module.hpp>

// Include the basic definition of a player, as the code now needs to reference individuals.
#include <open.mp/Player.hpp>

// The main controller class for this module.
class RealWeatherController
	// Since there is only one instance of this module, it derives from `SingletonModule` with CRTP.
	: public openmp::SingletonModule<RealWeatherController>
{
public:
	// Declare the constructor.
	RealWeatherController();

private:
	// Because the API returns weather names as strings, this function converts them to game IDs.
	int ConvertWeatherToID(std::string const & weatherName);

	// When a player connects this module is informed.  Declare the method used for this.
	bool OnPlayerConnect(openmp::Player_s player);

	// Remember the name of the real-world weather, to be used repeatedly.
	std::string
		currentWeather_;
};

