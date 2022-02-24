#pragma once
#include "Object.h"

enum class COMPONENT_TYPE : uint8
{
	TRANSFORM,
	MESH_RENDERER,
	CAMERA,
	LIGHT,
	PARTICLE_SYSTEM,
	// etc
	MONO_BEHAVIOUR,
	END,
};

enum
{
	FIXED_COMPONENT_COUNT = static_cast<uint8>(COMPONENT_TYPE::END) - 1
};

class GameObject;
class Transform;

class Component : public Object
{
public : 
	Component(COMPONENT_TYPE type);
	virtual ~Component();

public : 
	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void FinalUpdate() {}

public : 
	COMPONENT_TYPE GetType() const { return _type; }
	bool IsValid() { return _gameObject.expired() == false; }

	shared_ptr<GameObject> GetGameObject();
	shared_ptr<Transform> GetTransform();

private : 
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }

protected : 
	COMPONENT_TYPE _type;
	// weak_ptr
	// 단순히 shared_ptr을 사용을 하면 GameObject와 Component는 가리키면 
	// 순환 구조가 생겨버려서, 영영 레퍼런스 카운터가 줄어들지 않는 문제가 발생한다.
	// 이러한 문제에서 어느 정도 자유로은 것이 바로 weak_ptr이다.
	weak_ptr<GameObject> _gameObject;
};

