#pragma once

class Physics
{
	DECLARE_SINGLE(Physics);

public : 
	void Init();

private : 
	// Foundation�� �����ϴµ� �ʿ��� ����
	physx::PxDefaultAllocator		_allocator;
	physx::PxDefaultErrorCallback	_errorCallback;
	physx::PxFoundation*			_pFoundation = NULL;

	// PxFoundation�� �־�� gPhysics�� ������ �� �ִ�.
	physx::PxPhysics*				_pPhysics = NULL;

	physx::PxTolerancesScale		_toleranceScale;

	// CPU ���ҽ��� ȿ�������� ������ �� �ֵ��� �ϱ� ���� �����ϴ� ���� ��õ
	physx::PxDefaultCpuDispatcher*	_pDispatcher = NULL;
	// Scene
	physx::PxScene*					_pScene = NULL;
	// m_pPhysics�� ����� createMaterial�ؼ� ���
	// �浹ü ������, Dynamic ������, ź������ �����Ͽ� ���
	physx::PxMaterial*				_pMaterial = NULL;

	////Visual Debugger
	PxPvd*							_pvd = NULL;
};

