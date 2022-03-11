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
};

