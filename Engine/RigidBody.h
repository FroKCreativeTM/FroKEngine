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
};

