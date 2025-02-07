#include "ObjectVsBroadPhaseLayerFilterImpl.h"

bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const
{
	switch (inLayer1)
	{
	    case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
	    case Layers::MOVING:
			return true;	
	    default:
			JPH_ASSERT(false);
			return false;
	}
}
