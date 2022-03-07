#pragma once

class Physics
{
	DECLARE_SINGLE(Physics);

public : 
	void Init();

private : 
	// Foundation을 생성하는데 필요한 변수
	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;
	physx::PxFoundation*		m_pFoundation = NULL;

	// PxFoundation이 있어야 gPhysics를 생성할 수 있다.
	physx::PxPhysics* m_pPhysics = NULL;

	// CPU 리소스를 효율적으로 공유할 수 있도록 하기 위해 구현하는 것을 추천
	physx::PxDefaultCpuDispatcher* m_pDispatcher = NULL;
	// Scene
	physx::PxScene* m_pScene = NULL;
	// m_pPhysics를 사용해 createMaterial해서 사용
	// 충돌체 마찰력, Dynamic 마찰력, 탄성력을 지정하여 사용
	physx::PxMaterial* m_pMaterial = NULL;

	////Visual Debugger
	//PxPvd*                  gPvd = NULL;
};

