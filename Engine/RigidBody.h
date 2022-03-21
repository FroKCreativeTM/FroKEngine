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

const float GRAVITY = 9.81f;

class RigidBody :
    public Component
{
private : 
    bool _isGravity;

public:
    RigidBody();
    virtual ~RigidBody();

    virtual void Init(BaseCollider * col);

    void SetGravityOn(bool b) { _isGravity = b; }
    bool GetGravityOn() const { return _isGravity; }

    virtual void FinalUpdate() override;
};

