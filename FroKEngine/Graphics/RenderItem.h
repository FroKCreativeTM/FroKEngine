#pragma once

#include "../Game.h"

using namespace DirectX;

enum class RenderLayer : int
{
	Opaque = 0,		// 불투명체
	Count
};

// 하나의 물체를 그리는 데 필요한 매개변수들을 담는 클래스이다.
// 이 클래스의 구체적인 구성은 응용 프로그램마다 달라질 것이다.
class RenderItem
{
public : 
	RenderItem() = default;

	// 세계 공간을 기준으로 물체의 로컬 공간을 서술ㅇ하는 세계 행렬이다.
	// 이 행렬은 세계 공간 상에서의 물체의 위치, 방향, 크기를 결정한다.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는 지의 여부를 뜻하는 더티 플래그이다.
	// FrameResource마다 물체의 cbuffer가 있으므로, FrameResource마다 갱신을 적용해야 한다.
	// 따라서 물체의 자료를 수정할 때에는 반드시
	// NumFrameDirty = gNumFrameResources로 설정해야한다.
	// 그래야 각각의 프레임 자원이 갱신된다.
	int nFramesDirty = gNumFrameResource;

	// 이 렌더 아이템의 물체 상수 버퍼에 해당하는 GPU 상수 버퍼의 색인
	UINT objCBIdx = -1;

	// 이 렌더 항목에 연관된 기하 구조및 마테리얼을 연결한다. 
	// 여러 렌더가 항목이 같은 기하 구조를 참조할 수 있음을 주의하라.
	Material* Mat = nullptr;
	MeshGeometry* pGeometry = nullptr;

	// 기본 도형 위상 구조
	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced 매개변수들
	UINT nIdxCnt = 0;
	UINT nStartIdxLocation = 0;
	int nBaseVertexLocation = 0;
};