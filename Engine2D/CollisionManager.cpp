#include "pch.h"
#include "CollisionManager.h"
#include "Engine.h"
#include "GameObject.h"

#include "BaseCollider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

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
	// 두 개의 충돌체가 있어야 충돌한다.
	if (m_CollisionList.size() < 2)
	{
		m_CollisionList.clear();
		return;
	}

	/* 이중 루프를 돌면서 모든 충돌체간 처리를 담당할 것이다. */
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

	// 오브젝트간 충돌 처리를 한다.
	m_CollisionList.clear();
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

	// ViewSpace에서 Picking 진행
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

		// ViewSpace에서의 Ray 정의
		Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);

		// WorldSpace에서의 Ray 정의
		rayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
		rayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
		rayDir.Normalize();

		// WorldSpace에서 연산
		float distance = 0.f;

		// 충돌 실패
		// if (gameObject->GetCollider()->CollisionSphereToRay(dynamic_pointer_cast<SphereCollider>(gameObject->GetCollider())->, rayOrigin, rayDir, OUT distance) == false)
		// 	continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	return picked;
}