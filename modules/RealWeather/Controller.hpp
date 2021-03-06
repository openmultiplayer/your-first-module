#pragma once

// All modules derive from `BaseModule`, included transitively here.
#include <open.mp/Module.hpp>

// Include the basic definition of a player, as the code now needs to reference individuals.
#include <open.mp/Player.hpp>

// Include the finite pool, a container for a limited number of fires.
#include <open.mp/InfinitePool.hpp>

// Include the definition of an fire "entity" (in-game world item).
#include "Entity.hpp"

// Define the new event.  Takes a single parameter - the name of the new weather.
DEFINE_EVENT(OnRealWorldWeatherChange, (std::string const & newWeather));

#define MICROSECONDS_TO_SECONDS (1000000)

// The main controller class for this module.
class RealWeatherController
	// Since there is only one instance of this module, it derives from `SingletonModule` with CRTP.
	: public openmp::SingletonModule<RealWeatherController>
	// The module is now an infinite pool as well, the container for unlimited fires.
	, public openmp::InfinitePool<RWWFire>
{
public:
	// Declare the constructor.
	RealWeatherController();

	// Override the default modules implementation of this method.
	static bool OptionsDescription(openmp::reporting::OptionsDescription & parent);

	// Declare the method that will return the current weather.
	std::string const & GetCurrentWeather() const
	{
		return currentGameWeather_;
	}

	// Used to enable (sync the real-world weather to them) or disable a player.
	bool TogglePlayer(openmp::Player_s player, bool enabled);

private:
	// Because the API returns weather names as strings, this function converts them to game IDs.
	int ConvertWeatherToID(std::string const & weatherName);

	// Update how many seconds have passed, and check if that passed a threshold
	bool CheckElapsedTime(uint32_t* counter, uint32_t elapsedMicroSeconds, uint32_t threshold) const;

	// Update the current real-world weather.
	void UpdateWeather();

	// Refresh fires, as they're explosions that need to be repeatedly re-shown.
	void UpdateFires();

	// Declare the method to be called every time the `OnTick` event fires.
	bool OnTick(uint32_t elapsedMicroSeconds);

	// Remember the name of the real-world weather, to be used repeatedly.  Invalid default.
	std::string
		currentRealWeather_ = "";

	// Remember the name of the in-game weather, different to real-world when a change is rejected.
	std::string
		currentGameWeather_ = "";

	// This member keeps track of the number of microseconds since the last poll.  The default means
	// it will be called instantly.
	uint32_t
		timeSinceLastPoll_ = pollRate_ * MICROSECONDS_TO_SECONDS;

	// This member keeps track of the number of microseconds since the last fire refresh.
	uint32_t
		timeSinceLastFire_ = 2 * MICROSECONDS_TO_SECONDS;

	// This static member stores the number of seconds for the poll rate from settings.
	static inline uint32_t
		pollRate_ = 60;

	// A static variable to store the location in.  Options are global and shared between all
	// instances of a module (of which there is only one here).
	static inline std::string
		realWorldLocation_ = "";

	// Declare a publisher matching the event declaration above.
	openmp::Event<std::string const &>
		OnRealWorldWeatherChange_;

	// A streamer, which determines which fires to show to a player at any given time.
	SimpleStreamer<RWWFire, RealWeatherController, MAX_FIRES>
		streamer_;
};

