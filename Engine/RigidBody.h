#pragma once
#include "Component.h"

enum class INTERPOLATION_TYPE : uint8
{
    NONE,               // ������ ������� �ʽ��ϴ�.
    INTERPOLATE,        // ���� �������� Ʈ�������� �°� �������� �ε巴�� ó���մϴ�.
    EXTRAPOLATE,        // ���� �������� Ʈ�������� ������ �������� �ε巴�� ó���մϴ�.
};

enum class COLLISION_DETECTION_TYPE : uint8
{
    Discrete,
    Continuous,
    Continuous_Speculative,
};

class BaseCollider;

class RigidBody :
    public Component
{
public:
    RigidBody();
    virtual ~RigidBody();

    virtual void Init(BaseCollider * col);

    virtual void FinalUpdate() override;

public :
    physx::PxMaterial* GetMaterial() { return _material; }
    //physx::PxTransform*  GetPhysicsTransform() { return _transform; }

    bool GetGravity() { _pActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY); }
    void SetGravity(bool b) { _pActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, b); }

private : 
    // �浹ü�� �����°� ź���°��� �浹 ������ ������ش�.
    physx::PxMaterial*                          _material = nullptr;
    // �浹ü�� ��ġ�� ȸ�� ���� ������ �� �ִ� Transform�̴�.
    physx::PxTransform                          _transform;
    physx::PxShape*                             _pShape;
    physx::PxRigidDynamic*                      _pActor;
};

