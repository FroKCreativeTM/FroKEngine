#pragma once

#include "../Game.h"

class Camera
{
public :
	Camera();
	~Camera();

	// ���� ���� ī�޶� ��ġ�� ��ȸ �� �����ϴ� �޼���
	DirectX::XMVECTOR GetPosition()const;
	DirectX::XMFLOAT3 GetPosition3f()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);

	// ī�޶��� �������͸� ��ȸ
	DirectX::XMVECTOR GetRight()const;
	DirectX::XMFLOAT3 GetRight3f()const;
	DirectX::XMVECTOR GetUp()const;
	DirectX::XMFLOAT3 GetUp3f()const;
	DirectX::XMVECTOR GetLook()const;
	DirectX::XMFLOAT3 GetLook3f()const;

	// ����ü �Ӽ� ��ȸ
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// �þ� ���� ���� ���� �����*�� ��� �Ÿ����� ��ȸ
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

	// �þ� ����ü ����
	void SetLens(float fovY, float aspect, float zn, float zf);

	// ī�޶� ��ġ, �ü� ����, ���� ���ͷ� ī�޶� ��ǥ�� ����
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	// �þ� ��İ� ���� ��� ��ȸ
	DirectX::XMMATRIX GetView()const;
	DirectX::XMMATRIX GetProj()const;

	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const;

	// ī�޶� �Ÿ� d��ŭ Ⱦ�̵�/���̵��� �Ѵ�.
	void Strafe(float d);
	void Walk(float d);

	// ī�޶� ȸ���Ѵ�.
	void Pitch(float angle);
	void RotateY(float angle);

	// ī�޶� ��ġ�� ������ ������ �Ŀ��� �� �޼��带 ȣ���ؼ� �þ� ����� �籸���Ѵ�.
	void UpdateViewMatrix();

private:

	// ���� ���� ������ ī�޶� ��ǥ��
	DirectX::XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };

	// ����ü �Ӽ���
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;
	float m_NearWindowHeight = 0.0f;
	float m_FarWindowHeight = 0.0f;

	bool m_ViewDirty = true;

	// �þ� ��İ� ���� ���
	DirectX::XMFLOAT4X4 m_View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();
};

