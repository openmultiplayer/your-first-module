 Your First Module
===================

This guide will look at writing a complete module, with many common features in a reduced form.
This includes the module initialisation, options, API, events, RPCs, entities, and streamer.  The
example will be a real-world weather system.  Config options will say where in the world to match
the weather to; the API will allow enabling/disabling the weather control for players and querying
the current weather; RPCs will send the new weather to players; event subscriptions will tell the
module when a player enters a building (to disable their weather); and event publishers will tell
other modules when the weather changes.  Finally, for a bit of flair, entities and a streamer will
be used to create fire when the weather is lightning.

 Getting Started
-----------------

The first step in creating a module is... creating a module.  All modules must derive from one of
the common module types, which provides many common functions and register the module with the
server.  There will be just one instance of this module, but this is not currently done
automatically so needs to be added to `main.cpp`:

```cpp
// With the other includes.
#include <modules/RealWeather/Controller.hpp>
```

```cpp
	// In the list of modules in `using App = Server<...>;`
	RealWeatherController,
```

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/10e4e6bb5083af226482fbba5fd61ee23b49c874)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/10e4e6bb5083af226482fbba5fd61ee23b49c874/modules/RealWeather)

 Sending The Weather
---------------------

To know the real-world weather, the module uses an external weather information source.  The details
of exactly how to do this aren't important to showing how to write a module---it is an
implementation detail of what the module does, not part of how a module is laid out.  For now, it is
assumed that there is some library that will enable this feature via a  function.

The module now knows what the current weather is, but the data isn't much use confined to the
server.  It needs to be sent, using ***packets*** to the players.  A packet is a structure that
contains data to be sent "over the wire", that is, over the network.  Data is sent as either a
***sync*** packet, one that updates information such as player position constantly; or as an
***rpc*** packet, a ***R***emote ***P***rocedure ***C***all that triggers a specific action.  Since
changing the weather is a single action, it is an RPC.  For compatibility reasons, the weather
packet matches the structure of the SA:MP `SetWeather` RPC.  It doesn't need to, but not doing so
necessitates more code to convert to the old format.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/e2ce1a2cd8b8b4de3f3cafdd55c6d0d8dea5740e)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/e2ce1a2cd8b8b4de3f3cafdd55c6d0d8dea5740e/modules/RealWeather)

 Joining Players
-----------------

The current weather is sent in the controller constructor to all players connected at that exact
moment.  Players who join later will not have the new weather.  Sadly, since the constructor is
called as soon as the server starts it is not possible for any players to yet be connected.  The
code never tells anyone.  To inform new players of the weather the module must ***subscribe*** to
the `OnPlayerConnect` ***event*** and respond to it by sending them the current weather.

An event is something that happens somewhere in the server that other code wants to know about.
When an event happens the source ***publishes*** this fact, and code that wants to be informed
***subscribes*** to the event.  Events have names and parameters---the names are used to determine
which events to subscribe to, and the parameters pass additional details about what exactly
happened.  For `OnPlayerConnect` the callback parameter is the player that connected.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/f26b974cac9857e9fec279c0979ae13c66322201)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/f26b974cac9857e9fec279c0979ae13c66322201/modules/RealWeather)

 Updating The Weather
----------------------

Currently the code looks up the weather once at server start, but this data needs polling regularly
to detect changes.  The way this is done is with the `OnTick` event---called every time the main
server loops through its code (once per frame).   `OnTick` events are unique per thread, and the
calbacks parameter is the number of microseconds since the last time it was called.  Checking for
weather changes need only be done once a minute, so this `OnTick` callback keeps track of time.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/648c8f67b2697ff6a8e1fc218d5706d981edc693)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/648c8f67b2697ff6a8e1fc218d5706d981edc693/modules/RealWeather)

 Selecting A Location
----------------------

