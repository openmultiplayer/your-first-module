#pragma once

// Include the general definition from which all packets derive.
#include <open.mp/Packet.hpp>

// Define the `SetWeatherPacket` structure, which holds all the data for serialisation.
struct SetWeatherPacket
	// Packets must derive from `openmp::Packet` with CRTP to inherit `.Send()` methods and more.
	: public openmp::Packet<SetWeatherPacket>
{
	// There is only one piece of data in this packet (as defined by SA:MP), an 8-bit weather ID.
	uint8_t Weather;
};

