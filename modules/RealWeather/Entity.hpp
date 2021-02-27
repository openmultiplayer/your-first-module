#pragma once

// Include the basic entity definition, for faster development.
#include <open.mp/Entities/Basic.hpp>

// Define the maximum number of fires (explosions) the game can create at once.
#define MAX_FIRES (32)

// Inherit from `BasicEntity` (with CRTP) to inherit players, positions, worlds, and more.
class RWWFire : public openmp::BasicEntity<RWWFire>
{
public:
	// Constructor taking an ID (auto-assigned) and a world position.
	RWWFire(entity_id id, glm::vec3 const & position);

	// Method to generate and send a packet to show this explosion.
	void Show() const;

	// Simple function to get the ID if requried.
	entity_id ID() const
	{
		return id_;
	}

	// Method called by the streamer to initially show the entity.  Unused as `Show` handles this.
	bool StreamInForPlayer(openmp::Player_s player)
	{
		return true;
	}

	// Method called by the streamer to finally hide the entity.  Unused as `Show` handles this.
	bool StreamOutForPlayer(openmp::Player_s player)
	{
		return true;
	}

private:
	// The ID of this explosion, relative only to other explosions.
	entity_id const
		id_;
};