The real-world weather is tied to Malta.  Malta is very nice, but not everyone wants their server to
match Maltese weather, so a run-time setting should be added to allow server owners to configure
this mirrored location.  To create some options for the module, add the `OptionsDescription` static
method to the module, and return `true` from it.  The options need a namespace, so they don't
conflict with global options; a name, so they can be used; and a destination, so they can be stored
and retrieved.

Additionally, the weather currently updates once per minute (every 60 seconds).  This poll rate
should be configurable, to the nearest second, and with a default.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/9d6de8da92bf6be4beadb88af7fdadf93fa21663)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/9d6de8da92bf6be4beadb88af7fdadf93fa21663/modules/RealWeather)

To set the option, launch the server with an addition flag
(`server.exe --modules.rww.location "Siberia"`) or add a nested setting to a config file:

### `config.json`

```json
{
  "modules": {
    "rww": {
      "location": "Leicester, UK",
	  "pollrate": 120
    }
  }
}
```

 API
-----

The API is what allows scripts to interact with a module; to get and set data, or issue commands.
API functions are declared using a wrapper syntax that allows many different languages to
automatically derive bindings; the examples here will use PAWN.  The first function to add to this
module allows scripts to retrieve the current real-world weather:

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/b41ea2504d31fb182bc3838f7ee7da17efb163e6)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/b41ea2504d31fb182bc3838f7ee7da17efb163e6/modules/RealWeather)

 Dependency Injection
----------------------

The simple function in the API relies too much on the module being defined as a singleton.  Instead,
the script system has dependency injection built-in, to automatically resolve and pass module
instances:

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/513598cdc90f0ec3190827f5f3bdbe437d011a5a)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/513598cdc90f0ec3190827f5f3bdbe437d011a5a/modules/RealWeather)

 Creating New Callbacks
------------------------

If the weather changes, that's an interesting thing to happen.  Interesting things that happen are
events, and events are things that other code should be told about.  Thus, when weather changes an
event should be fired, making this module a ***publisher***.  All modules are publishers by default,
so the event just needs declaring and called.  Events seem to be declared twice---once with
`DECLARE_EVENT();` and once with `OnEvent_()`.  These are not the same.  The former declares the
actual internal event, the thing that publishers publish to and subscribers subscribe to.  The
latter declares the publisher that connects to the event.  This publisher is not unique---many
different modules (and other code) can all publish the same event.  Fortunately, `DECLARE_EVENT`
does not need to be unique, it can appear multiple times in code yet only declare a single event.

`DEFINE_EVENT();` and `Event<> OnEvent_;` are normal prototypes.  It is convention to name events
as `OnEvent` and the publishers as `OnEvent_`, differing by just the trailing underscore.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/978a5ea0f5b7ab04ed169b0c9c53e289718c775d)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/978a5ea0f5b7ab04ed169b0c9c53e289718c775d/modules/RealWeather)

 Event Returns
---------------

Some weathers are interesting, some aren't, and some can't actually be shown in-game.  The return
value from `OnRealWorldWeatherChange` should be used to determine whether or not to switch to the
new weather.  If any subscriber doesn't like the new weather (returns `false`), don't switch.
Subscribers can only return `true` or `false` from their callback, but what that means and how those
return values are combined in the case of multiple subscribers, varies depending on the semantics
of the event itself.  For `OnRealWorldWeatherChange` the event should return `false` if any
subscriber returns false, but all subscribers should always be called regardless.  This is return
mode `ALL_1`---the event returns `true` if all subscribers return `true` and returns `true` (`1`) if
there are no subscribers at all.  The default is `ANY_0`---the event returns `true` if any
subscriber returned `true`, but returns `false` (`0`) if there are no subscribers at all.  Another
way to look at this is that the event returns `true` if at least one subscriber returned `true`.

The event should not be repeatedly called with the new weather when the change was already denied.
To prevent this event spam both the current in-game and current real-world weathers must be tracked
separately.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/fba8626fe3fd430e597698ebcd5d3d9de6cca0a9)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/fba8626fe3fd430e597698ebcd5d3d9de6cca0a9/modules/RealWeather)

 Selecting Players
