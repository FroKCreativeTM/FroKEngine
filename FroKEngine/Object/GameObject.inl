#include "../Collision/Collider.h"
#include "GameObject.h"

template<typename T>
inline void GameObject::AddCollisionFunction(const string& strTag,
	COLLISION_STATE eState, T* pObj, void(T::* pFunc)(Collider*, Collider*, float))
{
	list<CCollider*>::iterator iter;
	list<CCollider*>::iterator iterEnd = m_ColliderList.end();

	for (iter = m_ColliderList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter)->GetTag() == strTag)
		{
			(*iter)->AddCollisionFunction(eState, pObj, pFunc);
			break;
		}
	}
}