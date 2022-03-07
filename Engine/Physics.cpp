#include "pch.h"
#include "Physics.h"

DEFINITION_SINGLE(Physics)

Physics::Physics() {}
Physics::~Physics() {}

void Physics::Init()
{
	// Foundation을 만들어 줍니다.
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	// PhysX를 만들어 줍니다.
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale(), true);

	// Scene을 Set 해줍니다.
	physx::PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f); // 중력 세팅
	// Dispatcher를 Set 만들어 줍니다.
	m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	// 위에서 설정한대로 Scene을 만들어 줍니다.
	m_pScene = m_pPhysics->createScene(sceneDesc);
}
