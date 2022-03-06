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
	// Foundation�� �����ϴµ� �ʿ��� ����
	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;
	PxFoundation*		m_pFoundation = NULL;

	// PxFoundation�� �־�� gPhysics�� ������ �� �ִ�.
	PxPhysics* m_pPhysics = NULL;

	// CPU ���ҽ��� ȿ�������� ������ �� �ֵ��� �ϱ� ���� �����ϴ� ���� ��õ
	PxDefaultCpuDispatcher* m_pDispatcher = NULL;
	// Scene
	PxScene* m_pScene = NULL;
	// m_pPhysics�� ����� createMaterial�ؼ� ���
	// �浹ü ������, Dynamic ������, ź������ �����Ͽ� ���
	PxMaterial* m_pMaterial = NULL;

	////Visual Debugger
	//PxPvd*                  gPvd = NULL;
};

