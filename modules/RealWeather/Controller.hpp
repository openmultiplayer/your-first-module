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

	// Override the default modules implementation of this method.
	static bool OptionsDescription(openmp::reporting::OptionsDescription & parent);

	// Declare the method that will return the current weather.
	std::string const & GetCurrentWeather() const
	{
		return currentWeather_;
	}

private:
	// Because the API returns weather names as strings, this function converts them to game IDs.
	int ConvertWeatherToID(std::string const & weatherName);

	// When a player connects this module is informed.  Declare the method used for this.
	bool OnPlayerConnect(openmp::Player_s player);

	// Declare the method to be called every time the `OnTick` event fires.
	bool OnTick(uint32_t elapsedMicroSeconds);

	// Remember the name of the real-world weather, to be used repeatedly.  Invalid default.
	std::string
		currentWeather_ = "";

	// This member keeps track of the number of microseconds since the last poll.  The default means
	// it will be called instantly.
	uint32_t
		timeSinceLastPoll_ = pollRate_ * 1000000;

	// This static member stores the number of seconds for the poll rate from settings.
	static inline uint32_t
		pollRate_ = 60;

	// A static variable to store the location in.  Options are global and shared between all
	// instances of a module (of which there is only one here).
	static inline std::string
		realWorldLocation_ = "";
};

