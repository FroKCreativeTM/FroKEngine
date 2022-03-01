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
    float _mass;                                    // 질량
    float _drag;                                    // 오브젝트가 힘에 의해 움직일 때 공기 저항이 영향을 미치는 정도
    float _angularDrag;                             // 오브젝트가 토크로 회전할 때 공기 저항이 영향을 미치는 정도 (0이면 공기 저항이 없음)
    bool _useGravity;
    bool _isKinetic;                                // 활성화되면 오브젝트는 물리 엔진으로 제어되지 않고 오로지 Transform 으로만 조작
    INTERPOLATION_TYPE _interpolate;    
    COLLISION_DETECTION_TYPE _collisionDetection;   // 빠르게 움직이는 오브젝트가 충돌의 감지 없이 다른 오브젝트를 지나쳐가는 것을 방지
    bool Constraints[2][3];                         // 0 : Freeze Position
};

