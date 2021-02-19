// Include the controller, which is where the data being manipulated lives.
#include "Controller.hpp"

// Include the scripting system, for the `SCRIPT_API` wrapper and more.
#include <open.mp/Server/Scripting.hpp>

// Include this module's per-player data.
#include "Data.hpp"

// Define an external interface to this module.  The PAWN language provider converts an output
// string to a destination array and length, so this would be referenced as:
//
//     native RWW_GetCurrentWeather(string:output[], length = sizeof (output));
//
// And used as:
//
//    new weather[32];
//    RWW_GetCurrentWeather(weather);
//    printf("The current real-world weather is: %s", weather);
//
// The name is `RWW_GetCurrentWeather`, and the return type is `void`.  Out parameters are always
// passed as pointers, to differentiate them to the marshalling templates.
//
// Instead of using global statics, the controller is passed in via dependency-injection.
SCRIPT_API(RWW_GetCurrentWeather, void (std::string * output, DI<RealWorldController> controller))
{
	// Call the method on the controller that gets the internal weather data.
	*output = controller->GetCurrentWeather();
}

// The `Player_s` pointer is passed as a simple ID and resolved by the scripting system.
SCRIPT_API(RWW_TogglePlayer, bool (openmp::Player_s player, bool toggle, DI<RealWorldController> controller))
{
	// The variable could be set here, but that wouldn't instantly set the weather.
	return controller->TogglePlayer(player, toggle);
}

// The RealWeatherPlayerData pointer is also passed as a player ID, with automated lookup and cast.
SCRIPT_API(RWW_IsPlayerEnabled, bool (std::shared_ptr<RealWeatherPlayerData> player))
{
	return player->Enabled;
}

