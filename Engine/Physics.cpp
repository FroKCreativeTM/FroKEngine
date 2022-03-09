#include "pch.h"
#include "Physics.h"

using namespace physx;

DEFINITION_SINGLE(Physics)

Physics::Physics() {}
Physics::~Physics() 
{
	if (_pPhysics)
	{
		_pPhysics->release();
	}
	_pPhysics = nullptr;

	if (_pFoundation)
	{
		_pFoundation->release();
	}
	_pFoundation = nullptr;
	
	if(_pDispatcher)
	{
		_pDispatcher->release();
	}
	_pDispatcher = nullptr;

	if (_pScene)
	{
		_pScene->release();
	}
	_pScene = nullptr;
}

void Physics::Init()
{
	/* Physx 초기화 */

	// Foundation을 만들어 줍니다.
	_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocator, _errorCallback);
	
	if (!_pFoundation)
		throw("PxCreateFoundation failed!");

	_pvd = PxCreatePvd(*_pFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// PhysX를 만들어 줍니다.
	bool recordMemoryAllocations = true;
	_toleranceScale.length = 100;
	_toleranceScale.speed = 981;

	_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *_pFoundation,
		_toleranceScale, recordMemoryAllocations, _pvd);

	if (!_pPhysics)
		throw("PxCreatePhysics failed!");

	PxSceneDesc sceneDesc(_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.f, -9.81f, 0.f);
	
	_pDispatcher = PxDefaultCpuDispatcherCreate(2);

	sceneDesc.cpuDispatcher = _pDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	_pScene = _pPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pPvdClient = _pScene->getScenePvdClient();
	if (pPvdClient)
	{
		pPvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pPvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pPvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
}
