// Include the controller's header.
#include "Controller.hpp"

// For `std::cout` debugging.
#include <iostream>

// constructor
	RealWeatherController::
	RealWeatherController()
:
	// Pass a human-friendly name for this module through to the parent constructor.
	SingletonModule<RealWeatherController>("Real Weather")
{
	std::cout << "Real World Weather module: v0.1" << std::endl;
}

