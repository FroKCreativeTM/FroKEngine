#pragma once

class Material
{
public : 
	// 고유한 재질 이름
	std::string Name;

	// 이 재질에 해당하는 상수 버퍼의 색인
	int nMatCBIdx = -1;

	// SRV 힙에서 이 재질에 해당하는 분산 텍스처의 인덱스
	int nDiffuseSrvHeapIdx = -1;

	// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는 지의 여부를 뜻하는 더티 플래그이다.
	// FrameResource마다 물체의 cbuffer가 있으므로, FrameResource마다 갱신을 적용해야 한다.
	// 따라서 물체의 자료를 수정할 때에는 반드시
	// NumFrameDirty = gNumFrameResources로 설정해야한다.
	// 그래야 각각의 프레임 자원이 갱신된다.
	int nFramesDirty = gNumFrameResource;

	// 셰이딩에 쓰이는 재질 상수 버퍼 자료
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };		// 분산 반사(금속같은 경우는 0이다.)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f ,0.01f };			// 매질값 예로들어 물의 경우는 0.02, 0.02 0.02이다. 
																	// 반사광의 양은 법선과 빛 벡터 사이의 각도와 재질(Rf(0˚)에 의존한다.
	float fRoughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct MaterialConstants
{
	// 셰이딩에 쓰이는 재질 상수 버퍼 자료
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };		// 분산 반사(금속같은 경우는 0이다.)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f ,0.01f };			// 매질값 예로들어 물의 경우는 0.02, 0.02 0.02이다. 
																	// 반사광의 양은 법선과 빛 벡터 사이의 각도와 재질(Rf(0˚)에 의존한다.
	float fRoughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};