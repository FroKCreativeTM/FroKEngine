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
    void SetMass(float m) { _mass = m; }
    float GetMass() const { return _mass; }

    void SetDrag(float g) { _drag = g; }
    float GetDrag() const { return _drag; }

    void SetUseGravity(bool b) { _useGravity = b; }
    bool GetUseGravity() const { return _useGravity; }

private : 
    float _mass;                                    // ����
    float _drag;                                    // ������Ʈ�� ���� ���� ������ �� ���� ������ ������ ��ġ�� ����
    float _angularDrag;                             // ������Ʈ�� ��ũ�� ȸ���� �� ���� ������ ������ ��ġ�� ���� (0�̸� ���� ������ ����)
    bool _useGravity;
    bool _isKinetic;                                // Ȱ��ȭ�Ǹ� ������Ʈ�� ���� �������� ������� �ʰ� ������ Transform ���θ� ����
    INTERPOLATION_TYPE _interpolate;    
    COLLISION_DETECTION_TYPE _collisionDetection;   // ������ �����̴� ������Ʈ�� �浹�� ���� ���� �ٸ� ������Ʈ�� �����İ��� ���� ����
    bool Constraints[2][3];                         // 0 : Freeze Position
};

