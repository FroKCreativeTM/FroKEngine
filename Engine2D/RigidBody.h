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

const float GRAVITY = 9.81f;

class RigidBody :
    public Component
{
public:
    RigidBody();
    virtual ~RigidBody();

    virtual void FinalUpdate() override;

public :
    void SetUseGravity(bool b) { _useGravity = b; }
    bool GetUseGravity() const { return _useGravity; }

private : 
    physx::PxRigidBody* _rigidBody = nullptr;
    bool _useGravity;
    float speed = 1.f;
};

