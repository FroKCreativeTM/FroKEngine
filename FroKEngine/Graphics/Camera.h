#pragma once

#include "../Game.h"

class Camera
{
public :
	Camera();
	~Camera();

	// 세계 공간 카메라 위치를 조회 및 설정하는 메서드
	DirectX::XMVECTOR GetPosition()const;
	DirectX::XMFLOAT3 GetPosition3f()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);

	// 카메라의 기저벡터를 조회
	DirectX::XMVECTOR GetRight()const;
	DirectX::XMFLOAT3 GetRight3f()const;
	DirectX::XMVECTOR GetUp()const;
	DirectX::XMFLOAT3 GetUp3f()const;
	DirectX::XMVECTOR GetLook()const;
	DirectX::XMFLOAT3 GetLook3f()const;

	// 절두체 속성 조회
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// 시야 공간 기준 가장 가까운*먼 평면 거리들을 조회
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

	// 시야 절두체 설정
	void SetLens(float fovY, float aspect, float zn, float zf);

	// 카메라 위치, 시선 벡터, 상향 벡터로 카메라 좌표계 설정
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	// 시야 행렬과 투영 행렬 조회
	DirectX::XMMATRIX GetView()const;
	DirectX::XMMATRIX GetProj()const;

	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const;

	// 카메라를 거리 d만큼 횡이동/축이동을 한다.
	void Strafe(float d);
	void Walk(float d);

	// 카메라를 회전한다.
	void Pitch(float angle);
	void RotateY(float angle);

	// 카메라 위치나 방향을 수정한 후에는 이 메서드를 호출해서 시야 행렬을 재구축한다.
	void UpdateViewMatrix();

private:

	// 세계 공간 기준의 카메라 좌표계
	DirectX::XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };

	// 절두체 속성들
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;
	float m_NearWindowHeight = 0.0f;
	float m_FarWindowHeight = 0.0f;

	bool m_ViewDirty = true;

	// 시야 행렬과 투영 행렬
	DirectX::XMFLOAT4X4 m_View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();
};

