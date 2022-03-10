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
	// Foundation�� �����ϴµ� �ʿ��� ����
	physx::PxDefaultAllocator		_allocator;
	physx::PxDefaultErrorCallback	_errorCallback;
	physx::PxFoundation*			_pFoundation = nullptr;

	// PxFoundation�� �־�� gPhysics�� ������ �� �ִ�.
	physx::PxPhysics*				_pPhysics = nullptr;

	physx::PxTolerancesScale		_toleranceScale;

	// CPU ���ҽ��� ȿ�������� ������ �� �ֵ��� �ϱ� ���� �����ϴ� ���� ��õ
	physx::PxDefaultCpuDispatcher*	_pDispatcher = nullptr;
	// Scene
	physx::PxScene*					_pScene = nullptr;

	////Visual Debugger
	physx::PxPvd*							_pvd = nullptr;
};

