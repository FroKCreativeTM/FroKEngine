//***************************************************************************************
// 
// 타원체, 구, 원기둥, 원뿔 등을 그리기 위한 기하구조를 생성하는 클래스
// 하늘 돔 그리기, 디버깅, 충돌 검출 시각화, 지연 렌더링 등등에 유용하다.
// 이 클래스는 정점과 인덱스를 이용해서 시스템 메모리에 생성하기 때문에
// 그 자료는 반드시 정점 버퍼와 인덱스 버퍼에 복사 해야 한다.
// 
//***************************************************************************************

#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>


class GeometryGenerator
{
public:
	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;

	struct Vertex
	{
		Vertex() {}
		Vertex(
			const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT3& t,
			const DirectX::XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 TangentU;
		DirectX::XMFLOAT2 TexC;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices32;

		std::vector<uint16>& GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); ++i)
					mIndices16[i] = static_cast<uint16>(Indices32[i]);
			}

			return mIndices16;
		}

	private:
		std::vector<uint16> mIndices16;
	};


	///<summary>
	/// 주어진 치수로 원점을 중심으로 상자를 생성합니다.
	/// face에는 m개의 행과 n개의 꼭짓점이 있습니다.
	///</summary>
	MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	///<summary>
	/// 주어진 반지름으로 원점을 중심으로 구를 만듭니다. 
	/// Slices 및 stacks 매개변수는 테셀레이션 정도를 제어합니다.
	///</summary>
	MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// 주어진 반경으로 원점을 중심으로 지리권을 생성합니다. 깊이는 테셀레이션 수준을 제어합니다.
	///</summary>
	MeshData CreateGeosphere(float radius, uint32 numSubdivisions);

	///<summary>
	/// y축에 평행하고 원점을 중심으로 하는 원통을 만듭니다.
	/// 하단 및 상단 반경은 실제 실린더가 아닌 다양한 원뿔 모양을 형성하기 위해 달라질 수 있습니다.
	/// Slices 및 stacks 매개변수는 테셀레이션 정도를 제어합니다.
	///</summary>
	MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// 지정된 너비와 깊이로 원점을 중심으로 m 행과 n 열이 있는 xz 평면에 mxn 그리드를 만듭니다.
	///</summary>
	MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

	///<summary>
	/// 화면과 정렬된 쿼드를 만듭니다. 후처리 및 화면 효과에 유용합니다.
	///</summary>
	MeshData CreateQuad(float x, float y, float w, float h, float depth);

private:
	void Subdivide(MeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
};

