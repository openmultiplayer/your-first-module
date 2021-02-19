// Include the controller's header.
#include "Controller.hpp"

// Include this module's packet defintions.
#include "Networking.hpp"

// Include this module's per-player data.
#include "Data.hpp"

// For `std::cout` debugging.
#include <iostream>

// Imaginary real world weather lookup library.
#include <imaginary-real-world-weather-lookup-library>

// No additional includes are required to use `OnTick` - it is a core part of the server.
REQUIRED_EVENT(OnTick);

// Since this module is a publisher, it declares the new event, unlike just saying it is needed.
DECLARE_EVENT(OnRealWorldWeatherChange);

// constructor
	RealWeatherController::
	RealWeatherController()
:
	// Pass a human-friendly name for this module through to the parent constructor.
	SingletonModule<RealWeatherController>("Real Weather")

	// Initialise the event publisher to connect to the named event.
	, OnRealWorldWeatherChange_(::OnRealWorldWeatherChange)
{
	std::cout << "Real World Weather module: v0.10" << std::endl;

	// There is no longer any need to send the weather in the constructor.  There are no players.

	// There is no longer any need to subscribe to `OnPlayerConnect`.  The default is no weather.

	// Start listening to the `OnTick` event.
	On(::OnTick, &RealWeatherController::OnTick);

	// Set the event return processing type to `ALL_1`.
	OnRealWorldWeatherChange_.BreakMode(PUB_SUB_CHAIN::ALL_1);

	// Register the per-player data with the server, so it is (de)allocated with all players.
	openmp::PlayerData::Register<RealWeatherPlayerData>();
}

// Override for the `Module` base class method.  Called before the constructor.
bool
	RealWeatherController::
	OptionsDescription(openmp::reporting::OptionsDescription & parent)
{
	// Create the namespace for options in this module.  `parent` is defined as `modules.`, so with
	// the (shortened) namespace of `rww` (for "Real World Weather"), all options on the command-
	// line will be prefixed with `--modules.rww.`.
	parent.AddOptions("rww")
		// Add the options.  Each one consists of five parts:
		//
		//   `"location"` - The name of the option.  Will become `--modules.rww.location`.
		//   `value<std::string>` - The type of the option.
		//   `(&realWorldLocation_)` - A reference to the member in which to store the value.
		//   `"The real...` - A human-friendly description displayed with `--help`.
		//   `default_value(60)` - The poll rate is optional so a `default_value` is added.
		//
		("location", boost::program_options::value<std::string>(&realWorldLocation_), "The real location in the world to copy the current weather from.")
		("pollrate", boost::program_options::value<uint32_t>(&pollRate_)->default_value(60), "How often (in seconds) to check for new weather (default 60).")
	;

	// This module has options, so return `true`.
	return true;
}

// Define the method called every time a player connects and the `OnPlayerConnect` event fires.
bool
	RealWeatherController::
	TogglePlayer(openmp::Player_s player, bool enabled)
{
	// Get a handle to the real-world weather data associated with the player.
	RealWeatherPlayerData &
		weatherPlayerData = player_cast<RealWeatherPlayerData &>(player);

	// Check if the player settings are already the same.
	if (enabled == weatherPlayerData.Enabled)
	{
		// No change
		return false;
	}

	// Store the fact that this player can (or can't) see the real-world weather.
	weatherPlayerData.Enabled = enable;

	// If the syncing is being disabled, there's nothing more to do.  No packets to send.
	if (enable == false)
	{
		// Setting was changed.
		return true;
	}

	// Create an anonymous packet and send to the one player that was just enabled.
	SetWeatherPacket {
		// Due to a limitation in how C++ structs derive, all packet structures start with `{}`.
		{},

		// Convert from the string name of weather to a San Andreas weather type.
		ConvertWeatherToID(currentGameWeather_),
	}.SendTo(player);

	// Setting was changed.
	return true;
}

// Define the method called every time the main server loops and the `OnTick` event fires.
bool
	RealWeatherController::
	OnTick(uint32_t elapsedMicroSeconds)
{
	// Keep track of time between weather polls.
	timeSinceLastPoll_ += elapsedMicroSeconds;
	
	// Poll with a frequency given by the `pollrate` setting converted from seconds to microseconds.
	if (timeSinceLastPoll_ < pollRate_ * 1000000)
	{
		// Insufficient time has passed.
		return false;
	}

	// Adjust down for the next time.  Subtracting instead of resetting reduces jitter.
	timeSinceLastPoll_ -= pollRate_ * 1000000;

	// Get the current weather in the selected real-world location.
	std::string
		newWeather = LookUpRealWorldWeather(realWorldLocation_);

	// Check if the weather has actually changed.
	if (newWeather == currentRealWeather_)
	{
		// The weather hasn't changed.
		return false;
	}

	// It has changed.  Store it and inform subscribers.
	currentRealWeather_ = newWeather;

	// Publish the event.  With function call syntax to make this simpler.
	if (OnRealWorldWeatherChange_(currentRealWeather_))
	{
		// The change was accepted.  Store it and inform players.
		currentGameWeather_ = currentRealWeather_;

		// Create a named packet, for sending to some players later.
		SetWeatherPacket
			weatherPacket {
				{},
				ConvertWeatherToID(currentGameWeather_),
			};

		// Use the defined `PlayerPool` iterator to loop over all players.
		for (auto & player : PlayerPool::Instance())
		{
			// Get this player's custom real-world weather data.
			RealWeatherPlayerData &
				weatherPlayerData = player_cast<RealWeatherPlayerData &>(player);

			// Send the weather to only enabled players.
			if (weatherPlayerData.Enabled)
			{
				// Re-use a single packet instance, not a temporary struct instance.
				weatherPacket.SendTo(player);
			}
		}
	}

	// Ignored in this specific event, but still required.
	return true;
}

// Common APIs will not return the current weather as an ID that the game will understand.  This
// function converts from a real weather name to an in-game weather ID.  It does not handle many
// cases, both because there are not many valid weather types in-game, and because fool-proof
// parsing of API responses is not the point of this modules guide.
int
	RealWeatherController::
	ConvertWeatherToID(std::string const & weatherName)
{
	// See the open.mp wiki for more weather types:
	//
	//     https://open.mp/docs/scripting/resources/weatherid
	//
	if (weatherName == "sunny")
	{
		return 5;
	}
	if (weatherName == "rainy")
	{
		return 8;
	}
	if (weatherName == "foggy")
	{
		return 9;
	}
	if (weatherName == "cloudy")
	{
		return 7;
	}

	// No other matches, return something else.
	return 19;
}

