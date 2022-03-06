#include "pch.h"
#include "Physics.h"

void Physics::Init()
{
	// Foundation을 만들어 줍니다.
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	// PhysX를 만들어 줍니다.
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true);

	// Scene을 Set 해줍니다.
	PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f); // 중력 세팅
	// Dispatcher를 Set 만들어 줍니다.
	m_pDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	// 위에서 설정한대로 Scene을 만들어 줍니다.
	m_pScene = m_pPhysics->createScene(sceneDesc);
	// Material을 만들어 줍니다.
	m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f); // 충돌체 마찰력,Dynamic마찰력, 탄성력

	// 평지를 만들어 줍니다.
	PxRigidStatic* groundPlane = PxCreatePlane(*m_pPhysics, PxPlane(0, 1, 0, 0), *m_pMaterial);
	m_pScene->addActor(*groundPlane);
}
