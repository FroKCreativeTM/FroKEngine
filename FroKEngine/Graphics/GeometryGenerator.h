//***************************************************************************************
// 
// Ÿ��ü, ��, �����, ���� ���� �׸��� ���� ���ϱ����� �����ϴ� Ŭ����
// �ϴ� �� �׸���, �����, �浹 ���� �ð�ȭ, ���� ������ �� �����ϴ�.
// �� Ŭ������ ������ �ε����� �̿��ؼ� �ý��� �޸𸮿� �����ϱ� ������
// �� �ڷ�� �ݵ�� ���� ���ۿ� �ε��� ���ۿ� ���� �ؾ� �Ѵ�.
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
	/// �־��� ġ���� ������ �߽����� ���ڸ� �����մϴ�.
	/// face���� m���� ��� n���� �������� �ֽ��ϴ�.
	///</summary>
	MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	///<summary>
	/// �־��� ���������� ������ �߽����� ���� ����ϴ�. 
	/// Slices �� stacks �Ű������� �׼����̼� ������ �����մϴ�.
	///</summary>
	MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// �־��� �ݰ����� ������ �߽����� �������� �����մϴ�. ���̴� �׼����̼� ������ �����մϴ�.
	///</summary>
	MeshData CreateGeosphere(float radius, uint32 numSubdivisions);

	///<summary>
	/// y�࿡ �����ϰ� ������ �߽����� �ϴ� ������ ����ϴ�.
	/// �ϴ� �� ��� �ݰ��� ���� �Ǹ����� �ƴ� �پ��� ���� ����� �����ϱ� ���� �޶��� �� �ֽ��ϴ�.
	/// Slices �� stacks �Ű������� �׼����̼� ������ �����մϴ�.
	///</summary>
	MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// ������ �ʺ�� ���̷� ������ �߽����� m ��� n ���� �ִ� xz ��鿡 mxn �׸��带 ����ϴ�.
	///</summary>
	MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

	///<summary>
	/// ȭ��� ���ĵ� ���带 ����ϴ�. ��ó�� �� ȭ�� ȿ���� �����մϴ�.
	///</summary>
	MeshData CreateQuad(float x, float y, float w, float h, float depth);

private:
	void Subdivide(MeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
};

