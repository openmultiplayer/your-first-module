// Include the controller's header.
#include "Controller.hpp"

// Include this module's packet defintions.
#include "Networking.hpp"

// For `std::cout` debugging.
#include <iostream>

// Imaginary real world weather lookup library.
#include <imaginary-real-world-weather-lookup-library>

// constructor
	RealWeatherController::
	RealWeatherController()
:
	// Pass a human-friendly name for this module through to the parent constructor.
	SingletonModule<RealWeatherController>("Real Weather")
{
	std::cout << "Real World Weather module: v0.2" << std::endl;

	// Get the weather for "Malta" from the real-world lookup system.
	std::string
		currentWeather = LookUpRealWorldWeather("Malta");

	// Create a packet to send to all players.
	SetWeatherPacket
		weatherPacket {
			// Due to a limitation in how C++ structs derive, all packet structures start with `{}`.
			{},

			// Convert from the string name of weather to a San Andreas weather type.
			ConvertWeatherToID(currentWeather),
		};

	// Now send the packet to all players, telling their games to change their weather.
	weatherPacket.SendToAll();
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

