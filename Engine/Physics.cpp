#include "pch.h"
#include "Physics.h"

DEFINITION_SINGLE(Physics)

Physics::Physics() {}
Physics::~Physics() {}

void Physics::Init()
{
	// Foundation�� ����� �ݴϴ�.
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	// PhysX�� ����� �ݴϴ�.
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale(), true);

	// Scene�� Set ���ݴϴ�.
	physx::PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f); // �߷� ����
	// Dispatcher�� Set ����� �ݴϴ�.
	m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	// ������ �����Ѵ�� Scene�� ����� �ݴϴ�.
	m_pScene = m_pPhysics->createScene(sceneDesc);
}
