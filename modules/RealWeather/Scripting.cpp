// Include the controller, which is where the data being manipulated lives.
#include "Controller.hpp"

// Include this module's entity lookup definitions.
#include "Scripting.hpp"

// Include this module's per-player data.
#include "Data.hpp"

// Include the `RWWFire` entity for lookup and manipulation.
#include "Entity.hpp"

// Include the injectors and iterators for player pools.
#include <open.mp/Server/PlayerModule.hpp>

// Actually define the lookup method, now that the `RWWFire` class is in scope.
std::shared_ptr<RWWFire>
	pawn_natives::
	ParamLookup<RWWFire>::
	Ref(cell ref)
{
	// Call `InfinitePool::Get`, passing what is known to be an `entity_id`.
	return RealWorldController::Instance()->Get(static_cast<entity_id>(ref));
}

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

// Pass two controllers via dependency-injection.  In scripts this has separate x/y/z parameters.
SCRIPT_API(RWW_CreateFire, entity_id (vec3 position, DI<RealWorldController> controller, DI<PlayerPool> playerpool))
{
	// Call the base `InfinitePool::Emplace` method, which constructs the entity, prepending an ID.
	auto fire = controller->Emplace(position);

	// By default all entities are created displayed to eveyone.
	fire->DisplayDefault(false);

	// Loop over all current players.
	for (auto const & player : *playerpool)
	{
		// Check if this player has real-world weather enabled.
		if (player_cast<RealWeatherPlayerData &>(player).Enabled)
		{
			// Display this fire to them.
			fire->Display(player, true);
		}
	}

	// Return the ID of this entity.  Scripts don't hold true references.
	return fire->ID();
}

// No ID-based lookup is needed to destroy an fire, since that would create a new pointer.
SCRIPT_API(RWW_DestroyFire, bool (entity_id id, DI<RealWorldController> controller))
{
	// Return `true` if the fire existed and was destroyed.
	return controller->Remove(id);
}

// Use the newly defined `RWWFire` lookup scheme.  Returns `false` if the lookup failed.
SCRIPT_API(RWWFire_SetRadius, bool (std::shared_ptr<RWWFire> fire, float radius))
{
	// Set the radius.
	fire->SetRadius(radius);

	// Return true, since everything was fine with the lookup.
	return true;
}

// `SCRIPT_METHOD` REQUIRES an ID lookup, uses it for `this`, and defines the wrapper automatically.
SCRIPT_METHOD(RWWFire, GetRadius, float ())
{
	// Return the private radius from `this`, as this is the implementation of `RWWFire::GetRadius`.
	return radius_;
}

