#include "GameObject.h"
#include "../Graphics/Material.h"

GameObject::GameObject()
{
}

GameObject::GameObject(const GameObject& ref)
{
}

GameObject::~GameObject()
{
}

void GameObject::Input(float fDeltaTime)
{
}

int GameObject::Update(float fDeltaTime)
{
	return 0;
}

int GameObject::LateUpdate(float fDeltaTime)
{
	return 0;
}

void GameObject::Collision(float fDeltaTime)
{
}

void GameObject::Render(float fDeltaTime)
{
}

Collider* GameObject::GetCollider(const string& strTag)
{
	list<Collider*>::iterator iter;
	list<Collider*>::iterator iterEnd = m_ColliderList.end();

	for (iter = m_ColliderList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter)->GetTag() == strTag)
		{
			(*iter)->AddRef();
			return *iter;
		}
	}

	return nullptr;
}
