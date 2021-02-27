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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

 Updating The Weather
----------------------

Currently the code looks up the weather once at server start, but this data needs polling regularly
to detect changes.  The way this is done is with the `OnTick` event---called every time the main
server loops through its code (once per frame).   `OnTick` events are unique per thread, and the
calbacks parameter is the number of microseconds since the last time it was called.  Checking for
weather changes need only be done once a minute, so this `OnTick` callback keeps track of time.

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

 Dependency Injection
----------------------

The simple function in the API relies too much on the module being defined as a singleton.  Instead,
the script system has dependency injection built-in, to automatically resolve and pass module
instances:

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