-------------------

Scripts should have a way to select which players get the synced weather---maybe it makes no sense
in certain mini-games, or for admins, or only one city has the real weather.  Whatever the reason,
the module should know which players do and don't have the weather sync enabled.  As this needs to
be stored per-player, and reset when new players connect, it is ***player data***.  Any module can
define custom player data by extending from the `PlayerData` class.  This will handle allocations
for new players automatically, and provides the `player_cast` operator to convert from an instance
of a player to an instance of the module's data.  Previously all player data was stored in the
`Player` class, but this became both monolithic and a compilation bottle-neck.

Once the data is set, the update function needs to use this information to determine who to send the
update packet to.  There is also no longer a need for the initial setup in `OnPlayerConnect`, it is
instead moved to be done when the syncing is enabled for the selected player.  The simplest way to
do this is a loop, and this is used here.  However, a more clever way to do it would be to use some
intrinsic properties of ***entities***---any item seen in-game.  Entities have lists of players for
whom they are enabled already, and a way to send data to all those players
(`packet.SendFrom(entity)`).  Were the weather defined as an entity, both these features would be
included for free.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/f73ce7487b60a617fc7340d117aa03d169ee06f3)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/f73ce7487b60a617fc7340d117aa03d169ee06f3/modules/RealWeather)

 Fires!
-------------

There's a huge storm raging.  Thunder rattles the windows.  Lightning turns night to day.
Everywhere you look power lines are down, torrents of water wash cars away, and people scramble
helplessly to protect their most precious possessions from ruin.  Every ground strike, each
measuring millions of volts, lights a new fire; overwhelming the LS, SF, and LV fire departments.
The city is in tatters.

To represent this carnage, the `CreateExplosion` RPC can be used to place fires all over San
Andreas.  The game doesn't have real fires, but explosion type 9, constantly refreshed, can be used
to simulate them.  A fire is an ***entity***---something that appears in the game world; and has a
position, virtual worlds, and a list of players for whom it is visible.  The list of players in this
case will always exactly match the list of players who have the real-world weather enabled, but can
be leveraged for sending packets.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/46906b081251f10dda68e319d4975368730bad97)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/46906b081251f10dda68e319d4975368730bad97/modules/RealWeather)

 MORE Fires!
-------------

Sadly, fires (explosions) in the legacy SA:MP client are extremely limited - there's no way to cover
an entire state with them.  This is where a streamer comes in.  A streamer shows only the closest
few of something to a player.  So if there are 1000 fires anywhere, but the game can only render 32
at once, only 32 are sent to the player.  As they move around the world the list of which subset of
all fires are shown to them is updated.  The basic operation of a streamer is fairly simple: measure
the distance between the player and every fire, rank them by distance, and pick the closest ones.
There are many many ways to improve this, but for fires this method is both sufficient and provided
by open.mp out of the box.  Adding a streamer is as simple as switching to an infinite pool and then
declaring the streamer.

It is important to distinguish between ***displayed*** and ***visible*** when using a streamer.  A
***displayed*** entity (set using `.Display` on the `BasicEntity` base class) means that the player
is ***allowed*** to use/view/interact with the entity; the streamer uses this when determining which
entities to stream to that player.  ***visible*** means that the entity is actually currently on the
client, after a streamer has determined that it should be right now.  The method
`entity.Has(player)` will return `true` if it is ***displayed*** to that player, regardless of
whether or not they have been sent the data and thus know about it.

* Diff for this step: [View On Github](https://github.com/openmultiplayer/your-first-module/commit/8faea7d86403f411f7fc658e8d01a370ec5d0c37)

* Full code at this step: [View On Github](https://github.com/openmultiplayer/your-first-module/tree/8faea7d86403f411f7fc658e8d01a370ec5d0c37/modules/RealWeather)

