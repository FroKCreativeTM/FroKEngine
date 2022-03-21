#include "pch.h"
#include "CollisionManager.h"
#include "Engine.h"
#include "GameObject.h"
#include "Transform.h"
#include "Camera.h"

#include "BaseCollider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"

#include "Camera.h"
#include "SceneManager.h"
#include "Scene.h"

DEFINITION_SINGLE(CollisionManager)

CollisionManager::CollisionManager() {}
CollisionManager::~CollisionManager() {}

void CollisionManager::AddObject(shared_ptr<GameObject> pObj)
{
	if (pObj->GetCollider())
	{
		m_CollisionList.push_back(pObj);
	}
}

void CollisionManager::Update()
{
	// �� ���� �浹ü�� �־�� �浹�Ѵ�.
	if (m_CollisionList.size() < 2)
	{
		m_CollisionList.clear();
		return;
	}

	/* ���� ������ ���鼭 ��� �浹ü�� ó���� ����� ���̴�. */
	list<shared_ptr<GameObject>>::iterator iter;
	list<shared_ptr<GameObject>>::iterator iterEnd = m_CollisionList.end();
	--iterEnd;

	for (iter = m_CollisionList.begin(); iter != iterEnd; ++iter)
	{
		list<shared_ptr<GameObject>>::iterator innerIter = iter;
		++innerIter;
		list<shared_ptr<GameObject>>::iterator innerIterEnd = m_CollisionList.end();

		for (; innerIter != innerIterEnd; ++innerIter)
		{
			Collision(*(iter)->get(), *(innerIter)->get());
		}
	}

	// ������Ʈ�� �浹 ó���� �Ѵ�.
	m_CollisionList.clear();
}

void CollisionManager::Render()
{
#ifdef _DEBUG
	// �� ���� �浹ü�� �־�� �浹�Ѵ�.
	if (m_CollisionList.size() < 2)
	{
		m_CollisionList.clear();
		return;
	}

	for (auto& iter : m_CollisionList)
	{
		
	}

	// ������Ʈ�� �浹 ó���� �Ѵ�.
	m_CollisionList.clear();
#endif
}

bool CollisionManager::Collision(GameObject pSrc, GameObject pDst)
{
	return pSrc.GetCollider()->Collision(pDst.GetCollider().get());
}

shared_ptr<class GameObject> CollisionManager::Pick(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GET_SINGLE(SceneManager)->GetActiveScene()->GetMainCamera();

	float width = static_cast<float>(GEngine->GetWindow().width);
	float height = static_cast<float>(GEngine->GetWindow().height);

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	// ViewSpace���� Picking ����
	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	auto& gameObjects = GET_SINGLE(SceneManager)->GetActiveScene()->GetGameObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetCollider() == nullptr)
			continue;

		if (GET_SINGLE(SceneManager)->IndexToLayerName(gameObject->GetLayerIndex()) == L"UI")
			continue;

		// ViewSpace������ Ray ����
		Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);

		// WorldSpace������ Ray ����
		rayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
		rayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
		rayDir.Normalize();


		// WorldSpace���� ����
		float distance = 0.f;

		if (gameObject->GetCollider()->Collision(rayOrigin, rayDir, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;

			wcout << gameObject->GetName() << L"Hit" << endl;
		}
	}

	return picked;
}

shared_ptr<class GameObject> CollisionManager::PickUI(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GET_SINGLE(SceneManager)->GetActiveScene()->GetMainCamera();

	float width = static_cast<float>(GEngine->GetWindow().width);
	float height = static_cast<float>(GEngine->GetWindow().height);

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	// ViewSpace���� Picking ����
	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	auto& gameObjects = GET_SINGLE(SceneManager)->GetActiveScene()->GetGameObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetCollider() == nullptr)
			continue;

		if (GET_SINGLE(SceneManager)->IndexToLayerName(gameObject->GetLayerIndex()) != L"UI")
			continue;

		Matrix matTranslation = Matrix::CreateTranslation(gameObject->GetTransform()->GetLocalPosition());
		auto result = matTranslation* Camera::S_MatView * Camera::S_MatProjection;

		if (gameObject->GetCollider()->Collision(screenX, screenY) == false)
			continue;
		else
			return gameObject;
	}

	return nullptr;
}
