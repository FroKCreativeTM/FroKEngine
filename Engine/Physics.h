#pragma once

class Physics
{
	DECLARE_SINGLE(Physics);

public : 
	void Init();

private : 
	// Foundation을 생성하는데 필요한 변수
	physx::PxDefaultAllocator		_allocator;
	physx::PxDefaultErrorCallback	_errorCallback;
	physx::PxFoundation*			_pFoundation = NULL;

	// PxFoundation이 있어야 gPhysics를 생성할 수 있다.
	physx::PxPhysics*				_pPhysics = NULL;

	physx::PxTolerancesScale		_toleranceScale;

	// CPU 리소스를 효율적으로 공유할 수 있도록 하기 위해 구현하는 것을 추천
	physx::PxDefaultCpuDispatcher*	_pDispatcher = NULL;
	// Scene
	physx::PxScene*					_pScene = NULL;
	// m_pPhysics를 사용해 createMaterial해서 사용
	// 충돌체 마찰력, Dynamic 마찰력, 탄성력을 지정하여 사용
	physx::PxMaterial*				_pMaterial = NULL;

	////Visual Debugger
	PxPvd*							_pvd = NULL;
};

