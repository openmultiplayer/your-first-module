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

	// Initialise the fires pool.  This confers iterators, creation, destruction, and more.
	, FinitePool<RWWFire, MAX_FIRES>()

	// Initialise the event publisher to connect to the named event.
	, OnRealWorldWeatherChange_(::OnRealWorldWeatherChange)
{
	std::cout << "Real World Weather module: v0.11" << std::endl;

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

	// If the syncing is being disabled there's no packets to send.
	if (enable == false)
	{
		// Update all fire entities so they no longer send their packets to this player.
		player_id pid = player->ID();

		// Loop over all the fires using the iteration methods inherited from `FinitePool`.
		for (auto & fire : *this)
		{
			// Don't `Display` it.
			fire.Display(pid, false);
		}

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

	// Update all fire entities so they send their packets to this player next refresh (soonish).
	player_id pid = player->ID();

	// Loop over all the fires using the iteration methods inherited from `FinitePool`.
	for (auto & fire : *this)
	{
		// `Display` it.  When streaming this means they are ALLOWED to see the entity.
		fire.Display(pid, true);

		// Show it in `UpdateFires`, not here.  "Allowed to see" and "currently seen" are different.
	}

	// Setting was changed.
	return true;
}

// Define the method called every time the main server loops and the `OnTick` event fires.
bool
	RealWeatherController::
	OnTick(uint32_t elapsedMicroSeconds)
{
	// Check if `pollrate` seconds have passed.
	if (CheckElapsedTime(&timeSinceLastPoll_, elapsedMicroSeconds, pollRate_))
	{
		// And if so, update the weather.
		UpdateWeather();
	}

	// Check if two seconds have passed.
	if (CheckElapsedTime(&timeSinceLastFire_, elapsedMicroSeconds, 2))
	{
		// And if so, refresh the fires.
		UpdateFires();
	}

	// Ignored in this specific event, but still required.
	return true;
}

// Define a helper method to check how much time has passed between two updates.
bool
	RealWeatherController::
	CheckElapsedTime(uint32_t* counter, uint32_t elapsedMicroSeconds, uint32_t threshold) const
{
	// Keep track of time between polls.
	*counter += elapsedMicroSeconds;
	
	// Poll with a frequency given by the `threshold` setting converted from seconds to microseconds.
	if (*counter < threshold * MICROSECONDS_TO_SECONDS)
	{
		// Insufficient time has passed.
		return false;
	}

	// Adjust down for the next time.  Subtracting instead of resetting reduces jitter.
	*counter -= threshold * MICROSECONDS_TO_SECONDS;

	// Sufficient time has passed.
	return true;
}

void
	RealWeatherController::
	UpdateWeather()
{
	// Get the current weather in the selected real-world location.
	std::string
		newWeather = LookUpRealWorldWeather(realWorldLocation_);

	// Check if the weather has actually changed.
	if (newWeather == currentRealWeather_)
	{
		// The weather hasn't changed.
		return;
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
}

// A simple method which, at a fixed interval, resends explosions so they don't peter out.
void
	RealWeatherController::
	UpdateFires()
{
	// Loop over all the fires.  Thd base class `FinitePool` defines a contained entity iterator.
	for (auto const & fire : *this)
	{
		// Sends the fire's data to all players with it (matches the players with RWW enabled).
		fire.Show();
	}
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

