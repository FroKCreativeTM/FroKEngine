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
