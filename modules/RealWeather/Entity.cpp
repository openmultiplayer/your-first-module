// Include the definition of this class.
#include "Entity.hpp"

// Include the basic entity definition, for faster development.
#include <open.mp/Entities/Basic.hpp>

// Include the packet structs.
#include <open.mp/Entities/Networking.hpp>

// constructor
	RWWFire::
	RWWFire(entity_id id, glm::vec3 const & position)
:
	// Call the parent constructor, which deals with positions, amongst other things.
	BasicEntity(position)

	// Store the ID.
	, id_(id)
{
	// Explosions defining fires never move or change, so a packet could be created here in advance
	// and stored for later in the `RWWFire` entity instance, constantly re-used for display.
}

// Method to generate and send a packet to show this fire.
void
	RWWFire::
	Show() const
{
	// Fires are simulated by explosion type 9, and this is re-shown every few seconds to keep
	// burning.
	CreateExplosionPacket {
		// Always required in all packets.
		{},
		// The position, stored in the parent `BasicEntity` class.
		GetPosition(),
		// The type (type 9 for now).
		9,
		// The radius in game units.  Is customised.
		radius_
	// Send the packet from this entity, meaning to all players that have this entity streamed in.
	}.SendFrom(*this);
}

