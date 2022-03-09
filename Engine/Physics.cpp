#include "pch.h"
#include "Physics.h"

using namespace physx;

DEFINITION_SINGLE(Physics)

Physics::Physics() {}
Physics::~Physics() {}

void Physics::Init()
{
	// Foundation�� ����� �ݴϴ�.
	_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocator, _errorCallback);
	
	if (!_pFoundation)
		throw("PxCreateFoundation failed!");

	_pvd = PxCreatePvd(*_pFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// PhysX�� ����� �ݴϴ�.
	bool recordMemoryAllocations = true;
	_toleranceScale.length = 100;
	_toleranceScale.speed = 981;

	_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *_pFoundation,
		_toleranceScale, recordMemoryAllocations, _pvd);

	if (!_pPhysics)
		throw("PxCreatePhysics failed!");


}
