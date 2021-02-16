#pragma once

// All modules derive from `BaseModule`, included transitively here.
#include <open.mp/Module.hpp>

// The main controller class for this module.
class RealWeatherController
	// Since there is only one instance of this module, it derives from `SingletonModule` with CRTP.
	: public openmp::SingletonModule<RealWeatherController>
{
public:
	// Declare the constructor.
	RealWeatherController();
};

