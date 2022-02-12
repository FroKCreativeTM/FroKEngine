#include "Camera.h"

DEFINITION_SINGLE(Camera)

Camera::Camera()
{
    SetLens(0.25f * MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{
}

DirectX::XMVECTOR Camera::GetPosition() const
{
    return XMLoadFloat3(&m_Position);
}

DirectX::XMFLOAT3 Camera::GetPosition3f() const
{
    return m_Position;
}

void Camera::SetPosition(float x, float y, float z)
{
    m_Position = XMFLOAT3(x, y, z);
    m_ViewDirty = true;
}

void Camera::SetPosition(const DirectX::XMFLOAT3& v)
{
    m_Position = v;
    m_ViewDirty = true;
}

DirectX::XMVECTOR Camera::GetRight() const
{
    return XMLoadFloat3(&m_Right);
}

DirectX::XMFLOAT3 Camera::GetRight3f() const
{
    return m_Right;
}

DirectX::XMVECTOR Camera::GetUp() const
{
    return XMLoadFloat3(&m_Up);
}

DirectX::XMFLOAT3 Camera::GetUp3f() const
{
    return m_Up;
}

DirectX::XMVECTOR Camera::GetLook() const
{
    return XMLoadFloat3(&m_Look);
}

DirectX::XMFLOAT3 Camera::GetLook3f() const
{
    return m_Look;
}

float Camera::GetNearZ() const
{
    return m_NearZ;
}

float Camera::GetFarZ() const
{
    return m_FarZ;
}

float Camera::GetAspect() const
{
    return m_Aspect;
}

float Camera::GetFovY() const
{
    return m_FovY;
}

float Camera::GetFovX() const
{
    float halfWidth = 0.5f * GetNearWindowWidth();
    return 2.0f * atan(halfWidth / m_NearZ);
}

float Camera::GetNearWindowWidth() const
{
    return m_Aspect * m_NearWindowHeight;
}

float Camera::GetNearWindowHeight() const
{
    return m_NearWindowHeight;
}

float Camera::GetFarWindowWidth() const
{
    return m_Aspect * m_FarWindowHeight;
}

float Camera::GetFarWindowHeight() const
{
    return m_FarWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
    // 속성값들을 보관해둔다.
    m_FovY = fovY;
    m_Aspect = aspect;
    m_NearZ = zn;
    m_FarZ = zf;

    m_NearWindowHeight = 2.0f * m_NearZ * tanf(0.5f * m_FovY);
    m_FarWindowHeight = 2.0f * m_FarZ * tanf(0.5f * m_FovY);

    XMMATRIX P = XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
    XMStoreFloat4x4(&m_Proj, P);
}

void Camera::LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp)
{
    XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
    XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
    XMVECTOR U = XMVector3Cross(L, R);

    XMStoreFloat3(&m_Position, pos);
    XMStoreFloat3(&m_Look, L);
    XMStoreFloat3(&m_Right, R);
    XMStoreFloat3(&m_Up, U);

    m_ViewDirty = true;
}

void Camera::LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
{
    XMVECTOR P = XMLoadFloat3(&pos);
    XMVECTOR T = XMLoadFloat3(&target);
    XMVECTOR U = XMLoadFloat3(&up);

    LookAt(P, T, U);

    m_ViewDirty = true;
}

DirectX::XMMATRIX Camera::GetView() const
{
    assert(!m_ViewDirty);
    return XMLoadFloat4x4(&m_View);
}

DirectX::XMMATRIX Camera::GetProj() const
{
    return XMLoadFloat4x4(&m_Proj);
}

DirectX::XMFLOAT4X4 Camera::GetView4x4f() const
{
    assert(!m_ViewDirty);
    return m_View;
}

DirectX::XMFLOAT4X4 Camera::GetProj4x4f() const
{
    return m_Proj;
}

void Camera::Strafe(float d)
{
    // mPosition += d*mRight
    XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR r = XMLoadFloat3(&m_Right);
    XMVECTOR p = XMLoadFloat3(&m_Position);
    XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, r, p));

    m_ViewDirty = true;
}

void Camera::Walk(float d)
{
    // mPosition += d*mLook
    XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR l = XMLoadFloat3(&m_Look);
    XMVECTOR p = XMLoadFloat3(&m_Position);
    XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, l, p));

    m_ViewDirty = true;
}

void Camera::Pitch(float angle)
{
    // 상향 벡터와 시선 벡터를 오른쪽 벡터에 대해 회전한다.

    XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);

    XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
    XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

    m_ViewDirty = true;
}

void Camera::RotateY(float angle)
{
    // 기저 벡터들을 세계 공간 y축에 대해 회전한다.

    XMMATRIX R = XMMatrixRotationY(angle);

    XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), R));
    XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
    XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

    m_ViewDirty = true;
}

void Camera::UpdateViewMatrix()
{
    if (m_ViewDirty)
    {
        XMVECTOR R = XMLoadFloat3(&m_Right);
        XMVECTOR U = XMLoadFloat3(&m_Up);
        XMVECTOR L = XMLoadFloat3(&m_Look);
        XMVECTOR P = XMLoadFloat3(&m_Position);

        // 카메라의 축들이 서로 직교(수직상태인)인 단위벡터가 되게 한다.
        L = XMVector3Normalize(L);
        U = XMVector3Normalize(XMVector3Cross(L, R));

        // U와 L이 이미 정규직교이므로, 그 외적인 정규화할 필요가 없다
        R = XMVector3Cross(U, L);

        // 시야 행렬의 성분들을 채운다.
        float x = -XMVectorGetX(XMVector3Dot(P, R));
        float y = -XMVectorGetX(XMVector3Dot(P, U));
        float z = -XMVectorGetX(XMVector3Dot(P, L));

        XMStoreFloat3(&m_Right, R);
        XMStoreFloat3(&m_Up, U);
        XMStoreFloat3(&m_Look, L);

        m_View(0, 0) = m_Right.x;
        m_View(1, 0) = m_Right.y;
        m_View(2, 0) = m_Right.z;
        m_View(3, 0) = x;

        m_View(0, 1) = m_Up.x;
        m_View(1, 1) = m_Up.y;
        m_View(2, 1) = m_Up.z;
        m_View(3, 1) = y;

        m_View(0, 2) = m_Look.x;
        m_View(1, 2) = m_Look.y;
        m_View(2, 2) = m_Look.z;
        m_View(3, 2) = z;

        m_View(0, 3) = 0.0f;
        m_View(1, 3) = 0.0f;
        m_View(2, 3) = 0.0f;
        m_View(3, 3) = 1.0f;

        m_ViewDirty = false;
    }
}

bool Camera::Init(float x, float y, float z)
{
    m_Position = XMFLOAT3(x, y, z);
    m_ViewDirty = true;

    return true;
}

bool Camera::Init(const DirectX::XMFLOAT3& tPos)
{
    m_Position = tPos;
    m_ViewDirty = true;

    return true;
}

void Camera::Input(float fDeltaTime)
{
}

void Camera::Update(float fDeltaTime)
{
}
