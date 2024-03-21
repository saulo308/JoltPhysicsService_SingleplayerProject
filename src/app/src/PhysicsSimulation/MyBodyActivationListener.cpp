#include "MyBodyActivationListener.h"

void MyBodyActivationListener::OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData)
{
	cout << "A body got activated" << endl;
}

void MyBodyActivationListener::OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData)
{
	cout << "A body went to sleep" << endl;
}
