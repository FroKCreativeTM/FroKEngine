#pragma once

class Physics
{
	DECLARE_SINGLE(Physics);

public : 
	void Init();

private : 
	// Foundation�� �����ϴµ� �ʿ��� ����
	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;
	physx::PxFoundation*		m_pFoundation = NULL;

	// PxFoundation�� �־�� gPhysics�� ������ �� �ִ�.
	physx::PxPhysics* m_pPhysics = NULL;

	// CPU ���ҽ��� ȿ�������� ������ �� �ֵ��� �ϱ� ���� �����ϴ� ���� ��õ
	physx::PxDefaultCpuDispatcher* m_pDispatcher = NULL;
	// Scene
	physx::PxScene* m_pScene = NULL;
	// m_pPhysics�� ����� createMaterial�ؼ� ���
	// �浹ü ������, Dynamic ������, ź������ �����Ͽ� ���
	physx::PxMaterial* m_pMaterial = NULL;

	////Visual Debugger
	//PxPvd*                  gPvd = NULL;
};

