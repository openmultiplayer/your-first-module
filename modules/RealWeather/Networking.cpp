// Some players will be using the legacy (old) SA:MP client instead of the new open.mp client.  For
// those players some extra steps need to be taken to send packets to them.

// Include details of this module's packets.
#include "Networking.hpp"

// Include the legacy networking subsystem.
#include <open.mp/Server/RakNetProcessor.hpp>

// All legacy code is confined to the `openmp::legacy` namespace to separate it from modern code.
namespace openmp
{
// Some versions of C++ support `namespace openmp::legacy {}` syntax, nesting is better supported.
namespace legacy
{
// A deeper namespace to disambiguate packets, RPC IDs, and serialisers.
namespace legacy_rpc
{
	// In SA:MP, RPCs were defined by a list of arbitrary numbers.  To set weather is ID 152.
	legacy_rpc_type const
		SetWeatherRPC = 152;
};

// This is an outgoing packet, i.e. from server to client.  Define the legacy serialiser for it.
static RakNetOutgoing<SetWeatherPacket, legacy_rpc::SetWeatherRPC>
	// This initialisation auto-registers the serialiser and subscribes to outgoing packets.
	legacySetWeatherSerialiser_;
};
};

