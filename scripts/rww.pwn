// Include the main open.mp includes
#include <open.mp>

// Include colandreas, which actually defines `#pragma library` already for auto-loading.
#include <colandreas>

// Declare the natives added by the Real World Weather module.
native RWWFire:RWW_CreateFire(Float:x, Float:y, Float:z);

// Added tags for increased compile-time safety.
native RWW_DestroyFire(RWWFire:fire);

// Both player references and player data references are resolved from simple IDs in scripts.
native bool:RWW_IsPlayerEnabled(playerid);

// The last parameter in the C++ is a `DI<>` parameter---it is dependency injected, not passed here.
native bool:RWW_TogglePlayer(playerid, bool:toggle);

// String returns in pawn are two parameters---a string and a max length.
native void:RWW_GetCurrentWeather(string:weather[], length = sizeof (weather));

// Forward the callback from the module.  This string is an input, so no length required.
forward OnRealWorldWeatherChange(string:newWeather[]);

// Declare space to remember 100 fires.
static
	RWWFire:gFires[100];

// Callbacks matching the names of pubsub events are automatically subscribed with a low priority.
public OnRealWorldWeatherChange(string:newWeather[])
{
	// Check if we switched to a storm.
	if (!strcmp(newWeather, "stormy"))
	{
		// There wasn't a storm, but now is.  Create all the fires.
		for (new i = 0; i != sizeof (gFires); ++i)
		{
			// Generate a random 2D location within the world limits (+/-3000 units).
			new
				Float:x = RandomFloat(-3000, 3000),
				Float:y = RandomFloat(-3000, 3000),
				Float:z;

			// Look up the ground level at this random location.
			if (!CA_FindZ_For2DCoord(x, y, z))
			{
				// The lookup failed.  See the `colandreas` documentation for more details.
				continue;
			}

			// Check that this position isn't on or below the water.
			if (z <= 0.0)
			{
				continue;
			}

			// Create the fire and store the result.
			gFires[i] = RWW_CreateFire(x, y, z);
		}

		// No need to check the previous weather because this is only called when it changes.
		return true;
	}

	// Check if we were in a storm before and have switched away from it.
	new oldWeather[32];
	RWW_GetCurrentWeather(oldWeather);
	if (!strcmp(oldWeather, "stormy"))
	{
		// There was a storm, but now isn't.  Destroy all the fires.
		for (new i = 0; i != sizeof (gFires); ++i)
		{
			// Check if this fire was created successfully, by comparing it to `0`.
			if (!gFires[i])
			{
				continue;
			}

			// Call the native function that destroys the fires internally.
			RWW_DestroyFire(gFires[i]);

			// Reset the variable, using the default invalid entity ID of `0`.
			gFires[i] = RWWFire:0;
		}
	}

	// A return is required, and `true` to not cancel the weather change.
	return true;
}

