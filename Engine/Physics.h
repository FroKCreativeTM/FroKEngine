#pragma once

class PxDefaultAllocator;
class PxDefaultErrorCallback;
class PxFoundation;
class PxDefaultAllocator;

class Physics
{
public : 
	void Init();

private : 
	// Foundation을 생성하는데 필요한 변수
	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;
	PxFoundation*		m_pFoundation = NULL;

	// PxFoundation이 있어야 gPhysics를 생성할 수 있다.
	PxPhysics* m_pPhysics = NULL;

	// CPU 리소스를 효율적으로 공유할 수 있도록 하기 위해 구현하는 것을 추천
	PxDefaultCpuDispatcher* m_pDispatcher = NULL;
	// Scene
	PxScene* m_pScene = NULL;
	// m_pPhysics를 사용해 createMaterial해서 사용
	// 충돌체 마찰력, Dynamic 마찰력, 탄성력을 지정하여 사용
	PxMaterial* m_pMaterial = NULL;

	////Visual Debugger
	//PxPvd*                  gPvd = NULL;
};

