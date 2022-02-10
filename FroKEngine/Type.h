#pragma once

// 갠적으로 내가 선언한 타입을 저장합니다.

// 해상도 관련 정보를 저장합니다.
typedef struct _tagResolution
{
	unsigned int nWidth;
	unsigned int nHeight;

	_tagResolution() :
		nWidth(0), nHeight(0) {}

	_tagResolution(int x, int y) :
		nWidth(x), nHeight(y) {}
}RESOLUTION, * PRESOLUTION;

typedef struct _tagAABB
{
	DirectX::XMFLOAT3 m_vecMax;
	DirectX::XMFLOAT3 m_vecMin;
}AABB, * PAABB;