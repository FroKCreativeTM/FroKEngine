#pragma once

class Physics
{
	DECLARE_SINGLE(Physics);

public : 
	void Init();

	void Update();

public : 
	physx::PxPhysics* GetPhysics() { return _pPhysics; }
	physx::PxScene* GetScene() { return _pScene; }

private : 
	// Foundation을 생성하는데 필요한 변수
	physx::PxDefaultAllocator		_allocator;
	physx::PxDefaultErrorCallback	_errorCallback;
	physx::PxFoundation*			_pFoundation = nullptr;

	// PxFoundation이 있어야 gPhysics를 생성할 수 있다.
	physx::PxPhysics*				_pPhysics = nullptr;

	physx::PxTolerancesScale		_toleranceScale;

	// CPU 리소스를 효율적으로 공유할 수 있도록 하기 위해 구현하는 것을 추천
	physx::PxDefaultCpuDispatcher*	_pDispatcher = nullptr;
	// Scene
	physx::PxScene*					_pScene = nullptr;

	////Visual Debugger
	physx::PxPvd*							_pvd = nullptr;
};

