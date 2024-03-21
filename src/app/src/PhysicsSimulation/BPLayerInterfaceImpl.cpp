#include "BPLayerInterfaceImpl.h"

BPLayerInterfaceImpl::BPLayerInterfaceImpl()
{
	// Create a mapping table from object to broad phase layer
	mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
}

uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
{
	return BroadPhaseLayers::NUM_LAYERS;
}

BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(ObjectLayer inLayer) const
{
	return mObjectToBroadPhase[inLayer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const
{
	switch((BroadPhaseLayer::Type)inLayer)
		{
	    case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
            return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
            return "MOVING";
        default:													
            JPH_ASSERT(false); 
            return "INVALID";
    }
}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
