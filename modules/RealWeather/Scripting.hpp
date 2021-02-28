// Include the scripting system, for the `SCRIPT_API` wrapper and more.
#include <open.mp/Server/Scripting.hpp>

// Forward reference to the entity type being looked up.
class RWWFire;

// Extend the `pawn_natives` namespace, which is where the reference lookup code is.
namespace pawn_natives
{
	// Template specialisation for looking up a `RWWFire` parameter.
	template <>
	struct ParamLookup<RWWFire>
	{
		// Method to return a (shared) pointer from a (cell) reference ID.
		static std::shared_ptr<RWWFire> Ref(cell ref);
	};
};

