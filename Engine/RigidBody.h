#pragma once
#include "Component.h"

enum class INTERPOLATION_TYPE : uint8
{
    NONE,               // 보간이 적용되지 않습니다.
    INTERPOLATE,        // 이전 프레임의 트랜스폼에 맞게 움직임을 부드럽게 처리합니다.
    EXTRAPOLATE,        // 다음 프레임의 트랜스폼을 추정해 움직임을 부드럽게 처리합니다.
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
    // 충돌체의 마찰력과 탄성력같은 충돌 재질을 만들어준다.
    physx::PxMaterial*                          _material = nullptr;
    // 충돌체의 위치와 회전 값을 설정할 수 있는 Transform이다.
    physx::PxTransform                          _transform;
    physx::PxShape*                             _pShape;
    physx::PxRigidDynamic*                      _pActor;
};

