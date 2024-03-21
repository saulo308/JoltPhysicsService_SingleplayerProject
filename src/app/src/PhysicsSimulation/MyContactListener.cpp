#include "MyContactListener.h"

ValidateResult MyContactListener::OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult)
{
	//cout << "Contact validate callback" << endl;

	// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
	return ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MyContactListener::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	//cout << "A contact was added" << endl;
}

void MyContactListener::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	//cout << "A contact was persisted" << endl;
}

void MyContactListener::OnContactRemoved(const SubShapeIDPair &inSubShapePair)
{ 
	//cout << "A contact was removed" << endl;
}
